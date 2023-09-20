#include <iostream>
#include <cstring>
#include "TES2.h"
#include <fstream>
#include <vector>
#include <fstream>
#include <vector>

#define BLOCK_SIZE 1450

int main() {
    const char* serverIp = "127.0.0.1"; // Sunucu IP adresini buraya girin
    const char* clientIp = "127.0.0.1"; // Client IP adresini buraya girin
    uint16_t serverPort = 59345; // Sunucu port numarasını buraya girin
    uint16_t clientPort = 59321; // Client port numarasını buraya girin

    // TES_SRV nesnesi oluşturun ve başlangıç değerlerini ayarlayın
    TES_SRV client(clientIp, serverIp, clientPort, serverPort, "");

    // Checksum hesaplamayı isterseniz:
    // client.CalculateChecksum();

    char* udpData = new char[BLOCK_SIZE];

///////////////////////////
///////////////////////////
    std::ifstream dosya("book.txt", std::ios::binary); // Okunacak dosyanın adı
    if (!dosya) {
        std::cerr << "Dosya acilamadi." << std::endl;
        return 1;
    }

    char buffer[BLOCK_SIZE];
    std::vector<char> veriVektoru;
    int i =0;
    while (dosya.read(buffer, BLOCK_SIZE)) {
        veriVektoru.insert(veriVektoru.end(), buffer, buffer + BLOCK_SIZE);
    }
    // Dosyadan okunan veriler artık veriVektoru içindedir.

    dosya.close();

//////////////////////////
//////////////////////////
    while (true) {
   //     std::cout << std::endl << "gonderilecek datayi gir : " ;
  //      std::cin.getline(udpData, sizeof(udpData));


        for (int i = 0; i < veriVektoru.size(); i += BLOCK_SIZE) {
            char* blocks = new char[BLOCK_SIZE] ;
            for (int j = 0; j < BLOCK_SIZE; j++) {
                *(blocks+j) = veriVektoru[i + j] ;
            }
            // İstemciye veriyi ayarla
            client.SetData(blocks);

            // Veriyi sunucuya gönder
            client.SendPacket();
            
            std::memset(blocks, 0, sizeof(blocks));
           // std::cout << "gonderilecek veri " << udpData ;

            /*if (std::strcmp(udpData, "exit") == 0) {
                break;
            }*/
        
        }

        client.SetData("exit");
        client.SendPacket();

    }

    return 0;
}
