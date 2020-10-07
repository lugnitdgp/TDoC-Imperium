#include <cstdlib>
#include <fstream>
#include <vector>
#include <iostream>
#include <cstring>
#include <utility>
#include <sys/types.h>
#include <sys/stat.h>
#include <experimental/filesystem>
#include <chrono>
#include <ctime>
#include <sstream>
#include <openssl/sha.h>

using namespace std;
namespace fs = std::experimental::filesystem;
using namespace std::experimental::filesystem;

string root = "";
void init(string root)
{

    struct stat buffer;
    // cout << root << "\n";

    if (!stat(string(root + "/.imperium").c_str(), &buffer))
    {
        //if already exists
        cout << "The repository already exists.\n";
        return;
    }

    string ignorePath = root + "/.imperiumIgnore";
    ofstream ignore(ignorePath.c_str());
    ignore << "/.imperium/\n/node_modules/\n/.env/\n/.imperiumIgnore"
           << "\n";
    ignore.close();

    root += "/.imperium";
    int created = mkdir(root.c_str(), 0777);

    if (!created)
    {
        //if created
        string commitPath = root + "/commitlog";
        // cout<<commitPath<<"\n";
        ofstream commit(commitPath.c_str());
        commit.close();

        string addPath = root + "/addlog";
        // cout<<addPath<<"\n";
        ofstream add(addPath.c_str());
        add.close();

        string conflictPath = root + "/conflict";
        // cout<<conflictPath<<"\n";
        ofstream conflict(conflictPath.c_str());
        conflict << "False\n";
        conflict.close();

        cout << "Initialized empty Imperium Repository.\n";
    }
    else
    {
        cout << "ERROR,Directory not created\n";
    }
}

//add function

void addtoCache(const path &currentPath)
{
    // cout<<"Added to Cache:"<<currentPath<<"\n";

    int len = root.length();

    string relativePathString = currentPath.string().substr(len);
    // cout<<relativePathString<<"\n";
    path relativePath(relativePathString);

    path rootPath(root);
    path rootPathInUserDirectory(root);

    rootPath /= ".imperium";
    rootPath /= ".add";

    for (const auto &cur : relativePath)
    {
        if (cur == "/")
            continue;
        rootPath /= cur;
        rootPathInUserDirectory /= cur;

        // cout<<rootPath<<"\n";
        if (is_directory(rootPathInUserDirectory))
        {
            // cout<<rootPath<<" is a directory.\n";
            if (!exists(rootPath))
            {
                create_directory(rootPath.string().c_str());
            }
        }
        else
        {
            // cout<<rootPath<<" is not a directory.\n";
            //from -> to
            // cout<<rootPathInUserDirectory<<"\n";
            const auto copyOptions = copy_options::update_existing;
            if (!exists(rootPath))
            {
                cout << currentPath << " Initialized.\n";
            }
            else
            {
                cout << currentPath << " Updated.\n";
            }
            copy_file(currentPath.string(), rootPath.string(), copyOptions);
        }
    }

    // cout<<"\n";
}

bool searchString(ifstream &inputFile, const string &str)
{

    // cout<<"This file is now being searched from:"<<str;
    string line;
    while (getline(inputFile, line))
    {
        if (line.find(str) != string::npos)
            return true;
    }
    return false;
}

void toBeAdded(const path &currentPath)
{

    string addLogPath = root + "/.imperium/addlog";
    //check if file is alredy added to addLog
    ifstream addLogFile(addLogPath.c_str());

    if (!searchString(addLogFile, currentPath.u8string()))
    {
        // cout<<"The current path dosen't exist in:"<<currentPath<<"\n";
        ofstream fout(addLogPath.c_str(), ios::app);
        fout << currentPath << "\n";
        fout.close();
    }
    else
    {
        // cout<<"The "<<currentPath<<" path exists.\n";
    }
    addLogFile.close();
}

bool toBeIgnored(const path &currentPath)
{

    string ignorePath = root + "/.imperiumIgnore";

    string currentPathString = currentPath.string();
    string line;
    ifstream inputFile(ignorePath);

    while (getline(inputFile, line))
    {
        string Directory = root + line;
        // cout<<"directory"<<Directory<<"\n";

        if (line.back() == '/')
        {

            Directory.pop_back();
            //currrent line is a directory
            //so the current path must lie in the path to be checked
            // cout<<"Directory: "<<Directory<<"\n";
            if (currentPathString.find(Directory) != string::npos)
            {
                // cout<<"This directory is ignored\n";
                return true;
            }
        }
        else
        {
            //current line is some file
            //so entire path must be equal to it
            if (currentPathString == Directory)
                return true;
        }
    }
    inputFile.close();
    return false;
}

void addUtil(const path &currentPath)
{

    string imperiumFolder = root + "/.imperium";
    string addFolder = imperiumFolder + "/.add";

    if (!toBeIgnored(currentPath))
    {

        // cout<<currentPath<<" Should not be ignored\n";
        // check in add log
        toBeAdded(currentPath);

        // add to cache
        mkdir(addFolder.c_str(), 0777);
        addtoCache(currentPath);
    }
    else
    {
        // cout<<currentPath<<" should be ignored"<<"\n";
    }
}

