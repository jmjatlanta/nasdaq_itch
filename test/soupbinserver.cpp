#include <gtest/gtest.h>
#include "soup_bin_server.h"
#include "soup_bin_client.h"
#include <thread>


TEST(SoupBinServer, timer)
{
    class MyClass : public TimerListener
    {
        public:
        MyClass() : timer(this, 1000, Timer::get_time())
        {
        }

        virtual void OnTimer(uint64_t msSince)
        {
            numFires++;
        }
        uint16_t numFires = 0;
        Timer timer;
    };

    MyClass myClass;
    // check the timer, it should not have gone off
    EXPECT_EQ(myClass.numFires, 0);
    // wait for half of the timeout and then reset
    uint64_t start = Timer::get_time();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_EQ(myClass.numFires, 0);
    myClass.timer.reset();
    // now wait for the other half of the first timeout
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_EQ(myClass.numFires, 0);
    // now wait for the other half of the second timeout (plus a little)
    std::this_thread::sleep_for(std::chrono::milliseconds(510));
    EXPECT_EQ(myClass.numFires, 1);
}

TEST(SoupBinServer, OneClient)
{
    class MyConnection : public SoupBinConnection
    {
        public:
        MyConnection(boost::asio::ip::tcp::socket socket) : SoupBinConnection(std::move(socket)) {}
        MyConnection(const std::string& url, const std::string& username, const std::string& password) 
                : SoupBinConnection(url, username, password) {}
        virtual void on_login_request(const soupbintcp::login_request& in) override
        {
            soupbintcp::login_accepted msg;
            msg.set_int(soupbintcp::login_accepted::SEQUENCE_NUMBER, 0);
            msg.set_string(soupbintcp::login_accepted::SESSION, "ABC");
            send(msg.get_record_as_vec());
        }
        virtual void on_client_heartbeat(const soupbintcp::client_heartbeat& in) override
        {
            numClientHeartbeats++;
        }
        virtual void on_server_heartbeat(const soupbintcp::server_heartbeat& in) override
        {
            numServerHeartbeats++;
        }
        uint32_t numClientHeartbeats = 0;
        uint32_t numServerHeartbeats = 0;
    };
    class MySoupBinServer : public SoupBinServer<MyConnection>
    {
        public:
        MySoupBinServer(uint32_t port) : SoupBinServer(port)
        {
        }
        uint32_t GetNumClientHeartbeats()
        {
            uint32_t total = 0;
            for(auto c : connections)
                total += c->numClientHeartbeats;
            return total;
        }
        uint32_t GetNumServerHeartbeats()
        {
            uint32_t total = 0;
            for(auto c : connections)
                total += c->numServerHeartbeats;
            return total;
        }
    };
    class MySoupBinClient
    {
        public:
        MySoupBinClient(const std::string& url, const std::string& user, const std::string& pw)
                : connection(url, user, pw)
        {
        }
        uint32_t GetNumClientHeartbeats()
        {
            return connection.numClientHeartbeats;
        }
        uint32_t GetNumServerHeartbeats()
        {
            return connection.numServerHeartbeats;
        }
        MyConnection connection;
        boost::asio::io_context io_context;
    };

    MySoupBinServer server(9012);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    MySoupBinClient client("127.0.0.1:9012", "test1", "password");
    // wait 2.5 seconds
    std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    // we should have sent 1 heartbeat request and received 1 heartbeat response
    EXPECT_EQ(server.GetNumClientHeartbeats(), 1);
    EXPECT_EQ(server.GetNumServerHeartbeats(), 0);
    EXPECT_EQ(client.GetNumClientHeartbeats(), 0);
    EXPECT_EQ(client.GetNumServerHeartbeats(), 1);
    // if the server sends data, it should not bother sending heartbeat requests
    // the client always sends heartbeat requests
}

TEST(SoupBinServer, ServerStartStop)
{
    class MyConnection : public SoupBinConnection
    {
        public:
        MyConnection(boost::asio::ip::tcp::socket socket) : SoupBinConnection(std::move(socket)) {}
        MyConnection(const std::string& url, const std::string& username, const std::string& password) 
                : SoupBinConnection(url, username, password) {}
        virtual void on_login_request(const soupbintcp::login_request& in) override
        {
            soupbintcp::login_accepted msg;
            msg.set_int(soupbintcp::login_accepted::SEQUENCE_NUMBER, 0);
            msg.set_string(soupbintcp::login_accepted::SESSION, "ABC");
            send(msg.get_record_as_vec());
        }
        virtual void on_client_heartbeat(const soupbintcp::client_heartbeat& in) override
        {
            numClientHeartbeats++;
        }
        virtual void on_server_heartbeat(const soupbintcp::server_heartbeat& in) override
        {
            numServerHeartbeats++;
        }
        uint32_t numClientHeartbeats = 0;
        uint32_t numServerHeartbeats = 0;
    };
    class MySoupBinServer : public SoupBinServer<MyConnection>
    {
        public:
        MySoupBinServer(uint32_t port) : SoupBinServer(port)
        {
        }
        virtual ~MySoupBinServer() {}
        uint32_t GetNumClientHeartbeats()
        {
            uint32_t total = 0;
            for(auto c : connections)
                total += c->numClientHeartbeats;
            return total;
        }
        uint32_t GetNumServerHeartbeats()
        {
            uint32_t total = 0;
            for(auto c : connections)
                total += c->numServerHeartbeats;
            return total;
        }
    };
    MySoupBinServer server(9012);
    std::this_thread::sleep_for(std::chrono::seconds(2));
}