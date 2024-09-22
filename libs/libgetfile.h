//
// Created by LucasLu on 2024/9/14.
//

#ifndef CMDLIBS_LIBGETFILE_H
#define CMDLIBS_LIBGETFILE_H

#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>
using namespace std;

namespace Cmdlets {
    int libgetfile(string file_path) {
        if (filesystem::exists(file_path)) {
            fstream ret(file_path);
            string getret;
            ret >> getret;
            cout << getret << endl;
        } else {
            cout << "File Not Find!" << endl;
            return 0;
        }

    }
}

#endif //CMDLIBS_LIBGETFILE_H