void add(vector<string> &argv)
{

    string imperiumFolder = root + "/.imperium";
    string addFolder = imperiumFolder + "/.add";
    string addLogPath = imperiumFolder + "/addlog";
    string ignorePath = imperiumFolder + "/ignorelog";

    if (!exists(addLogPath))
    {
        ofstream add(addLogPath.c_str());
        add.close();
        return;
    }

    for (auto &str : argv)
    {

        if (str == ".")
        {
            str = "";
        }
        path currentPath(root);
        currentPath /= str;
        // cout<<currentPath<<"\n";

        if (!exists(currentPath))
        {
            cout << "Sorry the file: " << currentPath << " dosen't exist.\n";
            continue;
        }

        //if given path is a file
        if (is_regular_file(currentPath))
        {
            // cout<<"This is a normal file.\n";
            addUtil(currentPath);
            continue;
        }
        //if given path is a directory
        for (auto &p : recursive_directory_iterator(currentPath))
        {
            // cout<<"This is a directory.\n";
            // cout<<p.path()<<"\n";
            addUtil(p.path());
        }
    }
}

//commit functionality

string getCurrentTime()
{
    time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    string s(30, '\0');
    strftime(&s[0], s.size(), "%Y-%m-%d-%H:%M:%S", std::localtime(&now));
    return s;
}

void updateCommitLog(string hash, string message)
{
    string imperiumPath = root + "/.imperium";
    string commitLogPath = imperiumPath + "/commitlog";
    string tempFilePath = imperiumPath + "/tempfile";
    ifstream in(commitLogPath);
    ofstream temp(tempFilePath);
    temp << hash << " -- commit " << message << "\n";
    temp << in.rdbuf();
    temp.close();
    in.close();
    fs::remove(commitLogPath);
    std::rename(tempFilePath.c_str(), commitLogPath.c_str());
}

void getPreviousCommit(string prevHash, string currentHash)
{
    //
    string imperiumPath = root + "/.imperium";
    string addPath = imperiumPath + "/.add";
    string prevCommitPath = imperiumPath + "/.commit/" + prevHash;
    string currentCommitPath = imperiumPath + "/.commit/" + currentHash;

    for (auto &p : recursive_directory_iterator(prevCommitPath))
    {
        string relative = p.path().string().substr(prevCommitPath.size());
        // cout<<relative<<"\n";
        string currentPath = root + relative;
        string hashFolderPath = currentCommitPath + relative;

        //check if currentpath exists now in the project
        if (exists(currentPath))
        {
            //if directory then add if not existing already
            //if file then also add if not existing already
            if (!exists(hashFolderPath))
            {
                if (is_directory(currentPath))
                    create_directory(hashFolderPath);
                else
                {
                    fs::copy_file(currentPath, hashFolderPath);
                }
            }
        }
    }
}
void commit(vector<string> &args)
{

    string imperiumPath = root + "/.imperium";
    string addPath = imperiumPath + "/.add";
    string commitLogPath = imperiumPath + "/commitlog";
    string addLogPath = imperiumPath + "/addlog";
    string message;
    //firrst check if message passed
    // cout<<args.size();
    if (args.size() < 2 || args[0] != "-m")
    {
        cout << "Plese add a message with the commit.\n";
        return;
    }
    message = args[1];
    //next check if imperium exists
    if (!exists(imperiumPath))
    {
        cout << "Please initialize the imperium repository.\n";
        return;
    }

    //get time
    string currentTime = getCurrentTime();

    //hash
    unsigned char obuf[20];
    unsigned char time[21];
    char fintime[41];
    strcpy((char *)time, currentTime.c_str());
    SHA1(time, currentTime.size(), obuf);
    // cout<<currentTime<<"\n";

    for (int i = 0; i < 20; i++)
    {
        // printf("%02x",obuf[i]);
        sprintf(&fintime[i * 2], "%02x", obuf[i]);
    }
    printf("\nSHA1 HASH: %s\n", fintime);
    string commitHash(fintime);
    // cout<<"\n";
    // cout<<commitHash<<"\n";

    //now create a folder with .commit
    string commitPath = imperiumPath + "/.commit";
    if (!exists(commitPath))
    {
        create_directory(commitPath);
    }

    //create a path with hash
    string hashPath = commitPath + "/" + commitHash;
    // create_directory(hashPath);

    //move add folder to commit hash
    if (!exists(addPath))
    {
        cout << "No changes to commit.\n";
        return;
    }
    cout << addPath << "\n"
         << hashPath << "\n";

    const auto copyOptions = copy_options::recursive;
    create_directory(hashPath);

    //copy directory to hashpath
    fs::copy(addPath, hashPath, copyOptions);

    //remove add and addlog
    remove_all(addPath);
    fs::remove(addLogPath);

    //update commit log
    //check if first commit or not by checking commit.log
    string line;
    ifstream commitFile(commitLogPath);
    getline(commitFile, line, ' ');
    commitFile.close();
    cout << "line is:" << line << "\n";
    if (!line.empty())
    {
        //get previous addlog path
        string prevHash = line;
        getPreviousCommit(prevHash, commitHash);
    }
    //update commitlog
    updateCommitLog(commitHash, message);
}

int main(int argc, char *argv[])
{
    root = getenv("dir");
    vector<string> Args;
    for (int i = 2; i < argc; i++)
    {
        Args.push_back(argv[i]);
    }

    if (argc == 1)
    {
        cout << "Please specify a flag\n";
    }
    else if (!strcmp(argv[1], "init"))
    {
        init(root);
    }
    else if (!strcmp(argv[1], "add"))
    {
        // cout<<"Add function invoked."<<"\n";

        add(Args);
    }
    else if (!strcmp(argv[1], "commit"))
    {

        commit(Args);
    }
    return 0;
}