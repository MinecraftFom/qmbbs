/*#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include <iostream>
#include <mutex>
#include <fstream>
#include <atomic>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

// 全局变量
HWND hOutputBox, hInputBox, hSendButton;
SOCKET sock;
std::atomic<bool> keepRunning(true); // 使用原子变量控制线程运行
std::mutex outputMutex;

std::string username; // 从文件中读取的用户名
const std::string usernameFile = ".username"; // 用户名文件

// 函数声明
void sendMessage(const std::string& message);
void receiveMessages();
void appendToOutput(const std::wstring& message);
void loadUsername();
void initSocket(const char* address);

// 函数定义
void loadUsername() {
    std::ifstream file(usernameFile);
    if (file.is_open()) {
        std::getline(file, username); // 从文件中读取用户名
        file.close();
    }
    if (username.empty()) {
        username = "Guest";
    }
}

void appendToOutput(const std::wstring& message) {
    std::lock_guard<std::mutex> lock(outputMutex);
    // 增加缓冲区的大小以容纳更多消息
    std::vector<wchar_t> buffer(4096); // 例如，将缓冲区大小增加到 4096
    GetWindowTextW(hOutputBox, buffer.data(), buffer.size());

    std::wstring ws(buffer.data());
    // 确保消息末尾有换行符
    if (!ws.empty() && ws.back() != L'\n') {
        ws += L"\r\n";
    }
    // 追加新消息
    ws += message;

    // 设置窗口文本
    SetWindowTextW(hOutputBox, ws.c_str());

    // 滚动到底部
    SendMessage(hOutputBox, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
    SendMessage(hOutputBox, EM_SCROLL, (WPARAM)SB_BOTTOM, 0);
}

void sendMessage(const std::string& message) {
    std::string fullMessage = "[" + username + "] " + message;
    if (message.back() != '\r') {
        fullMessage += "\r\n";
    }
    send(sock, fullMessage.c_str(), fullMessage.length(), 0);
    // 显示自己发送的消息
    appendToOutput(L"Me: " + std::wstring(fullMessage.begin(), fullMessage.end()));
}

void receiveMessages() {
    char buffer[1024];
    while (keepRunning) {
        int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::string receivedMessage(buffer);
            if (receivedMessage.back() != '\r') {
                receivedMessage += "\r\n";
            }
            appendToOutput(std::wstring(receivedMessage.begin(), receivedMessage.end()));
        } else if (bytesReceived == 0) {
            appendToOutput(L"Server disconnected.\r\n");
            keepRunning = false;
        } else {
            int error = WSAGetLastError();
            appendToOutput(std::wstring(L"Receive failed: ") + std::to_wstring(error) + L"\r\n");
            keepRunning = false;
        }
    }
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
    WSACleanup();
}

void initSocket(const char* address) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        appendToOutput(L"Failed to create socket.\r\n");
        keepRunning = false;
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1145); // 端口号
    serverAddr.sin_addr.s_addr = inet_addr(address);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        appendToOutput(L"Connection failed. Retrying...\r\n");
        closesocket(sock);
        sock = INVALID_SOCKET;
        keepRunning = false;
        return;
    }

    appendToOutput(L"Connected to server.\r\n");
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            hInputBox = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
                                       10, 10, 260, 60, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hOutputBox = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_MULTILINE | ES_AUTOVSCROLL,
                                        10, 80, 260, 200, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hSendButton = CreateWindow("BUTTON", "SEND", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                       10, 290, 100, 30, hwnd, (HMENU)1, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            break;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == 1) { // SEND button
                char buffer[256];
                GetWindowTextA(hInputBox, buffer, sizeof(buffer));
                std::string message(buffer);
                if (!message.empty()) {
                    sendMessage(message); // 发送消息
                    SetWindowTextA(hInputBox, ""); // 清空输入框
                }
            }
            break;
        case WM_DESTROY:
            keepRunning = false; // 告诉后台线程停止运行
            PostQuitMessage(0);
            break;
        case WM_SIZE: {
            RECT rc;
            GetClientRect(hwnd, &rc);
            int width = rc.right - rc.left;
            int height = rc.bottom - rc.top;

            MoveWindow(hInputBox, 10, 10, width - 20, 60, TRUE);
            MoveWindow(hOutputBox, 10, 80, width - 20, height - 120, TRUE);
            MoveWindow(hSendButton, 10, height - 40, 100, 30, TRUE);
        }
            break;
        case WM_MOUSEWHEEL: {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            if (delta > 0) {
                SendMessage(hOutputBox, WM_VSCROLL, SB_LINEUP, 0); // 向上滚动
            } else {
                SendMessage(hOutputBox, WM_VSCROLL, SB_LINEDOWN, 0); // 向下滚动
            }
            return 0;
        }
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nShowCmd) {
    loadUsername(); // 从文件中加载用户名

    const char CLASS_NAME[] = "Chat Window Class";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, "Chat Client", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                               CW_USEDEFAULT, CW_USEDEFAULT, 300, 400, NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, SW_SHOWMAXIMIZED);
    UpdateWindow(hwnd);

    // Initialize socket
    initSocket("127.0.0.1");
    std::thread receiveThread(receiveMessages);
    receiveThread.detach(); // 启动接收消息线程

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (sock != INVALID_SOCKET) {
        closesocket(sock);
    }
    WSACleanup();
    return msg.wParam;
}*/
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include <iostream>
#include <mutex>
#include <fstream>
#include <atomic>

