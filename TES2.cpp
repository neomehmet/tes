#include "TES2.h"
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <unistd.h>

TES_SRV::TES_SRV() {
    // Varsayılan kurucu işlev, başlangıç değerlerini ayarlar
    std::memset(&ipHeader_, 0, sizeof(struct ip));
    std::memset(&udpHeader_, 0, sizeof(struct udphdr));
    std::memset(&targetAddress_, 0, sizeof(sockaddr_in));
}

TES_SRV::TES_SRV(const char* sourceIp, const char* destIp, uint16_t sourcePort, uint16_t destPort, const char* udpData) {
    // Parametreli kurucu işlev, verilen değerleri kullanarak başlangıç değerlerini ayarlar
    SetIPHeader(sourceIp, destIp);
    SetUDPHeader(sourcePort, destPort, udpData);
    SetTargetAddress(destIp, destPort);
}

TES_SRV::~TES_SRV() {
    // Destructor, gerekirse bellek temizliği veya kaynakları serbest bırakma işlemlerini gerçekleştirebilir
}

void TES_SRV::SetIPHeader(const char* sourceIp, const char* destIp) {
    // IP başlığını ayarlayan işlev
    ipHeader_.ip_v = 4; // IPv4
    ipHeader_.ip_hl = 5; // 5x4 = 20 byte başlık uzunluğu
    ipHeader_.ip_tos = 0; // Type of Service (hizmet türü)
    ipHeader_.ip_id = 0; // Paket kimliği (0 olarak ayarlandı)
    ipHeader_.ip_off = 0; // Parçalanmış paketler için offset (0 olarak ayarlandı)
    ipHeader_.ip_ttl = 64; // Time to Live (TTL)
    ipHeader_.ip_p = IPPROTO_UDP; // Üst katman protokolü (UDP)
    ipHeader_.ip_sum = 0; // Kontrol toplamı (0 olarak ayarlandı)
    ipHeader_.ip_src.s_addr = inet_addr(sourceIp); // Kaynak IP adresi
    ipHeader_.ip_dst.s_addr = inet_addr(destIp); // Hedef IP adresi
}

void TES_SRV::SetUDPHeader(uint16_t sourcePort, uint16_t destPort, const char* udpData) {
    // UDP başlığını ayarlayan işlev
    udpHeader_.uh_sport = htons(sourcePort); // Kaynak port numarası
    udpHeader_.uh_dport = htons(destPort); // Hedef port numarası
    udpHeader_.uh_ulen = htons(sizeof(struct udphdr) + std::strlen(udpData)); // UDP datagramının toplam uzunluğu
    udpHeader_.uh_sum = 0; // Kontrol toplamı (0 olarak ayarlandı)
    std::strcpy(udpData_, udpData);
}

void TES_SRV::SetTargetAddress(const char* destIp, uint16_t destPort) {
    // Hedef adres bilgilerini ayarlayan işlev
    targetAddress_.sin_family = AF_INET;
    targetAddress_.sin_port = htons(destPort); // Hedef port numarası
    targetAddress_.sin_addr.s_addr = inet_addr(destIp); // Hedef IP adresi
}

void TES_SRV::SetData(const char* newData) {
    std::memset(this->udpData_, 0, 1500); // Belleği sıfırla

//    std::cout << "SetData and updated data :" << this->udpData_ << "<-yani bura bos olmali " <<std::endl;
//    std::cout << "SetData fonksiyonunag gelen data and new data : " <<  newData << std::endl;

    if (newData != nullptr) {
        size_t newDataLength = std::strlen(newData);
        if (newDataLength < 1500) {
            std::memcpy(udpData_, newData, newDataLength); // Yeni veriyi kopyala
        } else {
            std::cerr << "Hata: Yeni veri çok büyük!" << std::strlen(newData) << std::endl;
        }
    } else {
        std::cerr << "Hata: Geçersiz yeni veri!" << std::endl;
    }
}

const char* TES_SRV::GetUDPData() {
    return udpData_;
}


