#pragma once
#include "soup_bin_connection.h"
#include <vector>
#include <memory>
#include <boost/asio.hpp>

class SoupBinLoginVerifier
{
    public:
    virtual bool IsValid(const std::string& username, const std::string& password) = 0;
};

class AnonymousLoginVerifier : public SoupBinLoginVerifier
{
    public:
    virtual bool IsValid(const std::string& u, const std::string& p) override { return true; }
};

/***
 * A SoupBin server that listens on a socket
*/
template<typename CONNECTION>
class SoupBinServer
{
    public:
    SoupBinServer(int32_t listenPort)
    {
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), listenPort);
        acceptor = new boost::asio::ip::tcp::acceptor(io_context, endpoint);
        do_accept();
        runThread = std::thread([this]() { io_context.run(); } );
    }
    virtual ~SoupBinServer()
    {
        shuttingDown = true;
        delete acceptor;
        if (runThread.joinable())
            runThread.join();
    }
    void set_login_verifier(SoupBinLoginVerifier* verifier) { loginVerifier = verifier; }

    void send_unsequenced(const std::vector<unsigned char>& bytes)
    {
        for(auto c : connections)
            c->send_unsequenced(bytes);
    }

    void send_sequenced(const std::vector<unsigned char>& bytes)
    {
        for(auto c : connections)
            c->send_sequenced(bytes);
    }

    private:
    // boost asio
    void do_accept()
    {
        acceptor->async_accept([this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
            if (!ec)
                connections.emplace_back(std::make_shared<CONNECTION>(std::move(socket)));
            if (!shuttingDown)
                do_accept();
        });
    }

    protected:
    std::vector<std::shared_ptr<CONNECTION> > connections;
    SoupBinLoginVerifier* loginVerifier = nullptr;
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::acceptor* acceptor;
    std::thread runThread;
    bool shuttingDown = false;
};