#pragma comment(lib, "Ws2_32.lib")

// 全局变量
HWND hOutputBox, hInputBox, hSendButton;
SOCKET sock;
std::atomic<bool> keepRunning(true);
std::mutex outputMutex;

std::string username;
const std::string usernameFile = ".username";
std::string serverAddress = "qmbbs.net";

// 函数声明
void sendMessage(const std::string& message);
void receiveMessages();
void appendToOutput(const std::wstring& message);
void loadUsername();
void initSocket(const std::string& address);
std::string utf8_encode(const std::wstring& wstr);
std::wstring utf8_decode(const std::string& str);

// 函数定义
void loadUsername() {
    std::ifstream file(usernameFile);
    if (file.is_open()) {
        std::getline(file, username);
        file.close();
    }
    if (username.empty()) {
        username = "Guest";
    }
}

void appendToOutput(const std::wstring& message) {
    std::lock_guard<std::mutex> lock(outputMutex);
    SendMessageW(hOutputBox, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
    SendMessageW(hOutputBox, EM_REPLACESEL, 0, (LPARAM)message.c_str());
    SendMessage(hOutputBox, EM_SCROLL, (WPARAM)SB_BOTTOM, 0);
}

void sendMessage(const std::string& message) {
    std::string fullMessage = "[" + username + "] " + message;
    if (message.back() != '\r') {
        fullMessage += "\r\n";
    }
    std::string utf8Message = utf8_encode(std::wstring(fullMessage.begin(), fullMessage.end()));
    send(sock, utf8Message.c_str(), utf8Message.length(), 0);
    appendToOutput(std::wstring(utf8Message.begin(), utf8Message.end()));
}

void receiveMessages() {
    char buffer[1024];
    while (keepRunning) {
        int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::string receivedMessage(buffer);
            std::wstring receivedMessageW = utf8_decode(receivedMessage);
            appendToOutput(receivedMessageW + L"\r\n");
        } else if (bytesReceived == 0) {
            appendToOutput(L"Server disconnected.\r\n");
            keepRunning = false;
        } else {
            keepRunning = false;
        }
    }
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
    WSACleanup();
}

void initSocket(const std::string& address) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        appendToOutput(L"Failed to create socket.\r\n");
        keepRunning = false;
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1145);
    serverAddr.sin_addr.s_addr = inet_addr(address.c_str());

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        appendToOutput(L"Connection failed. Retrying...\r\n");
        closesocket(sock);
        sock = INVALID_SOCKET;
        keepRunning = false;
        return;
    }

    appendToOutput(L"Connected to server.\r\n");
}

std::string utf8_encode(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], size_needed, NULL, NULL);
    return str;
}

std::wstring utf8_decode(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], size_needed);
    return wstr;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            hInputBox = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
                                       10, 10, 260, 60, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hOutputBox = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
                                        10, 80, 260, 200, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hSendButton = CreateWindow(L"BUTTON", L"SEND", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                       10, 290, 100, 30, hwnd, (HMENU)1, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            break;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == 1) { // SEND button
                char buffer[256];
                GetWindowTextA(hInputBox, buffer, sizeof(buffer));
                std::string message(buffer);
                if (!message.empty()) {
                    sendMessage(message);
                    SetWindowTextA(hInputBox, "");
                }
            }
            break;
        case WM_DESTROY:
            keepRunning = false;
            PostQuitMessage(0);
            break;
        case WM_SIZE: {
            RECT rc;
            GetClientRect(hwnd, &rc);
            int width = rc.right - rc.left;
            int height = rc.bottom - rc.top;

            MoveWindow(hInputBox, 10, 10, width - 20, 60, TRUE);
            MoveWindow(hOutputBox, 10, 80, width - 20, height - 120, TRUE);
            MoveWindow(hSendButton, 10, height - 40, 100, 30, TRUE);
        }
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nShowCmd) {
    loadUsername();

    if (lpCmdLine && *lpCmdLine) {
        serverAddress = lpCmdLine;
    }

    const wchar_t CLASS_NAME[] = L"Chat Window Class";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
            0, CLASS_NAME, L"Chat Client", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT, 300, 400, NULL, NULL, hInstance, NULL
    );
    ShowWindow(hwnd, SW_SHOWMAXIMIZED);
    UpdateWindow(hwnd);

    initSocket(serverAddress);
    std::thread receiveThread(receiveMessages);
    receiveThread.detach();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (sock != INVALID_SOCKET) {
        closesocket(sock);
    }
    WSACleanup();
    return msg.wParam;
}
