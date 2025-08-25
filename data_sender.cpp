// SPDX-License-Identifier: BSL-1.0
/*!
 * @author Adrian Przekwas <adrian.v.przekwas@gmail.com>
 */


#include "data_sender.h"

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> // for close socket

DataSender::DataSender() {
    sockfd = -1;
}

int DataSender::openSocket(void) {
    if (isSocketOpened()){
        closeSocket();
    }

    const char* server_ip = "127.0.0.1"; // localhost

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Socked creation failed!" << std::endl;
        return -1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(MONADO_PORT);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        std::cerr << "Adress conversion error!" << std::endl;
        closeSocket();
        return -1;
    }

    if (connect(sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Server connection error!" << std::endl;
        closeSocket();
        return -1;
    }
    return 0;
}

bool DataSender::isSocketOpened(void){
    if (sockfd >= 0)
        return true;
    else
        return false;
}

void DataSender::closeSocket(void){
    if (isSocketOpened()) {
        close(sockfd);
        sockfd = -1;
    }
}

int DataSender::sendData(const r_remote_data& data){
    if (!isSocketOpened()){
        return -1;
    }
    // ssize_t send(int socket, const void *buffer, size_t length, int flags);
    if (send(sockfd, &data, sizeof(data), 0) < 0) {
        std::cerr << "Data sending error!" << std::endl;
        return -1;
    }
    return 0;
}

DataSender::~DataSender(){
    closeSocket();
    std::cout << "Connection closed" << std::endl;
}
