#include <iostream>
#include <winsock2.h>
#include <thread>
#include <string>
#include <vector>
#include <fstream>

#pragma comment(lib, "ws2_32.lib")  // 链接 Winsock 库

#define PORT 1145
#define BUFFER_SIZE 1024

void sendMessage(SOCKET sock, const std::string& username) {
    while (true) {
        std::string message;
        std::cout << "(MSG):      ";
        std::getline(std::cin, message); // 使用 getline 来读取整行
        if (!message.empty()) {
            std::string fullMessage = "[" + username + "] " + message + "\n";
            if (send(sock, fullMessage.c_str(), fullMessage.length(), 0) == SOCKET_ERROR) {
                std::cerr << "Send failed with error code: " << WSAGetLastError() << std::endl;
                break;
            }
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <server_ip>" << std::endl;
        return 1;
    }

    // 读取用户名
    std::ifstream fu(".username");
    std::string username;
    if (fu.is_open()) {
        std::getline(fu, username);
        fu.close();
    } else {
        std::cerr << "Unable to open .username file." << std::endl;
        return 1;
    }

    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in serverAddr;

    // 初始化 Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock. Error Code: " << WSAGetLastError() << std::endl;
        return 1;
    }

    // 创建套接字
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cerr << "Could not create socket. Error Code: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // 配置服务器地址
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]); // 服务器的 IP 地址

    // 连接到服务器
    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connect failed with error code: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server." << std::endl;

    // 发送消息
    sendMessage(sock, username);

    // 关闭套接字和清理 Winsock
    closesocket(sock);
    WSACleanup();
    return 0;
}