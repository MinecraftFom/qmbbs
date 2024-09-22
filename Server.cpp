#include <iostream>
#include <winsock2.h>
#include <vector>
#include <thread>
#include <string>
#include <mutex>
#include <algorithm>
#include <deque>

#pragma comment(lib, "ws2_32.lib")

#define PORT 1145
#define BUFFER_SIZE 1024
#define MAX_HISTORY 200

std::vector<SOCKET> clients;
std::mutex clientsMutex;
std::deque<std::string> messageHistory;  // 存储消息历史记录

void say_word(const char* _animetgt) {
    std::cout << _animetgt;
    std::cout.flush();
}

SOCKET createServerSocket() {
    WSADATA wsaData;
    SOCKET serverSocket;
    struct sockaddr_in serverAddr;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock. Error Code: " << WSAGetLastError() << std::endl;
        exit(EXIT_FAILURE);
    }

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cerr << "Could not create socket. Error Code: " << WSAGetLastError() << std::endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error code: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 3) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error code: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    say_word("Server is Listening on Port: ");
    std::cout << PORT << std::endl;
    return serverSocket;
}

void broadcastMessage(const std::string& message, SOCKET sender) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (SOCKET client : clients) {
        if (client != sender) {
            send(client, message.c_str(), message.length(), 0);
        }
    }
}

void sendMessageHistory(SOCKET clientSocket) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (const auto& message : messageHistory) {
        send(clientSocket, message.c_str(), message.length(), 0);
    }
}

void handleClient(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE] = {0};

    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.push_back(clientSocket);
    }

    // 发送历史消息
    sendMessageHistory(clientSocket);

    while (true) {
        int recvSize = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (recvSize <= 0) {
            break; // 客户端断开连接
        }
        buffer[recvSize] = '\0';
        say_word(buffer);
        std::cout << std::endl;

        // 将消息存储到历史记录
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            std::string message(buffer);

            // 根据需要添加换行符
            if (message.find("\n") != std::string::npos) {
                message += "\n"; // 添加换行符
            }

            messageHistory.push_back(message);
            if (messageHistory.size() > MAX_HISTORY) {
                messageHistory.pop_front();  // 保持最大历史记录数量
            }
        }

        // 广播消息给其他客户端
        broadcastMessage(buffer, clientSocket);
    }

    // 从客户端列表中移除
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
    }
    closesocket(clientSocket);
}

void acceptClientConnection(SOCKET serverSocket) {
    while (true) {
        SOCKET clientSocket;
        struct sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);

        if ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen)) == INVALID_SOCKET) {
            std::cerr << "Accept failed with error code: " << WSAGetLastError() << std::endl;
            continue;
        }

        say_word("A New Client Connected\n");
        std::thread(handleClient, clientSocket).detach();
    }
}

int main() {
    SOCKET serverSocket = createServerSocket();
    acceptClientConnection(serverSocket);
    return 0;
}
