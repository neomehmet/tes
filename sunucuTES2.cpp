#include <iostream>
#include "TES2.h"
#include <cstring>
#include <fstream>
int main() {
    const char* serverIp = "127.0.0.1"; // Sunucu IP adresini buraya girin
    const char* clientIp = "127.0.0.1"; // Client IP adresini buraya girin
    uint16_t serverPort = 59345; // Sunucu port numarasını buraya girin
    uint16_t clientPort = 59321; // Client port numarasını buraya girin

    const char* udpData = "Merhaba, istemci!"; // Başlangıçta gönderilecek UDP verisi

    // TES_SRV nesnesi oluşturun ve başlangıç değerlerini ayarlayın
    TES_SRV server(serverIp, clientIp, serverPort, clientPort, udpData);

    // Checksum hesaplamayı isterseniz:
    // server.CalculateChecksum();

    // Sunucunun çalışmasını istediğiniz sürece döngüyü sürdürün
    bool running = true;
    std::ofstream dosya("alinanbooka.txt",std::ios::binary);
    while (running) {
        // İstemciden gelen veriyi al
        server.ReceivePacket();

        // Alınan veriyi kontrol edin, eğer "exit" ise döngüyü sonlandırın
        if (std::strcmp(server.GetUDPData(), "exit") == 0) {
            running = false;
        } else {
           // std::cout << "yazilmis objeden okunan : " << server.GetUDPData() << std::endl ;
            dosya << server.GetUDPData() << std::endl;

        }
        if (std::strcmp(udpData, "exit") == 0) {
            dosya.close();
            running = false;
            break;
        }
    }

    return 0;
}
