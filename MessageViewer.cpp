#include <iostream>
#include <winsock2.h>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>
#include <string>

#pragma comment(lib, "ws2_32.lib")

#define PORT 1145
#define BUFFER_SIZE 1024

void extract_letters(const char* input, char* output) {
    std::vector<char> letters;
    for (int i = 0; input[i] != '\0'; ++i) {
        if (std::isalpha(static_cast<unsigned char>(input[i]))) {
            letters.push_back(input[i]);
        }
    }
    int length = letters.size();
    for (int i = 0; i < length; ++i) {
        output[i] = letters[i];
    }
    output[length] = '\0';
}

void say_word(const char* _animetgt) {
    std::cout << _animetgt;
    std::cout.flush();
}

class MessageViewer {
public:
    void addMessage(const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        // 只添加新消息
        if (messages_.empty() || messages_.back() != message) {
            messages_.push_back(message);
        }
    }

    void displayMessages() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& msg : messages_) {
            say_word(msg.c_str());
            std::cout <<  std::endl;
        }
        // 清空消息列表，避免重复显示
        messages_.clear();
    }

private:
    std::vector<std::string> messages_;
    std::mutex mutex_;
};

void receiveMessages(SOCKET sock, MessageViewer& viewer) {
    char buffer[BUFFER_SIZE];
    while (true) {
        int recvSize = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (recvSize == SOCKET_ERROR) {
            std::cerr << "Recv failed with error code: " << WSAGetLastError() << std::endl;
            break;
        } else if (recvSize == 0) {
            std::cout << "Connection closed by server." << std::endl;
            break;
        }
        buffer[recvSize] = '\0';
        viewer.addMessage(std::string(buffer));
    }
}

void displayMessagesThread(MessageViewer& viewer) {
    while (true) {
        viewer.displayMessages();
        std::this_thread::sleep_for(std::chrono::seconds(5)); // 每5秒显示一次消息
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <server_ip>" << std::endl;
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

    MessageViewer messageViewer;

    try {
        // 创建线程接收消息
        std::thread receiver(receiveMessages, sock, std::ref(messageViewer));
        receiver.detach();

        // 创建线程显示消息
        std::thread viewerThread(displayMessagesThread, std::ref(messageViewer));
        viewerThread.detach();

        // 主线程保持活跃，等待用户退出
        std::string input;
        say_word("Press Enter to exit...");
        std::cout <<  std::endl;
        std::getline(std::cin, input);

    } catch (const std::system_error& e) {
        std::cerr << "System error: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "An unexpected error occurred." << std::endl;
    }

    // 关闭套接字和清理 Winsock
    closesocket(sock);
    WSACleanup();
    return 0;
}
