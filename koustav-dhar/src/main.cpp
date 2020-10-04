#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <utility>
#include <sys/stat.h>
#include <sys/types.h>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;
using namespace std;

string root = "";

void init(string path)
{
    struct stat info;
    if (stat((path + "/.imperium").c_str(), &info) == 0){
        cout << "Repository has already been initialized.\n";
    }
    else{
        string ignorepath = path + "/.imperiumignore";
        ofstream ignore(ignorepath.c_str());
        ignore << ".gitignore\n.imperium\n.git\n.imperiumignore\n.node_modules\n";
        ignore.close();

        path += "/.imperium";
        int ok = mkdir(path.c_str(), 0777);

        if (!ok){
            string commitlog = path + "/commit.log";
            ofstream commit(commitlog.c_str());
            commit.close();

            string addlog = path + "/add.log";
            ofstream add(addlog.c_str());
            add.close();

            string conflictlog = path + "/conflict";
            ofstream conflict(conflictlog.c_str());
            conflict << "false" << endl;
            conflict.close();

            cout << "Successfully Initialized an empty repository.\n";
        }
        else{
            cout << "ERROR. Failed to initialize a repository.\n";
        }
    }
}

int main(int argc, char** argv){

    const string dir = getenv("dir");
    root = dir;

    if (strcmp(argv[1], "init") == 0){
        init(root);
    }

    return 0;

}