#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")

const int PORT = 1145;
const std::string MESSAGE_CACHE_FILE = "message_cache.txt";

class MessageCacher {
public:
    void loadMessages() {
        std::ifstream cacheFile(MESSAGE_CACHE_FILE);
        std::string line;
        while (std::getline(cacheFile, line)) {
            messages.push_back(line);
        }
    }

    void saveMessage(const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex);
        messages.push_back(message);
        std::ofstream cacheFile(MESSAGE_CACHE_FILE, std::ios::app);
        cacheFile << message << std::endl;
    }

    const std::vector<std::string>& getMessages() {
        return messages;
    }

private:
    std::vector<std::string> messages;
    std::mutex mutex;
};

void receiveMessages(SOCKET sock, MessageCacher& cacher) {
    char buffer[1024];

    while (true) {
        int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            break; // Server disconnected
        }
        buffer[bytesReceived] = '\0';
        std::string message(buffer);
        cacher.saveMessage(message); // Store received message

        std::cout << "Cached: " << message << std::endl; // Output cached message
    }
}

void sendCachedMessages(SOCKET sock, MessageCacher& cacher) {
    for (const auto& message : cacher.getMessages()) {
        std::string msgWithNewline = message + "\n"; // Add newline
        send(sock, msgWithNewline.c_str(), msgWithNewline.size(), 0);
    }
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    MessageCacher cacher;
    cacher.loadMessages(); // Load existing messages

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(PORT);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection to server failed!" << std::endl;
        return 1;
    }

    std::thread receiveThread(receiveMessages, sock, std::ref(cacher));
    receiveThread.detach();

    // After connecting, send cached messages to server
    sendCachedMessages(sock, cacher);

    // Keep the program running to receive messages
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
