#include <windows.h>
#include <wininet.h>
#include <iostream>
#include <fstream>
#include <string>

#pragma comment(lib, "wininet.lib")

const std::string VERSION_URL = "https://github.com/MinecraftFom/qmbbs/releases/download/untagged-0c5212672007bf9587b0/default.version";
const std::string LOCAL_VERSION_FILE = ".version";
const std::string FILE_TO_UPDATE = "your_file_to_update.exe"; // 要更新的文件
const std::string DOWNLOAD_URL;

bool downloadFile(const std::string& url, const std::string& outputFile) {
    HINTERNET hInternet = InternetOpen(reinterpret_cast<LPCSTR>(L"Version Checker"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    HINTERNET hConnect = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);

    if (!hConnect) {
        InternetCloseHandle(hInternet);
        return false;
    }

    std::ofstream file(outputFile, std::ios::binary);
    if (!file) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return false;
    }

    char buffer[4096];
    DWORD bytesRead;
    while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        file.write(buffer, bytesRead);
    }

    file.close();
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    return true;
}

std::string readVersionFile(const std::string& filename) {
    std::ifstream file(filename);
    std::string version;
    if (file.is_open()) {
        std::getline(file, version);
        file.close();
    }
    return version;
}

bool updateLocalFile() {
    return downloadFile(DOWNLOAD_URL, FILE_TO_UPDATE);
}

int main() {
    // 下载最新版本信息
    if (!downloadFile(VERSION_URL, LOCAL_VERSION_FILE)) {
        std::cerr << "Failed to download version file." << std::endl;
        return 1;
    }

    // 读取版本信息
    std::string remoteVersion = readVersionFile(LOCAL_VERSION_FILE);
    std::string localVersion = "1.0.0"; // 这里应为您当前的版本号

    // 比较版本
    if (remoteVersion > localVersion) {
        std::cout << "New version available: " << remoteVersion << std::endl;
        if (updateLocalFile()) {
            std::cout << "Update successful!" << std::endl;
        } else {
            std::cerr << "Failed to download the new file." << std::endl;
        }
    } else {
        std::cout << "You are on the latest version: " << localVersion << std::endl;
    }

    return 0;
}