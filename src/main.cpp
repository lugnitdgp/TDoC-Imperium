#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/stat.h>
#include<experimental/filesystem>
using namespace std;

string root = "";

void init(string path)
{
    struct stat sb;
    string path_new = path + "/.imperium";
    if (stat(path_new.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
        printf("Repository already initialised.\n");
    } 
    else {

        string ignorepath = path + "/.imperiumIgnore";
        ofstream ignore(ignorepath.c_str());

        ignore<<".gitignore\n.imperium\n.git\n.imperiumignore\n.node_modules\n";
        ignore.close();
        
        path+="/.imperium";
        //cout<<path;
        int created = mkdir(path.c_str(), 0777);
        
        if(created==0)
        {
            string commitlog= path + "/commitlog";
            ofstream commit(commitlog.c_str());
            commit.close();

            string addlog= path + "/addlog";
            ofstream add(addlog.c_str());
            add.close();
            
            string conflictlog= path + "/conflictlog";
            ofstream conflict(conflictlog.c_str());
            conflict.close();

            cout<<"Initialized a new directory\n";
        }
        else
        {
            cout<<"ERROR\n";
        }
    }
}

int main(int argc, char* argv[])
{
    const string dir = getenv("dir");
    root=dir;
    if(strcmp(argv[1],"init")==0)
        init(root);
    return 0;
}
