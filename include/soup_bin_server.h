#pragma once
#include <atomic>
#include <cstdint>
#include <vector>

class Timer
{
    public:
    Timer(currentTimeMs);
    void reset();
    private:
    uint64_t lastTimeMs;
};

/***
 * Represents a connected client
*/
class SoupBinConnection
{
    public:
    SoupBinConnection();

    /***
     * Stores message for repeats, plus sends it
    */
    void send_sequenced(uint64_t seqNo, const std:vector<unsigned char>& bytes);
    void send_sequenced(const std::vector<unsigned char>& bytes);
    void send_unsequenced(const std::vector<unsigned char>& bytes);
    uint64_t get_next_seq() { return nextSeq++; }

    private:
    std::atomic<uint64_t> nextSeq = 1;
    Timer heartbeatTimer; // fires off a heartbeat packet if nothing sent for 1 minute
};

/***
 * A SoupBin server that listens on a socket
*/
class SoupBinServer
{
    public:
    SoupBinServer(int32_t listenPort);

    void send(const std::vector<unsigned char>& bytes);

    private:
    std::vector<std::shared_ptr<SoupBinConnection> connections;
};