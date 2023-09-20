#ifndef TES2_H
#define TES2_H

#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/in.h>
#include <cstdint>

class TES_SRV {
public:
    TES_SRV();
    TES_SRV(const char* sourceIp, const char* destIp, uint16_t sourcePort, uint16_t destPort, const char* udpData);
    ~TES_SRV();

    void SetIPHeader(const char* sourceIp, const char* destIp);
    void SetUDPHeader(uint16_t sourcePort, uint16_t destPort, const char* udpData);
    void SetTargetAddress(const char* destIp, uint16_t destPort);
    void CalculateChecksum();
    void SendPacket();
    void SetData(const char* newData);
    void ReceivePacket();
    const char* GetUDPData();


private:
    struct ip ipHeader_;
    struct udphdr udpHeader_;
    char udpData_[1500];
    sockaddr_in targetAddress_;

};

#endif // TES_SRV_H
