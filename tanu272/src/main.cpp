#include <cstdio>
#include <iostream>
#include <sys/stat.h>
using namespace std;

int init(int argc, char* argv[])
{
    const char* folder;
    //folder = "C:\\Users\\...";
    folder = "$PWD/.imperium";
    struct stat sb={0};

    if (stat(folder, &sb) == 0 && S_ISDIR(sb.st_mode)) {
        printf("YES\n");
        return 0;
    } 
    else {
        mkdir( "$PWD/.imperium", 0700);
        fstream file;
        file.open("$PWD/.imperiumIgnore");
        file>>/node_modules;
        //file>>/.env;
        file.close();
        file.open("$PWD/.imperium/conflict");
        file>>"false";
        file.close();

        return 0;
    }
}

int main(int argc, char* argv[])
{
    if(argv[][1]== 'init')
        int init(int argc, char* argv[]);
    return 0;
}