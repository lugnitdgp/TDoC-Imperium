#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <utility>
#include <algorithm>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;
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

string getCurrentPath(string path)
{
    int len = root.length();
    if (path.substr(0, len) == root)
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

    return 0;

}