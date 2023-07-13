#include "soup_bin_server.h"
#include "soupbintcp.h"
#include <iostream>

SoupBinConnection::SoupBinConnection(boost::asio::ip::tcp::socket inSkt, boost::asio::io_context* io_context, MessageRepeater* parent)
        : heartbeatTimer(this, 20000, Timer::get_time()), localIsServer(true), skt(std::move(inSkt)), server_context(io_context), parent(parent)
{
    status = Status::CONNECTED;
    do_read_header();
}

SoupBinConnection::SoupBinConnection(const std::string& url, const std::string& user, const std::string& pw,
        const std::string& sessionId, uint64_t nextSequenceNo) 
        : heartbeatTimer(this, 20000, Timer::get_time()), localIsServer(false), skt(io_context), 
        username(user), password(pw), sessionId(sessionId), nextSeq(nextSequenceNo)
{
    try
    {
        std::string address = url;
        std::string port = "80";
        size_t pos = address.find(":");
        if (pos != std::string::npos)
        {
            port = address.substr(pos + 1);
            address = address.substr(0, pos);
        }
        boost::asio::ip::tcp::resolver resolver(io_context);
        do_connect(resolver.resolve(address, port));
        readerThread = std::thread([this]() { io_context.run(); });
    } 
    catch(const std::exception& e)
    {
        // TODO
    }
}

SoupBinConnection::~SoupBinConnection()
{
    try
    {
        close_socket();
        if (readerThread.joinable())
            readerThread.join();
    } catch(...) {
        // TODO: 
    }
}

std::string serverOrClient(bool server)
{
    if (server)
        return "Server";
    return "Client";
}

void SoupBinConnection::close_socket()
{
    try {
        status = Status::DISCONNECTED;
        if (skt.is_open())
            skt.close();
    } catch (...) {
    }
}

void SoupBinConnection::on_login_request(const soupbintcp::login_request& in)
{
    std::cout << "on_login_request received\n";
    std::string requestedSessionId = in.get_string(soupbintcp::login_request::REQUESTED_SESSION);
    if (requestedSessionId.empty())
    {
        std::stringstream ss;
        ss << std::right << std::setw(10) << "ABC";
        requestedSessionId = ss.str();
    }
    uint64_t requestedSeqNo = in.get_int(soupbintcp::login_request::REQUESTED_SEQUENCE_NUMBER); 
    bool resend = false;
    if (requestedSeqNo != 0)
        resend = true;
    else
        requestedSeqNo = 1;
    soupbintcp::login_accepted msg;
    msg.set_int(soupbintcp::login_accepted::SEQUENCE_NUMBER, requestedSeqNo);
    msg.set_string(soupbintcp::login_accepted::SESSION, requestedSessionId);
    send(msg.get_record_as_vec());
    if (resend)
    {
        parent->repeat_from(this, requestedSeqNo);
    }
}

