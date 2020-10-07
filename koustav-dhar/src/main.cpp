#include <iostream>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <time.h>
#include <string>
#include <string.h>
#include <utility>
#include <algorithm>
#include <vector>
#include <map>
#include <sys/stat.h>
#include <sys/types.h>
#include <filesystem>
#include <openssl/sha.h>

namespace fs = std::filesystem;
using namespace std;

string root = "";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// IMPERIUM INIT FUNCTION

string checkDirType(string path)
{
    struct stat info;
    if (!stat(path.c_str(), &info)) {
        if (info.st_mode & S_IFDIR)
            return "directory";
        else if (info.st_mode & S_IFREG)
            return "file";
    }
    return "\0";
}

bool initDone()
{
    if (checkDirType(root + "/.imperium") != "directory")
        return false;
    if (checkDirType(root + "/.imperium/add.log") != "file")
        return false;
    if (checkDirType(root + "/.imperium/commit.log") != "file")
        return false;
    if (checkDirType(root + "/.imperiumignore") != "file")
        return false;
    if (checkDirType(root + "/.imperium/conflict") != "file")
        return false;
    return true;
}

void init(string path)
{
    struct stat info;
    if (stat((path + "/.imperium").c_str(), &info) == 0) {
        cout << "Repository has already been initialized.\n";
    }
    else {
        string ignorepath = path + "/.imperiumignore";
        ofstream ignore(ignorepath.c_str());
        ignore << "/.gitignore\n/.imperium\n/.git\n/.imperiumignore\n/.node_modules\n/.env\n";
        ignore.close();

        path += "/.imperium";
        int ok = mkdir(path.c_str(), 0777);

        if (!ok) {
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
        else {
            cout << "ERROR. Failed to initialize a repository.\n";
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// IMPERIUM ADD FUNCTION

void add(string path);

bool addDone()
{
    if (checkDirType(root + "/.imperium/.add") != "directory")
        return false;
    return true;
}

string getCurrentPath(string path, string parent = root)
{
    int len = parent.length();
    if (path.substr(0, len) == parent)
        return path.substr(len + 1);
    return path;
}

void addToLog(string path)
{
    ifstream fin((root + "/.imperium/add.log").c_str());
    string check;
    while (getline(fin, check)) {
        if (check == ("/" + path))
            return;
    }
    fin.close();
    ofstream fout((root + "/.imperium/add.log").c_str(), fstream::app);
    fout << "/" + path << endl;
    fout.close();
}

void addToCache(string path, char dir_type)
{
    struct stat info;
    if (checkDirType(root + "/.imperium/.add") != "directory")
        mkdir((root + "/.imperium/.add").c_str(), 0777);

    if (dir_type == 'f') {
        string filePath = root + "/.imperium/.add/" + getCurrentPath(path);
        const fs::path file_path = filePath;
        fs::create_directories(file_path.parent_path());
        fs::copy_file(path, filePath, fs::copy_options::update_existing);
    }
    else if (dir_type == 'd') {
        string filePath = root + "/.imperium/.add/" + getCurrentPath(path);
        struct stat infofile;
        const fs::path file_path = filePath;
        if (checkDirType(filePath) != "directory")
            fs::create_directories(file_path);
    }
}

bool ignoreFolder(string path, vector<string> dirs)
{
    for (auto currdir : dirs) {
        currdir.pop_back();
        if (currdir.find(path) != string::npos)
            return true;
    }
    return false;
}

bool toBeIgnored(string path, int onlyImperiumIgnore = 1)
{
    ifstream addFile((root + "/.imperium/add.log").c_str());
    ifstream ignoreFile((root + "/.imperiumignore").c_str());
    string check;
    vector<string> filenames;
    vector<pair<string, char>> addedFileNames;
    vector<string> ignoreDir;

    path = getCurrentPath(path);
    while (getline(ignoreFile, check)){
        if (check == "/" + path.substr(0, check.size() - 1))
            return true;
    }
    ignoreFile.close();
    addFile.close();
    return false;
    
    // if (!onlyImperiumIgnore) {
    //     if (addFile.is_open()) {
    //         while (!addFile.eof()) {
    //             getline(addFile, check);
    //             if (check.length() > 4) {
    //                 addedFileNames.push_back({check.substr(check.length() - 4), check[check.length() - 1]});
    //             }
    //         }
    //     }
    // }

    // for (auto updateFile : addedFileNames) {
    //     if (path.compare(updateFile.first) == 0) {
    //         addToCache(path, updateFile.second);
    //         cout << "Updated : " << path << endl;
    //         return true;
    //     }
    // }
    // if (find(filenames.begin(), filenames.end(), path) != filenames.end() || ignoreFolder(path, ignoreDir))
    //     return true;

    // return false;
}

void add(string path)
{
    struct stat info;
    if (stat((root + "/.imperium").c_str(), &info) == 0) {
        struct stat infopath;
        if (stat(path.c_str(), &infopath) == 0) {
            if (infopath.st_mode & S_IFDIR) // Checking if the path's of a directory
            {
                if (toBeIgnored(path))
                    return;

                addToLog(path);
                addToCache(path, 'd');
                cout << "Added Directory : " << "\"" << path << "\"" << endl;

                for (auto &childdir : fs::recursive_directory_iterator(path)) {
                    if (toBeIgnored(childdir.path()))
                        continue;
                    struct stat infochild;
                    if (stat(childdir.path().c_str(), &infochild) == 0) {
                        if (infochild.st_mode & S_IFDIR)    // Checking if the child path's of a directory
                        {
                            addToLog(childdir.path());
                            addToCache(childdir.path(), 'd');
                            cout << "Added Directory : " << childdir.path() << endl;
                        }
                        else if (infochild.st_mode & S_IFREG)   // Checking if the child path's of a file
                        {
                            addToLog(childdir.path());
                            addToCache(childdir.path(), 'f');
                            cout << "Added File : " << childdir.path() << endl;
                        }
                    }
                }
            }
            else if (infopath.st_mode & S_IFREG)    // Checking if the path's of a file
            {
                if (toBeIgnored(path))
                    return;

                addToLog(path);
                addToCache(path, 'f');
                cout << "Added File : " << "\"" << path << "\"" << endl;
            }
            else
            {
                cout << "ERROR. Invalid Path.\n";
            }

        }
    }
    else {
        cout << "Repository has not been initialized yet.\n";
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// IMPERIUM COMMIT FUNCTION

string generateCommitHash()
{
    uint64_t timenow = time(nullptr);
    unsigned char result[20]={};

    SHA_CTX ctx = {};
    SHA1_Init(&ctx);
    SHA1_Update(&ctx, (const char*)&timenow, sizeof(timenow));
    SHA1_Final(result, &ctx);

    stringstream oss;
    for (auto x = 0; x < 20; x++) {
       oss << hex << setfill('0') << setw(2) << (unsigned int)(result[x]);
    }
    string commitHash = oss.str();
    return commitHash;
}

map<string, int> allStaged;

void generateAllStaged()
{
    for (auto &childdir : fs::recursive_directory_iterator(root + "/.imperium/.add/")){
        string relPath = getCurrentPath(childdir.path(), root + "/.imperium/.add");
        allStaged[relPath] = 1;
    }
}

void removeAllStaged()
{
    fs::remove_all(root + "/.imperium/.add");
    allStaged.clear();
    fstream fout;
    fout.open((root + "/.imperium/add.log").c_str(), fstream::out | fstream::trunc);
    fout.close();
}

bool checkIfDeleted(string path)
{
    if (allStaged[path])
        return false;
    return true;
}

void addPrevCommit(string prevCommitHash, string commitHash)
{
    string path = root + "/.imperium/.commit/" + prevCommitHash;
    for (auto &childdir : fs::recursive_directory_iterator(path + "/")){
        string relPath = "/" + getCurrentPath(childdir.path(), path);
        if (!checkIfDeleted(relPath)){
            if (checkDirType(childdir.path()) == "directory"){
                if (checkDirType(root + "/.imperium/.commit/" + commitHash + relPath) != "directory")
                    fs::create_directories(root + "/.imperium/.commit/" + commitHash + relPath);
            }
            else if (checkDirType(childdir.path()) == "file"){
                fs::path file_path = root + "/.imperium/.commit/" + commitHash + relPath;
                fs::create_directories(file_path.parent_path());
                fs::copy_file(childdir.path(), file_path, fs::copy_options::overwrite_existing);
            }
        }
    }
}

void addCommit(string commitHash)
{
    string path = root + "/.imperium/.add";
    for (auto &childdir : fs::recursive_directory_iterator(path + "/")){
        string relPath = "/" + getCurrentPath(childdir.path(), path);
        if (checkDirType(childdir.path()) == "directory"){
            if (checkDirType(root + "/.imperium/.commit/" + commitHash + relPath) != "directory")
                fs::create_directories(root + "/.imperium/.commit/" + commitHash + relPath);
        }
        else if (checkDirType(childdir.path()) == "file"){
            fs::path file_path = root + "/.imperium/.commit/" + commitHash + relPath;
            fs::create_directories(file_path.parent_path());
            fs::copy_file(childdir.path(), file_path, fs::copy_options::overwrite_existing);
        }
    }
}

void updateCommitLog(string commitHash, string message)
{
    fstream fout((root + "/.imperium/commit.log").c_str(), fstream::app);
    fout << "commit " << commitHash << " -- " << message << endl;
    fout.close();
}

void commit(string message)
{
    if (!initDone()){
        cout << "ERROR. Repository not initialized properly.\n";
        cout << "Try \"imperium init\".\n";
        return;
    }
    if (!addDone()){
        cout << "ERROR. Nothing to commit.\n";
        cout << "Try \"imperium add <path>\".\n";
        return;
    }
    string commitHash = generateCommitHash(), prevCommitHash = "", temp_hash = "";
    ifstream fin((root + "/.imperium/commit.log").c_str());
    while (getline(fin, temp_hash)){
        prevCommitHash = temp_hash.substr(7, 40);
    }
    fin.close();

    fs::create_directories(root + "/.imperium/.commit/" + commitHash);
    generateAllStaged();
    if (prevCommitHash != ""){
        addPrevCommit(prevCommitHash, commitHash);
    }
    addCommit(commitHash);
    updateCommitLog(commitHash, message);
    removeAllStaged();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {

    const string dir = getenv("dir");
    root = dir;

    if (strcmp(argv[1], "init") == 0) {
        if (argc == 2) {
            init(root);
        }
        else {
            cout << "ERROR. Command arguments isn't valid.\n";
            cout << "Did you mean the command \"imperium init\".\n";
        }
    }
    else if (strcmp(argv[1], "add") == 0) {
        if (argc == 3) {
            string path = root;
            path += "/";
            if (strcmp(argv[2], ".") != 0)
                path += argv[2];
            add(path);
        }
        else {
            cout << "Please specify a proper path.\n";
        }
    }
    else if (strcmp(argv[1], "commit") == 0){
        if (argc < 3 || argv[2] == ""){
            cout << "Please add a proper commit message.\n";
        }
        else{
            string message = "";
            for (int i = 2; i < argc; ++i){
                message += argv[i];
                if (i != argc - 1)
                    message += " ";
            }
            commit(message);
        }
    }

    return 0;

}