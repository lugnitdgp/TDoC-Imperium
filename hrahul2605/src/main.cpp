#include <bits/stdc++.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <filesystem>

using namespace std;

#define INIT "init"

const char *root = getenv("dir");

char *concat(const char *a, const char *b)
{
    char *c = (char *)malloc((strlen(a) + strlen(b)) * sizeof(char));
    strcpy(c, a);
    strcat(c, b);

    return c;
}

bool checkPath(const char *path)
{
    struct stat buffer;
    return stat(path, &buffer) == 0;
}

// imperium init
void init(const char *dir)
{
    const char *initFolder = "/.imperium";
    const char *initDir = concat(dir, initFolder);

    if (checkPath(initDir))
    {
        cout << "Repository already initialised.\n";
        return;
    }

    if (mkdir(initDir, 0777) == -1)
        cout << "Unable to initialise repository.\n";
    else
    {
        ofstream file;

        file.open(dir + string("/.imperiumIgnore"));
        file << ".gitignore\n.imperium\n.git\n.imperiumIgnore\n/node_modules\n.env\n";
        file.close();
        file.open(initDir + string("/commit.log"));
        file.close();
        file.open(initDir + string("/add.log"));
        file.close();
        file.open(initDir + string("/conflict"));
        file.close();

        cout << "Initialized empty Imperium repository.\n";
    }
}

// imperium --help
void help()
{
    cout << "\t\t\tImperium by hrahul2605\n\n";
    cout << "\t\t******************************************\n\n\n";
    cout << "\tinit\t\t\t - Initialises the Repository\n";
    cout << "\t--help,-H,-h\t\t - Help\n";
}

int main(int argc, char const *argv[])
{
    // current working directory

    if (argc == 1)
        cout << "Welcome to Imperium by hrahul2605.\n";
    else if (argc >= 2)
    {
        if (!strcmp(argv[1], INIT))
            init(root);
        else if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h") || !strcmp(argv[1], "-H"))
            help();
    }
    return 0;
}
