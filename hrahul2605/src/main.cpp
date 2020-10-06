#include <iostream>
#include <utility>
#include <fstream>
#include <string>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <experimental/filesystem>

using namespace std;
namespace fs = std::experimental::filesystem;

// imperium commands
#define INIT "init"
#define ADD "add"

// present working directory
const char *root = getenv("dir");

vector<string> ignorePaths; // files to be ignored from .imperiumIgnore
vector<string> addLogs;     // files to be ignored from add.log

// concat function -
// concats two const char* using malloc

char *concat(const char *a, const char *b)
{
    char *c = (char *)malloc((strlen(a) + strlen(b)) * sizeof(char));
    strcpy(c, a);
    strcat(c, b);

    return c;
}

// checkPath function -
// checks if the path provided is valid or not

bool checkPath(const char *path)
{
    struct stat buffer;
    return stat(path, &buffer) == 0;
}

// ignoreCheck function -
// checks if the file / folder is to be ignored based on .imperiumIgnore
// checks if the file / folder is to be ignored based on if its already ADDED to add.log

bool ignoreCheck(const char *path, vector<string> allIgnorePaths)
{
    for (int i = 0; i < allIgnorePaths.size(); i++)
    {
        string curPath = allIgnorePaths[i];
        int minLength = min(curPath.length(), string(path).length()), count = 0;

        for (int j = 0; j < minLength; j++)
        {
            if (curPath[j] != path[j])
            {
                count = -1;
                break;
            }
            count += curPath[j] == path[j];
        }
        if (count == minLength && count != string(root).length() + 1)
            return 1;
        if (!strcmp(curPath.c_str(), (string(root) + "/-d").c_str()))
            return 1;
    }
    return 0;
}

void addToCache(string path, char type)
{
    struct stat buffer;
    if (stat((string(root) + "/.imperium/.add").c_str(), &buffer) != 0)
    {
        mkdir((string(root) + "/.imperium/.add").c_str(), 0777);
    }
    if (type == 'f')
    {
        string filename = path.substr(string(root).length());
        string filerel = string(root) + "/.imperium/.add" +
                         filename.substr(0, filename.find_last_of("/"));

        struct stat buffer2;
        if (stat(filerel.c_str(), &buffer2) != 0)
        {
            fs::create_directories(filerel);
        }
        fs::copy_file(path, string(root) + "/.imperium/.add" + filename,
                      fs::copy_options::overwrite_existing);
    }
    else if (type == 'd')
    {
        string filename = path.substr(string(root).length());
        string filerel = string(root) + "/.imperium/.add" + filename;

        struct stat buffer3;
        if (stat(filerel.c_str(), &buffer3) != 0)
        {
            fs::create_directories(filerel);
        }
    }
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
        file << ".gitignore\n.imperium/\n.git\n.imperiumIgnore\nnode_modules\n.env\n";
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

// imperium add FILE/FOLDER
void add(const char *path)
{
    // checking if repo is initialised or not
    if (!checkPath((root + string("/.imperium")).c_str()))
    {
        cout << "Repository not initialised.\n";
        return;
    }

    string addPath = !strcmp(path, ".") ? root + string("/") : root + string("/") + path;

    // checking if path provided is valid
    if (!checkPath(addPath.c_str()))
    {
        cout << "File/Folder not found.\n";
        return;
    }

    // reset ignorePaths vector, if .imperiumIgnore gets updated
    ignorePaths.clear();
    ifstream imperiumIgnore;
    imperiumIgnore.open(root + string("/.imperiumIgnore"));
    while (imperiumIgnore)
    {
        string path;
        getline(imperiumIgnore, path);
        ignorePaths.push_back(root + string("/") + path);
    }

    imperiumIgnore.close();

    // reset addLogs vector, if add.log gets updated
    addLogs.clear();
    fstream addLogFile;
    addLogFile.open(root + string("/.imperium/add.log"), ios::in);
    while (addLogFile)
    {
        string path;
        getline(addLogFile, path);
        if (path.length() != 0)
            addLogs.push_back(path);
    }
    addLogFile.close();

    addLogFile.open(root + string("/.imperium/add.log"), ios::app);

    struct stat buffer;
    stat(addPath.c_str(), &buffer);

    // checking if path is directory or file
    if (buffer.st_mode & S_IFDIR)
    {
        if (!ignoreCheck(addPath.c_str(), ignorePaths))
        {
            if (!ignoreCheck(addPath.c_str(), addLogs))
            {
                addLogFile << addPath
                           << "-d\n";
                addToCache(addPath, 'd');
                cout << "ADDED directory: "
                     << addPath
                     << "\n";
            }
        }
        for (auto &iPath : fs::recursive_directory_iterator(addPath))
        {
            if (!ignoreCheck(iPath.path().c_str(), ignorePaths))
            {
                struct stat buffer3;
                if (stat(iPath.path().c_str(), &buffer3) == 0)
                {

                    if (buffer3.st_mode & S_IFDIR)
                    {
                        addToCache(iPath.path(), 'd');
                        if (!ignoreCheck(iPath.path().c_str(), addLogs))
                        {
                            addLogFile << iPath.path().c_str() << "-d\n";
                            cout << "ADDED directory: " << iPath.path().c_str() << "\n";
                        }
                        else
                            cout << "UPDATED directory: " << iPath.path().c_str() << "\n";
                    }
                    else if (buffer3.st_mode & S_IFREG)
                    {
                        addToCache(iPath.path(), 'f');
                        if (!ignoreCheck(iPath.path().c_str(), addLogs))
                        {
                            addLogFile << iPath.path().c_str() << "-f\n";
                            cout << "ADDED file: " << iPath.path().c_str() << "\n";
                        }
                        else
                            cout << "UPDATED file: " << iPath.path().c_str() << "\n";
                    }
                    else
                    {
                        continue;
                    }
                }
                // }
            }
        }
    }
    else
    {
        if (!ignoreCheck(addPath.c_str(), ignorePaths))
        {
            addToCache(addPath, 'f');
            if (!ignoreCheck(addPath.c_str(), addLogs))
            {
                addLogFile << addPath
                           << "-f\n";
                cout << "ADDED file: "
                     << addPath << "\n";
            }
            else
                cout << "UPDATED file: " << addPath << "\n";
        }
    }
    addLogFile.close();
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
    if (argc == 1)
        cout << "Welcome to Imperium by hrahul2605.\n";
    else if (argc >= 2)
    {
        if (!strcmp(argv[1], INIT))
            init(root);
        else if (!strcmp(argv[1], ADD))
        {
            if (argc < 3)
                cout << "Less arguments provided. Provide File/Folder name.\n";
            else
                add(argv[2]);
        }
        else if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h") || !strcmp(argv[1], "-H"))
            help();
    }
    return 0;
}
