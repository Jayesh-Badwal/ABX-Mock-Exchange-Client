#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdexcept>
#include <functional>
#include "defines.h"

#define MAX_LEN 4096

class TCPClient
{
public:
    using ResponseHandler = std::function<void(char *, ssize_t)>;

    TCPClient(const std::string &_serverIp, uint16_t _serverPort, ResponseHandler _handler)
        : m_sock_fd(-1), m_server_ip(_serverIp), m_server_port(_serverPort), m_response_handler(std::move(_handler))
    {
        createSocket();
    }

    ~TCPClient()
    {
    }

    void connectToServer()
    {
        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(m_server_port);
        if (inet_pton(AF_INET, m_server_ip.c_str(), &server_addr.sin_addr) <= 0)
            throw std::runtime_error("Invalid IP address");

        if (connect(m_sock_fd, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)) < 0)
            throw std::runtime_error("Connection failed: " + std::string(strerror(errno)));
        else
            std::cout << "Connection established with server.\n";
    }

    void sendRequest(RequestPayload &req)
    {
        ssize_t sent = send(m_sock_fd, (void *)&req, sizeof(req), 0);
        if (sent < 0)
            throw std::runtime_error("Send failed: " + std::string(strerror(errno)));
    }

    void receiveResponse()
    {
        while (true)
        {
            char buffer[MAX_LEN + 1];
            ssize_t received = recv(m_sock_fd, buffer, MAX_LEN, 0);
            if (received < 0)
                throw std::runtime_error("Receive failed: " + std::string(strerror(errno)));
            else if (received == 0)
            {
                std::cout << "Connection closed by server.\n";
                return;
            }

            buffer[received] = '\0';
            if (m_response_handler)
                m_response_handler(buffer, received);
        }
    }

    void closeConnection()
    {
        if (m_sock_fd != -1)
            close(m_sock_fd);
    }

private:
    int m_sock_fd;
    std::string m_server_ip;
    uint16_t m_server_port;
    ResponseHandler m_response_handler;

    void createSocket()
    {
        m_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (m_sock_fd < 0)
            throw std::runtime_error("Socket creation failed");

        int opt = 1;

        // Disable Nagle's algorithm
        if (setsockopt(m_sock_fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) < 0)
            throw std::runtime_error("Failed to set TCP_NODELAY");

        // Reuse address
        if (setsockopt(m_sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
            throw std::runtime_error("Failed to set SO_REUSEADDR");

        // Reuse port
        if (setsockopt(m_sock_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
            throw std::runtime_error("Failed to set SO_REUSEPORT");
    }
};
