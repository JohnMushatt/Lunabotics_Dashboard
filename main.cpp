#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <vector>
#include <fstream>
#include <chrono>

int main() {
    int server_fd, new_socket;
    //long valread;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    pthread_t tid;
    int addrlen = sizeof(server_address);
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("In socket");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "130.215.249.199", &server_address.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return EXIT_FAILURE;
    }
    server_address.sin_port = htons(9001);

    memset(server_address.sin_zero, '\0', sizeof server_address.sin_zero);
    signal(SIGPIPE, SIG_IGN);
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *) &reuse,
                   sizeof(reuse)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
    }
#ifdef SO_REUSEPORT
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, (const char*) &reuse,
            sizeof(reuse)) < 0) {
        perror("setsockopt(SO_REUSEPORT) failed");
    }
#endif

    if (bind(server_fd, (struct sockaddr *) &server_address,
             sizeof(server_address)) < 0) {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 20) < 0) {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    socklen_t client_size = sizeof(client_address);
    //If new client is connected
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *) &client_address,
                                 (socklen_t *) &client_size)) < 0) {
            perror("In accept");
            exit(EXIT_FAILURE);
        }
        printf("Pi connected\n");
        std::vector<char> buff(4096);
        int64_t incoming_file_size = recv(new_socket, &buff.at(0), buff.size(), 0);
        int64_t file_size = std::atoi(&buff.at(0));

        std::cout << "[+] Incoming file size: " << file_size << " bytes" << std::endl;
        buff.resize(file_size);
        if (buff.capacity() != file_size) {
            std::cout << "[-] Failed to resize buffer" << std::endl;
        }
        int64_t file_bytes_rec = 0;
        //buff.clear();
        while (file_bytes_rec < file_size) {
            file_bytes_rec += recv(new_socket, &buff.at(file_bytes_rec), buff.size(), 0);
            std::cout << "[+] Received packet size: " << file_bytes_rec << " bytes" << std::endl;
            std::cout << "[+] Remaining bytes to receive: " << file_size - file_bytes_rec << " bytes" << std::endl;
        }
        std::cout << "[+] Writing " << file_size << " bytes to file" << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        std::ofstream FILE("../framedata/frame_1.ply", std::ios::out | std::ofstream::binary);
        std::copy(buff.begin(), buff.end(), std::ostreambuf_iterator<char>(FILE));

        auto end = std::chrono::high_resolution_clock::now();
        double elapsed_seconds = std::chrono::duration_cast<
                std::chrono::duration<double> >(end - start).count();
        std::cout << "[+] File write time: " << elapsed_seconds << " seconds" << std::endl;
    }

    return EXIT_SUCCESS;

}