// Checksum hesaplama fonksiyonu
void TES_SRV::CalculateChecksum() {
    // UDP başlığı için kontrol toplamını hesaplayan işlev

    // İlk önce checksum alanını sıfırlayın
    udpHeader_.uh_sum = 0;

    // UDP başlığı ve verisi için toplam uzunluğu hesaplayın
    uint32_t totalLength = sizeof(struct udphdr) + strlen(udpData_);

    // Pseudo UDP başlığı oluşturun
    struct pseudo_udp_header {
        uint32_t source_ip;
        uint32_t dest_ip;
        uint8_t zero;
        uint8_t protocol;
        uint16_t udp_length;
    } pseudo_header;

    pseudo_header.source_ip = ipHeader_.ip_src.s_addr;
    pseudo_header.dest_ip = ipHeader_.ip_dst.s_addr;
    pseudo_header.zero = 0;
    pseudo_header.protocol = ipHeader_.ip_p;
    pseudo_header.udp_length = htons(totalLength);

    // UDP başlığı ve verisini bir araya getirin
    uint8_t* buffer = new uint8_t[totalLength];
    std::memcpy(buffer, &pseudo_header, sizeof(pseudo_header));
    std::memcpy(buffer + sizeof(pseudo_header), &udpHeader_, sizeof(struct udphdr));
    std::memcpy(buffer + sizeof(pseudo_header) + sizeof(struct udphdr), udpData_, strlen(udpData_));

    // Checksum hesaplamasını yapın
    uint16_t* p = reinterpret_cast<uint16_t*>(buffer);
    size_t length = totalLength + sizeof(pseudo_header);
    uint32_t sum = 0;
    while (length > 1) {
        sum += *p++;
        if (sum & 0x80000000) {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }
        length -= 2;
    }
    if (length) {
        sum += *reinterpret_cast<uint8_t*>(p);
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    // Sonuc olarak checksumi ayarlama ve tersleme 
    udpHeader_.uh_sum = ~static_cast<uint16_t>(sum);

    delete[] buffer;
}

// Ağ paketini gönderen fonksiyon
void TES_SRV::SendPacket() {
    // sockfd ile soket oluşturun, sendto ile paketi gönderin, hataları kontrol edin

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sockfd == -1) {
        perror("Soket oluşturma hatası");
        return;
    }

    // Hedef adres bilgilerini kullanarak sockaddr_in targetAddress_ 'ı kullanabiliriz
    ssize_t bytes_sent = sendto(sockfd, &ipHeader_, sizeof(struct ip) + sizeof(struct udphdr) + strlen(udpData_), 0, (struct sockaddr*)&targetAddress_, sizeof(targetAddress_));
    // yukardaki fonksiyonda gariplik var gibi gelebilir neden udp datasını gondermedik diyebilisiniz fakat
    // ipHeader ın adresini gonderirken buffer sizeda tum objenın boyutunu gonderiyoruz ;) 

    if (bytes_sent == -1) {
        perror("Gönderme hatası");
        close(sockfd); // Soketi kapat
        return;
    }

   // std::cout << "Ağ paketi gönderildi : veri ==" << this->udpData_ << std::endl;

    std::memset(udpData_, 0, 1500); // Belleği sıfırla
    close(sockfd); // Soketi kapat
}

void TES_SRV::ReceivePacket() {
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sockfd == -1) {
        perror("Soket oluşturma hatası");
        return;
    }
    //std::memset(udpData_, 0, 1500); // Belleği sıfırla
    char buffer[1500]; // bunun icini temizlemezsek daha onceki baska veriler olabilir asagida temizlemek icin memset yapiliyor
    std::memset(buffer, 0, 1500); // Belleği sıfırla
    
    ssize_t bytes_received = recv(sockfd, buffer, sizeof(buffer), 0);

    if (bytes_received == -1) {
        perror("Alma hatası");
        close(sockfd);
        return;
    }

    if (bytes_received > 0) {
        struct udphdr* udpHeader = reinterpret_cast<struct udphdr*>(buffer + sizeof(struct ip));
        char* udpData = buffer + sizeof(struct ip) + sizeof(struct udphdr);

/*     std::cout << "UDP Başlığı: Kaynak Port " << ntohs(udpHeader->uh_sport)
                  << ", Hedef Port " << ntohs(udpHeader->uh_dport)
                  << ", Uzunluk " << ntohs(udpHeader->uh_ulen)
                  << ", Checksum " << std::hex << ntohs(udpHeader->uh_sum) << std::dec
                  << std::endl;
*/
      //  std::cout << "UDP gelen Verisi: " << udpData << std::endl;

        // Aldığınız UDP verisini udpData_ üyesine kaydedin
        SetData(udpData);
    }

    close(sockfd);
}
