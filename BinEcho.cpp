//
// Created by LucasLu on 2024/9/15.
//

#include<iostream>
#include<windows.h>

int main(int ret,char** args){
    system("color 0A");
    if (args!= nullptr){
        std::printf(*args,"\n");
    }else{
        std::printf("Empty Argument for Command 'echo'");
    }
    system("color 07");
    return -1145;
}