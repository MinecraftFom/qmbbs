#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>


#include "libs/libgetfile.h"

void extract_letters(const char* input, char* output) {
    // Vector to store separated letters
    std::vector<char> letters;

    // Iterate over each character in the input
    for (int i = 0; input[i] != '\0'; ++i) {
        if (std::isalpha(static_cast<unsigned char>(input[i]))) {
            letters.push_back(input[i]);
        }
    }

    // Copy letters to the output char array
    int length = letters.size();
    for (int i = 0; i < length; ++i) {
        output[i] = letters[i];
    }
    output[length] = '\0'; // Null-terminate the output array
}

void say_word(const char* _animetgt) {
    // Calculate length of C-string
    char tmp[100];
    extract_letters(_animetgt,tmp);
    int length = 0;
    while (_animetgt[length] != '\0') {
        ++length;
    }

    // Print each character with a delay
    for (int i = 0; i < length; ++i) {
        std::cout << _animetgt[i];
        std::cout.flush(); // Ensure to flush the output buffer to display the text immediately
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // Ensure to flush the output buffer to display the text immediately
    std::cout.flush();
}
// Function to get command type
int get_ret_type(const string &in) {
    if (in == "quit") {
        return 0;
    } else if (in == "setpassword") {
        return 1;
    } else if (in == "setusername") {
        return 2;
    } else if (in == "readfile"){
        return 3;
    } else if (in == "sudo"){
        return 4;
    } else if (in == "start_server"){
        return 5;
    } else if (in == "join_chat"){
        return 6;
    } else if (in == "stop_server"){
        return 7;
    } else if (in == "stop_chat"){
        return 8;
    } else if (in == "help") {
        return 9;
    } else if (in == "join_chat_gui") {
        return 10;
    }
    else {
        return -1145;
    }
}

int main() {
    bool isAdmin;
    system("color 0A");
    system("echo off");
    system("title QMBBS - Powered by MinecraftFOM.");
    // Read inputs from files
    string fpg, fog;
    ifstream fn(".username");
    string fng;
    fn >> fng;
    string tempstd = "Admin";
    if (fng == tempstd) {
        while (true) {
            say_word("Enter Admin Password > ");
            string temp2;
            cin >> temp2;
            if (temp2 == "FomoSysAdminPwd") {
                break;
            } else if (temp2 == "quit") {
                exit(-1145);
            } else {
                say_word("Incorrect Password. Try Again. Or You Can Type in 'quit' to Exit");
                cout << endl;
            }
        }
    }
    fn.close();
    ifstream fp(".password");
    fp >> fpg;
    fp.close();
    ifstream fo(".open_statues");
    getline(fo, fog);
    fo.close();

    say_word("QMBBS Forum Powered by Fom477, Supported by DELL, CZY(Terry the Biang, IT club tm leader), Clare Deng(Art club tm employee), Sophia(Art club tm leader)");
    say_word(", Kimi(Moonshot (c)) -> Technique Support, Chatgpt(OpenAI (C)) -> Technique Support");
    cout << endl;
    if (fog == "true") {
        say_word("Hello, User ");
        say_word(fng.c_str());
        say_word(" !");
        cout << endl;
        while (true) {
            string gpsfu;
            say_word("Enter Password > ");
            cin >> gpsfu;
            if (gpsfu == fpg) {
                break;
            } else {
                say_word("Incorrect password. Try again.");
                cout << endl;
            }
        }
    } else {
        ofstream fop(".password");
        ofstream fop2(".open_statues");
        fop << "12345678";
        fop2 << "true";
        fop.close();
        say_word("Hello, User Default Admin! Use '/setusername' to Set User Name. ");
        cout << endl;
        say_word("Your Initial Password is 12345678. Use '/setpassword' to Set Password ");
        cout << endl;
    }
    say_word("QMBBS Powered by Fom477.");
    cout << endl;
    say_word("Welcome to Subscribe at Bilibili(account:");
    string account = "爱玩游戏的Fom";
    say_word(account.c_str());
    say_word(").Or Follow at Github(account: github.com/MinecraftFOM). ");
    cout << endl;
    say_word("Issues Can be Reported to github.com/MinecraftFOM/qmbbs/issues");
    cout << endl;
    say_word("Copyright Fom477 (c) 2024");
    cout << endl;
    say_word("Type '/help' for More Information");
    cout << endl;
    // Read command input
    while (true) {
        string read_in;
        cout << "$ ";
        getline(cin, read_in);

        // Ensure the command starts with '/'
        if (!read_in.empty() && read_in[0] == '/') {
            string command = read_in.substr(1); // Extract command without '/'
            size_t space_pos = command.find(' '); // Find the first space
            string cmd_type = (space_pos == string::npos) ? command : command.substr(0,
                                                                                     space_pos); // Extract command type
            basic_string<char> cmd_param = ((space_pos == string::npos) ? "" : command.substr(
                    space_pos + 1)); // Extract parameter

            int command_type = get_ret_type(cmd_type);

            switch (command_type) {
                case 0:
                    say_word("Quitting...");
                    cout << endl;
                    say_word("BYE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
                    system("taskkill /f /im Socket.Server.exe");
                    system("taskkill /f /im Socket.Client.exe");
                    system("taskkill /f /im MessageViewer.exe");
                    system("taskkill /f /im GUI.exe");
                    system("taskkill /f /im MessageCacher.exe");
                    exit(-1145);
                case 1: {
                    if (isAdmin) {
                        ofstream spd(".password");
                        say_word("Input Your Last Password: ");
                        string temp1;
                        cin >> temp1;
                        if (temp1 == fpg) {
                            say_word("Confirm your password: ");

                            string temp;
                            cin >> temp;
                            if (temp == cmd_param) {
                                spd << cmd_param; // Set new password
                                say_word("Have set password to ");
                                say_word(cmd_param.c_str());
                            } else {
                                say_word("Error");
                            }
                            spd.close();
                            cout << endl;
                        } else {
                            say_word("Error Password");
                            cout << endl;
                        }
                    } else {
                        say_word("You Need to be Administrator to Access This Command");
                        cout << endl;
                    }
                    break;
                }
                case 2: {
                    ofstream sun(".username");
                    sun << cmd_param; // Set new username
                    sun.close();
                    say_word("Have set username to ");
                    say_word(cmd_param.c_str());
                    cout << endl;
                    break;
                }
                case 3: {
                    Cmdlets::libgetfile(&read_in[2]);
                    break;
                }
                case 4: {
                    if (cmd_param == "FomoSysAdminPwd"){
                        isAdmin = true;
                    } else {
                        say_word("Incorrect Password !");
                        cout << endl;
                    }
                    break;
                }
                case 5: {
                    if (isAdmin) {
                        system("start \"SERVER\" /MIN Socket.Server.exe ");
                        system("start \"CACHER\" /MIN MessageCacher.exe ");
                    } else {
                        say_word("You Need to be Administrator to Access This Command");
                        cout << endl;
                    }
                    break;
                }
                case 6: {
                    system(("start \"CHAT\" /MIN Socket.Client " + cmd_param).c_str());
                    system(("start \"MESSAGE VIEWER\" /MIN MessageViewer.exe " + cmd_param).c_str());
                    break;
                }
                case 7: {
                    system("taskkill /f /im Socket.Server.exe");
                    system("taskkill /f /im MessageCacher.exe");
                    break;
                }
                case 8: {
                    system("taskkill /f /im Socket.Client.exe");
                    system("taskkill /f /im MessageViewer.exe");
                    break;
                }
                case 9: {
                    say_word("/ ========");
                    cout << endl;
                    say_word("| help: Show This Page");
                    cout << endl;
                    say_word("| join_chat: join_chat <server_ip> CMD Message Sender ( Without GUI )");
                    cout << endl;
                    say_word("| join_chat_gui: join_chat_gui <server_ip> Open GUI Chat");
                    cout << endl;
                    say_word("| readfile: A useless command. Very Useless");
                    cout << endl;
                    say_word("| setpassword: setpassword <password> Set Password of Your Programme");
                    cout << endl;
                    say_word("| setusername: setusername <username> Set Username of Your Programme");
                    cout << endl;
                    say_word("| stop_chat: Stop Your Client Thread");
                    cout << endl;
                    say_word("| stop_server: Stop The Server, Only Enabled by Administrator, Only On The Server");
                    cout << endl;
                    say_word("| sudo: I Suppose You Don't Know the Password, Used to Get Administrator. sudo <pwd>");
                    cout << endl;
                    break;
                }
                case 10: {
                    system(("start \"LOGGER\" GUI.exe " + cmd_param).c_str());
                    break;
                }
                default:
                    say_word(read_in.c_str());
                    cout << endl;
            }
        } else {
            say_word(read_in.c_str());
            cout << endl;
        }
    }
    system("pause");
    return 0;
}