void SoupBinConnection::do_connect(const boost::asio::ip::tcp::resolver::results_type& endpoints)
{
    boost::asio::async_connect(skt, endpoints, [this](boost::system::error_code ec, boost::asio::ip::tcp::endpoint) {
        if (!ec)
        {
            status = Status::CONNECTED;
            // attempt login
            soupbintcp::login_request req;
            req.set_string(soupbintcp::login_request::USERNAME, username);
            req.set_string(soupbintcp::login_request::PASSWORD, password);
            req.set_int(soupbintcp::login_request::REQUESTED_SEQUENCE_NUMBER, nextSeq);
            req.set_string(soupbintcp::login_request::REQUESTED_SESSION, sessionId);
            send(req.get_record_as_vec());
            do_read_header();
        }
    });
}
void SoupBinConnection::do_read_header()
{
    if (localIsServer)
        std::cout << "SoupBinConnection::do_read_header server waiting for more data\n";
    else
        std::cout << "SoupBinConnection::do_read_header client waiting for more data\n";
    // read from network, placing first 3 bytes into buffer
    boost::asio::async_read(skt, boost::asio::buffer(currentIncoming.data(), 3),
            [this](boost::system::error_code ec, std::size_t length) {
                if (!ec)
                {
                    if (currentIncoming.decode_header())
                    {
                        if (!localIsServer)
                            std::cout << "SoupBinConnection::do_read_header: client read " << length << " bytes.\n";
                        else
                            std::cout << "SoupBinConnection::do_read_header: server read " << length << " bytes.\n";
                        do_read_body();
                    }
                    else
                    {
                        std::cout << "SoupBinConnection::do_read_header: *** client read " << length << " bytes but header not valid ***\n";
                        do_read_header();
                    }
                }
                else
                {
                    if (localIsServer)
                        std::cout << "SoupBinConnection::do_read_header: error reading header bytes from client. Error " << ec << "\n";
                    else
                        std::cout << "SoupBinConnection::do_read_header: error reading header bytes from server. Error " << ec << "\n";
                    close_socket();
                }
            });
}
void SoupBinConnection::do_read_body()
{
    std::cout << "SoupBinConnection::do_read_body: "
            << serverOrClient(localIsServer) << ": Will attempt to read body of " << currentIncoming.body_length() << " bytes\n";
    boost::asio::async_read(skt, boost::asio::buffer(currentIncoming.body(), currentIncoming.body_length()),
            [this](boost::system::error_code ec, std::size_t length ) 
            {
                if (!ec)
                {
                    std::cout << "SoupBinConnection::do_read_body: " << serverOrClient(localIsServer) << ": read " << length << " bytes\n";
                    // if this is a system message, handle it. Otherwise place it in queue
                    if (currentIncoming.decode_header()) {
                        switch(currentIncoming.data()[2])
                        {
                            // from server or client
                            case('+'): // debug packet
                                on_debug(soupbintcp::debug_packet(currentIncoming.data()));
                                break;
                            // from server
                            case('A'): // login accepted
                                on_login_accepted(soupbintcp::login_accepted(currentIncoming.data()));
                                break;
                            case('J'): // login rejected
                                on_login_rejected(soupbintcp::login_rejected(currentIncoming.data()));
                                break;
                            case('S'):
                                on_sequenced_data(soupbintcp::sequenced_data(currentIncoming.data()));
                                break;
                            case('H'): // heartbeat coming from server
                                on_server_heartbeat(soupbintcp::server_heartbeat(currentIncoming.data()));
                                break;
                            case('Z'): // server end of session
                                on_end_of_session(soupbintcp::end_of_session(currentIncoming.data()));
                                break;
                            // from client
                            case('L'): // login request
                                on_login_request(soupbintcp::login_request(currentIncoming.data()));
                                break;
                            case('U'):
                                on_unsequenced_data(soupbintcp::unsequenced_data(currentIncoming.data()));
                                break;
                            case('R'):
                                on_client_heartbeat(soupbintcp::client_heartbeat(currentIncoming.data()));
                                break;
                            case('O'):
                                on_logout_request(soupbintcp::logout_request(currentIncoming.data()));
                                break;
                            default:
                            {
                                // this should never happen
                                std::cout << "SoupBinConnection::do_read_body: Read " << length << " bytes but not valid header\n";
                                // place message in queue
                                std::vector<unsigned char> vec(currentIncoming.data(), currentIncoming.data() + currentIncoming.body_length() + 3);
                                read_msgs.push_back(vec);
                                break;
                            }
                        }
                        do_read_header();
                    }
                    else
                    {
                        if (localIsServer)
                            std::cout << "SoupBinConnection::do_read_body: server read " << length << " bytes but could not decode header\n";
                        else
                            std::cout << "SoupBinConnection::do_read_body: client read " << length << " bytes but could not decode header\n";
                        close_socket();
                    }
                }
                else
                {
                    if (localIsServer)
                        std::cout << "SoupBinConnection::do_read_body: server read " << length << " bytes but had error " << ec << "\n";
                    else
                        std::cout << "SoupBinConnection::do_read_body: client read " << length << " bytes but had error " << ec << "\n";
                    close_socket();
                }
            });
}

void SoupBinConnection::send_sequenced(uint64_t seqNo, const std::vector<unsigned char>& bytes)
{
    // add to map
    messages.emplace(seqNo, bytes);
    soupbintcp::sequenced_data msg;
    msg.set_message(bytes);
    send(msg.get_record_as_vec());
}

void SoupBinConnection::send_sequenced(const std::vector<unsigned char>& bytes)
{
    send_sequenced(get_next_seq(), bytes);
}

void SoupBinConnection::send_unsequenced(const std::vector<unsigned char>& bytes)
{
    soupbintcp::unsequenced_data msg;
    msg.set_message(bytes);
    send(msg.get_record_as_vec());
}

void SoupBinConnection::do_write()
{
    std::cout << "SoupBinConnection::do_write() "<< serverOrClient(localIsServer) << " in do_write with " 
            << write_msgs.size() << " messages in queue. Message type: "
            << (char)write_msgs.front()[2] << " and number of bytes of "
            << write_msgs.front().size() << "\n";
    boost::asio::async_write(skt, boost::asio::buffer(write_msgs.front().data(), write_msgs.front().size()),
            [this](boost::system::error_code ec, std::size_t /* length */) {
                if (!ec) {
                    write_msgs.pop_front();
                    if (!write_msgs.empty())
                        do_write();
                } else {
                    std::cout << "SoupBinConnection::to_write: " << serverOrClient(localIsServer) 
                            << " write failed with error " << ec << "\n";
                    close_socket();
                }
            });
}

void SoupBinConnection::send(const std::vector<unsigned char>& bytes)
{
    std::cout << "SoupBinConnection::send() " << serverOrClient(localIsServer)
            << " About to send " << bytes.size()
            << " bytes of type " << (char)bytes[2]
            << " with " << write_msgs.size() << " messages in queue\n";
    if (localIsServer)
    {
        bool write_in_progress = !write_msgs.empty();
        write_msgs.push_back(bytes);
        if (!write_in_progress)
            do_write();
    }
    else
    {
        boost::asio::post(io_context, [this, bytes]() {
            bool write_in_progress = !write_msgs.empty();
            write_msgs.push_back(bytes);
            if (!write_in_progress) {
                do_write();
            }
        });
    }
}

uint64_t SoupBinConnection::get_next_seq(bool increment) 
{ 
    if(!increment)
        return nextSeq;
    return nextSeq++; 
}

void SoupBinConnection::OnTimer(uint64_t msSince)
{
    // send heartbeat
    if (localIsServer)
    {
        soupbintcp::server_heartbeat hb;
        send(hb.get_record_as_vec());
    }
    else
    {
        soupbintcp::client_heartbeat hb;
        send(hb.get_record_as_vec());
    }
}
