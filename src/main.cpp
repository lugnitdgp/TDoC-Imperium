#include <cstdio>
#include <iostream>
#include<sys/stat.h>
using namespace std;

int init(int argc, char* argv[])
{
    const char* folder;
    //folder = "C:\\Users\\...";
    folder = "/tmp";
    struct stat sb;

    if (stat(folder, &sb) == 0 && S_ISDIR(sb.st_mode)) {
        printf("YES\n");
        return 0;
    } 
    else {
        mkdir( "$PWD/imperium", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        mkdir( "$PWD/imperium/conflict", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    }
}

int main()
{
    return 0;
}