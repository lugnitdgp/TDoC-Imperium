#include <iostream>
#include <string.h>
#include <cstring>
#include <fstream>
#include <sys/stat.h>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <utility>
#include <string>
#include <openssl/sha.h>
#include <chrono>
#include <ctime>

namespace fs = std::filesystem;

std::string root = "";

void init(std::string path)
{
    struct stat buffer;

    if (stat((path + "/.imperium").c_str(), &buffer) == 0)
    {
        std::cout << "Already initialized as imperium repository"
                  << "\n";
    }
    else
    {
        std::string imperiumIgnore = path + "/.imperiumIgnore";
        std::ofstream ignore(imperiumIgnore.c_str());
        path += "/.imperium";
        ignore << ".imperium/\n"
               << ".git/\n"
               << "/.imperiumIgnore\n"
               << ".node_modules/\n"
               << "/.env\n";
        int created = mkdir(path.c_str(), 0777);
        if (created == 0)
        {
            std::string commitLog = path + "/commit.log";
            std::ofstream commit(commitLog.c_str());
            std::string addLog = path + "/add.log";
            std::string conflictLog = path + "/conflict";
            std::ofstream conflict(conflictLog.c_str());
            std::ofstream add(addLog.c_str());
            add.close();
            commit.close();
            conflict << "false\n";
            conflict.close();
            std::cout << "Initialised imperium repository"
                      << "\n";
        }
        else
        {
            std::cout << "Error with creation"
                      << "\n";
        }
    }
}

int main(int argc, const char **argv)
{
    const char *dir = getenv("dir");
    root = dir;

    if (strcmp(argv[1], "init") == 0)
    {
        init(dir);
    }
    return 0;
}