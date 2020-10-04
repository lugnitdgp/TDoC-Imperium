#include<iostream>
#include<fstream>
#include<utility>
#include<string>
#include<string.h>
#include <sys/stat.h>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

std::string root = "";

void init(std::string path){

    struct stat buf;
    if(stat((path+"/.imperium").c_str(),&buf)==0){
        std::cout<<"Repository has already been initialized\n";
    }
    else{
        std::string ignorepath = path + "/.imperiumIgnore";
        std::ofstream ignore(ignorepath.c_str());

        ignore<<".git\n.gitignore\n.imperium\n.imperiumIgnore\n.node_modules\n";
        ignore.close();

        path += "/.imperium";
        int created=mkdir(path.c_str(),0777);

        if(created == 0){
            std::string commitlog = path + "/commitlog";
            std::ofstream commit(commitlog.c_str());
            commit.close();

            std::string addlog = path + "/addlog";
            std::ofstream add(addlog.c_str());
            add.close();
            
            std::string conflictlog = path + "/conflictlog";
            std::ofstream conflict(conflictlog.c_str());
            conflict.close();

            std::cout<<"Successfully Initialised an empty repository\n";
        }
        else{

            std::cout << "ERROR in initialising an empty repository.\n";
        }
    }
}

int main(int argc, char** argv){

    const std::string dir = getenv("dir");
    root = dir;

    if(strcmp(argv[1],"init") == 0){
        init(root);
    }    
    return 0;
}