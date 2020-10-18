#include <iostream>
#include <string.h>
#include <cstring>
#include <fstream>
#include <sys/stat.h>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <utility>
#include <string>
#include <openssl/sha.h>
#include <chrono>
#include <ctime>

namespace fs = std::filesystem;

std::string root = "";

int ignoreFolder(std::string path, std::vector<std::string> folderList)
{
    auto i = folderList.begin();
    while (i != folderList.end())
    {
        std::string folderName = *i++;
        folderName.pop_back();
        if (path.find(folderName, 0) != std::string::npos)
            return 1;
    }

    return 0;
}

void addToCache(std::string file_path, char type)
{
    struct stat l;
    if (stat((root + "/.imperium/.add").c_str(), &l) != 0)
        mkdir((root + "/.imperium/.add").c_str(), 0777);

    if (type == 'f')
    {
        struct stat k;
        std::string filename = file_path.substr(root.length());
        std::string filerel = root + "/.imperium/.add" + filename.substr(0, filename.find_last_of('/'));
        if (stat(filerel.c_str(), &k) != 0)
        {
            fs::create_directories(filerel);
        }

        fs::copy_file(file_path, root + "/.imperium/.add" + filename, fs::copy_options::overwrite_existing);
    }
    else if (type == 'd')
    {
        struct stat k;
        std::string filename = file_path.substr(root.length());
        std::string filerel = root + "/.imperium/.add" + filename;
        if (stat(filerel.c_str(), &k) != 0)
        {
            fs::create_directories(filerel);
        }
    }
}

int toBeIgnored(std::string path, int onlyImperiumIgnore = 0)
{
    std::ifstream readIgnoreFile, readAddLog;
    std::string filename;
    std::vector<std::string> filenames;
    std::vector<std::pair<std::string, char>> addedFilenames;
    std::vector<std::string> ignoreDir;

    readIgnoreFile.open(root + "/.imperiumIgnore");
    readAddLog.open(root + "/.imperium/add.log");

    if (readIgnoreFile.is_open())
    {
        while (!readIgnoreFile.eof())
        {
            std::getline(readIgnoreFile, filename);
            auto i = filename.end();
            i--;
            if (*i == '/')
            {
                ignoreDir.push_back(filename);
            }
            else
                filenames.push_back(root + filename);
        }
    }

    readIgnoreFile.close();

    if (onlyImperiumIgnore == 0)
    {

        //Read the add log and put them as a path-type pair in addedFilenames
        if (readAddLog.is_open())
        {
            while (!readAddLog.eof())
            {
                std::getline(readAddLog, filename);
                if (filename.length() > 4)
                {
                    addedFilenames.push_back(make_pair(filename.substr(1, filename.length() - 4), filename.at(filename.length() - 1)));
                }
            }
        }

        readAddLog.close();
        //Loop through the addedFilenames(add log) and check if file to be added already exists
        //if yes, then put it into cache overwriting the previous file
        for (auto i = addedFilenames.begin(); i != addedFilenames.end(); i++)
        {
            std::pair<std::string, char> fileEntry = *i;
            if (path.compare(fileEntry.first) == 0)
            {
                addToCache(path, fileEntry.second);
                std::cout << "Updated : " << path << "\n";
                return 1;
            }
        }
    }

    if (std::find(filenames.begin(), filenames.end(), path) != filenames.end() || ignoreFolder(path, ignoreDir))
        return 1;

    return 0;
}

void init(std::string path)
{
    struct stat buffer;

    if (stat((path + "/.imperium").c_str(), &buffer) == 0)
    {
        std::cout << "Already initialized as imperium repository"
                  << "\n";
    }
    else
    {
        std::string imperiumIgnore = path + "/.imperiumIgnore";
        std::ofstream ignore(imperiumIgnore.c_str());
        path += "/.imperium";
        ignore << ".imperium/\n"
               << ".git/\n"
               << "/.imperiumIgnore\n"
               << ".node_modules/\n"
               << "/.env\n";
        int created = mkdir(path.c_str(), 0777);
        if (created == 0)
        {
            std::string commitLog = path + "/commit.log";
            std::ofstream commit(commitLog.c_str());
            std::string addLog = path + "/add.log";
            std::string conflictLog = path + "/conflict";
            std::ofstream conflict(conflictLog.c_str());
            std::ofstream add(addLog.c_str());
            add.close();
            commit.close();
            conflict << "false\n";
            conflict.close();
            std::cout << "Initialised imperium repository"
                      << "\n";
        }
        else
        {
            std::cout << "Error with creation"
                      << "\n";
        }
    }
}

void add(char **argv)
{
    struct stat buffer;
    if (stat((root + "/.imperium").c_str(), &buffer) == 0)
    {
        std::ofstream addFile;

        addFile.open(root + "/.imperium/add.log", std::ios_base::app);
        std::string path = root;
        if (strcmp(argv[2], ".") != 0)
        {
            path += "/";
            path += argv[2];
        }
        struct stat s;
        if (stat(path.c_str(), &s) == 0)
        {
            if (s.st_mode & S_IFDIR)
            {
                //Check if file is to be ignored or updated
                //if false then toBeIgnored updated cached directory/file and returned 1 so skip adding to cache here
                //if true then implies new untracked file is being added so move on to file creation in cache
                if (!toBeIgnored(path))
                {
                    addFile << "\"" << path << "\""
                            << "-d\n";
                    addToCache(path, 'd');
                    std::cout << "added directory: "
                              << "\"" << path << "\""
                              << "\n";
                }
                for (auto &p : fs::recursive_directory_iterator(path))
                {
                    //same as imperium add .
                    if (toBeIgnored(p.path()))
                        continue;
                    if (stat(p.path().c_str(), &s) == 0)
                    {
                        if (s.st_mode & S_IFREG)
                        {
                            addFile << p.path() << "-f\n";
                            addToCache(p.path(), 'f');
                            std::cout << "added file: " << p.path() << "\n";
                        }
                        else if (s.st_mode & S_IFDIR)
                        {
                            addFile << p.path() << "-d\n";
                            addToCache(p.path(), 'd');
                            std::cout << "added directory: " << p.path() << "\n";
                        }
                        else
                        {
                            continue;
                        }
                    }
                }
            }
            else if (s.st_mode & S_IFREG)
            {
                if (!toBeIgnored(path))
                {
                    addFile << "\"" << path << "\""
                            << "-f\n";
                    addToCache(path, 'f');
                    std::cout << "added file: "
                              << "\"" << path << "\""
                              << "\n";
                    addFile.close();
                }
            }
            else
            {
                std::cout << "path is not a file.\n";
            }
        }
        else
        {
            std::cout << "file doesn't exist, kindly check.\n";
            addFile.close();
        }

        std::cout << "add used\n";
    }
    else
    {
        std::cout << "Not an imperium repository. Type imperium init\n";
    }
}

std::string getTime()
{
    auto end = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    std::string time = std::ctime(&end_time);

    return time;
}

void getCommitLog()
{
    std::string commitLogPath = root + "/.imperium/commit.log";
    std::string commitLine;
    std::ifstream commitLog;
    commitLog.open(commitLogPath);
    while (getline(commitLog, commitLine))
    {
        std::cout << commitLine << "\n";
    }
}

std::string getCommitHash()
{
    std::string time = getTime();
    std::string commitFilename = time;
    char buf[3];
    std::string commitHash = "";

    int length = commitFilename.length();
    unsigned char hash[20];
    unsigned char *val = new unsigned char[length + 1];
    strcpy((char *)val, commitFilename.c_str());

    SHA1(val, length, hash);
    int i;
    for (i = 0; i < 20; i++)
    {
        sprintf(buf, "%02x", hash[i]);
        commitHash += buf;
    }
    return commitHash;
}

void repeatCommit(std::string absPath, char type, std::string commitHash, int len)
{
    mkdir((root + "/.imperium/.commit/" + commitHash).c_str(), 0777);
    struct stat n;

    if (type == 'f')
    {
        struct stat h;
        std::string filename = absPath.substr(root.length() + 19 + len);
        std::string filerel = root + "/.imperium/.commit/" + commitHash + filename.substr(0, filename.find_last_of('/'));

        if (stat((root + filename).c_str(), &h) == 0)
        {
            if (stat(filerel.c_str(), &h) != 0)
            {
                fs::create_directories(filerel);
            }

            // std::string commitFile=root+"/.imperium/.commit/"+commitHash+filename;
            // calculateSHA1(absPath,commitFile);

            fs::copy_file(absPath, root + "/.imperium/.commit/" + commitHash + filename, fs::copy_options::overwrite_existing);
        }
    }
    else if (type == 'd')
    {
        struct stat k;
        std::string filename = absPath.substr(root.length() + 19 + len);
        std::string filerel = root + "/.imperium/.commit/" + commitHash + filename;

        if (stat((root + filename).c_str(), &k) == 0)
        {
            if (stat(filerel.c_str(), &k) != 0)
            {
                fs::create_directories(filerel);
            }
        }
    }
}

void addCommit(std::string absPath, char type, std::string commitHash)
{
    struct stat n;
    if (stat((root + "/.imperium/.commit").c_str(), &n) != 0)
        mkdir((root + "/.imperium/.commit").c_str(), 0777);

    if (stat((root + "/.imperium/.commit/" + commitHash).c_str(), &n) != 0)
        mkdir((root + "/.imperium/.commit/" + commitHash).c_str(), 0777);

    if (type == 'f')
    {
        struct stat h;
        std::string filename = absPath.substr(root.length() + 15);
        std::string filerel = root + "/.imperium/.commit/" + commitHash + filename.substr(0, filename.find_last_of('/'));

        if (stat(filerel.c_str(), &h) != 0)
        {
            fs::create_directories(filerel);
        }

        // uncomment when you want to hash the commit files
        // std::string commitFile=root+"/.imperium/.commit/"+commitHash+filename;
        // calculateSHA1(absPath,commitFile);

        // comment when you want the hashed files
        fs::copy_file(absPath, root + "/.imperium/.commit/" + commitHash + filename, fs::copy_options::overwrite_existing);
    }
    else if (type == 'd')
    {
        struct stat k;
        std::string filename = absPath.substr(root.length() + 15);
        std::string filerel = root + "/.imperium/.commit/" + commitHash + filename;
        if (stat(filerel.c_str(), &k) != 0)
        {
            fs::create_directories(filerel);
        }
    }
}

void updateCommitLog(std::string commitHash, std::string message)
{
    std::ofstream writeCommitLog;
    std::ofstream writeHeadLog;
    std::ifstream readCommitLog;

    std::string line;

    readCommitLog.open(root + "/.imperium/commit.log");
    writeCommitLog.open(root + "/.imperium/commitNew.log", std::ios::app);
    writeHeadLog.open(root + "/.imperium/head.log");

    writeCommitLog << commitHash << " -- " << message << " -->HEAD\n";
    writeHeadLog << commitHash << " -- " << message << " -->HEAD\n";

    while (std::getline(readCommitLog, line))
    {
        if (line.find(" -->HEAD") != std::string::npos)
        {
            writeCommitLog << line.substr(0, line.length() - 8) << "\n";
        }
        else
        {
            writeCommitLog << line << "\n";
        }
    }
    remove((root + "/.imperium/commit.log").c_str());
    rename((root + "/.imperium/commitNew.log").c_str(), (root + "/.imperium/commit.log").c_str());
    readCommitLog.close();
    writeCommitLog.close();
}

void commit(char **argv)
{
    struct stat buf;
    if (stat((root + "/.imperium").c_str(), &buf) != 0)
    {
        std::cout << "Not an imperium repo\n";
    }
    else
    {
        std::cout << "commit used\n";
        std::string message = argv[2];

        if (strcmp(argv[2], "") != 0)
        {
            std::string commitHash = getCommitHash();
            struct stat s;
            if (stat((root + "/.imperium/head.log").c_str(), &buf) == 0)
            {
                std::string headHash;
                std::ifstream readCommitLog;

                readCommitLog.open(root + "/.imperium/head.log");

                std::getline(readCommitLog, headHash);
                headHash = headHash.substr(0, headHash.find(" -- "));

                for (auto &p : fs::recursive_directory_iterator(root + "/.imperium/.commit/" + headHash))
                {
                    if (stat(p.path().c_str(), &s) == 0)
                    {
                        if (s.st_mode & S_IFREG)
                        {
                            repeatCommit(p.path(), 'f', commitHash, headHash.length());
                            // std::cout << "committed file: " << p.path() << "\n";
                        }
                        else if (s.st_mode & S_IFDIR)
                        {
                            repeatCommit(p.path(), 'd', commitHash, headHash.length());
                            // std::cout << "committed directory: " << p.path() << "\n";
                        }
                        else
                        {
                            continue;
                        }
                    }
                }
            }
            for (auto &p : fs::recursive_directory_iterator(root + "/.imperium/.add"))
            {
                if (stat(p.path().c_str(), &s) == 0)
                {
                    if (s.st_mode & S_IFREG)
                    {
                        addCommit(p.path(), 'f', commitHash);
                        // std::cout << "committed file: " << p.path() << "\n";
                    }
                    else if (s.st_mode & S_IFDIR)
                    {
                        addCommit(p.path(), 'd', commitHash);
                        // std::cout << "committed directory: " << p.path() << "\n";
                    }
                    else
                    {
                        continue;
                    }
                }
            }
            fs::remove_all((root + "/.imperium/.add").c_str());
            remove((root + "/.imperium/add.log").c_str());
            updateCommitLog(commitHash, message);
        }
    }
}

int BUFFER_SIZE = 8192;

int compareFiles(std::string path1, std::string path2)
{

    std::ifstream lFile(path1.c_str(), std::ifstream::in | std::ifstream::binary);
    std::ifstream rFile(path2.c_str(), std::ifstream::in | std::ifstream::binary);

    if (!lFile.is_open() || !rFile.is_open())
    {
        return 1;
    }

    char *lBuffer = new char[BUFFER_SIZE]();
    char *rBuffer = new char[BUFFER_SIZE]();

    do
    {
        lFile.read(lBuffer, BUFFER_SIZE);
        rFile.read(rBuffer, BUFFER_SIZE);
        int numberOfRead = lFile.gcount(); //I check the files with the same size
        if (numberOfRead != rFile.gcount())
            return 1;

        if (std::memcmp(lBuffer, rBuffer, numberOfRead) != 0)
        {
            memset(lBuffer, 0, numberOfRead);
            memset(rBuffer, 0, numberOfRead);
            return 1;
        }
    } while (lFile.good() || rFile.good());

    delete[] lBuffer;
    delete[] rBuffer;

    return 0;
}

int commitCheck(std::string checkPath)
{
    std::string commitFolderPath = root + "/.imperium/.commit/" + checkPath;
    for (auto &p : fs::recursive_directory_iterator(commitFolderPath))
    {
        std::string filename = ((std::string)p.path()).substr(root.length() + 19 + checkPath.length());
        struct stat s;
        struct stat b;
        if (stat(p.path().c_str(), &s) == 0 && stat((root + filename).c_str(), &b) == 0)
        {

            if (s.st_mode & S_IFDIR && b.st_mode & S_IFDIR)
                continue;
            else if (s.st_mode & S_IFREG && b.st_mode & S_IFREG)
            {
                if (compareFiles(p.path(), (root + filename)))
                    return 1;
            }
        }
    }

    return 0;
}

void mergeConflict()
{
    std::ofstream cfile;
    cfile.open(root + "/.imperium/conflict", std::ofstream::trunc);
    cfile << "true\n";
    cfile.close();
}

void revert(char **argv)
{
    if (argv[2] == "")
    {
        std::cout << "Please provide a Hash"
                  << "\n";
        return;
    }
    std::string revertCommitHash = argv[2];
    std::string commitFolderName = revertCommitHash.substr(0, 40);
    bool switchBool = false;
    bool dirBool = false;
    std::ifstream commitLog;
    std::string commitLine;

    commitLog.open(root + "/.imperium/commit.log");
    std::string lastCommit = "";
    if (std::getline(commitLog, commitLine))
    {
        lastCommit = commitLine.substr(0, 40);
    }
    commitLog.close();

    commitLog.open(root + "/.imperium/commit.log");
    while (std::getline(commitLog, commitLine))
    {
        if (switchBool == true)
        {
            commitFolderName = commitLine.substr(0, 40);
            switchBool = false;
            break;
        }

        //std::cout << commitLine.substr(0, 40) << "\n";
        if (commitFolderName == commitLine.substr(0, 40))
        {

            switchBool = true;
            dirBool = true;
        }
    }
    commitLog.close();

    if (!dirBool)
    {
        std::cout << "incorrect Hash provided"
                  << "\n";
        return;
    }
    else if (dirBool && lastCommit != "")
    {
        if (commitCheck(lastCommit) == 0)
        {
            if (!switchBool)
            {
                std::string passedHash = revertCommitHash.substr(0, 40);
                std::string commitFolderPath = root + "/.imperium/.commit/" + commitFolderName;
                if (lastCommit == passedHash)
                {
                    fs::copy(commitFolderPath, root + "/", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                    for (auto &p : fs::recursive_directory_iterator(root))
                    {
                        struct stat s;

                        if (stat(p.path().c_str(), &s) == 0)
                        {
                            std::string relPath = ((std::string)p.path()).substr(root.length());
                            std::string prevPath = commitFolderPath + relPath;
                            std::string revertPath = root + "/.imperium/.commit/" + passedHash + relPath;

                            struct stat a;
                            struct stat b;
                            if (s.st_mode & S_IFDIR)
                                continue;
                            else if (s.st_mode & S_IFREG)
                            {
                                if (stat(prevPath.c_str(), &a) != 0 && stat(revertPath.c_str(), &b) == 0)
                                {
                                    remove(p.path());
                                }
                            }
                        }
                    }
                }
                else
                {
                    for (auto &p : fs::recursive_directory_iterator(commitFolderPath))
                    {
                        struct stat s;

                        if (stat(p.path().c_str(), &s) == 0)
                        {
                            if (s.st_mode & S_IFDIR)
                                continue;
                            else if (s.st_mode & S_IFREG)
                            {
                                std::string relPath = ((std::string)p.path()).substr(commitFolderPath.length());
                                std::string rootPath = root + relPath;
                                std::string revertPath = root + "/.imperium/.commit/" + passedHash + relPath;
                                struct stat a;
                                struct stat b;
                                if (stat(rootPath.c_str(), &a) == 0)
                                {
                                    if (compareFiles(rootPath, p.path()))
                                    {
                                        std::ifstream ifile(p.path().c_str(), std::ifstream::in);
                                        std::ofstream ofile(rootPath, std::ofstream::out | std::ofstream::app);
                                        ofile << "\n \nYOUR CHANGES________<<<<<<<<<<<<<<<<<<<\n \n_________INCOMING CHANGES>>>>>>>>>>>>>>>>\n \n";
                                        ofile << ifile.rdbuf();
                                        std::cout << "\033[1;31mMERGE CONFLICT IN : \033[0m" << rootPath << "\n";
                                    }
                                    else
                                    {
                                        continue;
                                    }
                                }
                                else
                                {
                                    fs::copy_file(p.path(), rootPath, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                                }
                            }
                        }
                    }

                    for (auto &p : fs::recursive_directory_iterator(root + "/.imperium/.commit/" + passedHash))
                    {
                        std::string relPath = ((std::string)p.path()).substr((root + "/.imperium/.commit/" + passedHash).length());
                        struct stat s;
                        if (stat(p.path().c_str(), &s) == 0)
                        {
                            if (s.st_mode & S_IFDIR)
                                continue;
                            else if (s.st_mode & S_IFREG)
                            {
                                struct stat a;
                                struct stat b;
                                std::string rootPath = root + relPath;
                                std::string prevPath = commitFolderPath + relPath;
                                if (stat(rootPath.c_str(), &a) == 0 && stat(prevPath.c_str(), &b) != 0)
                                {
                                    if (compareFiles(rootPath, p.path()) == 0)
                                        remove(rootPath.c_str());
                                    else
                                    {
                                        std::ofstream ofile(rootPath, std::ofstream::out | std::ofstream::app);
                                        ofile << "\n\nTHIS FILE WASN'T PRESENT IN THE previous checkpoint,\nhowever your changes in the succeeesing commits are saved here\n\n";
                                        std::cout << "\033[1;31mMERGE CONFLICT IN : \033[0m" << rootPath << "\n";
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                std::string commitFolderPath = root + "/.imperium/.commit/" + commitFolderName;
                for (auto &p : fs::recursive_directory_iterator(commitFolderPath))
                {
                    struct stat s;
                    std::string relPath = ((std::string)p.path()).substr((root + "/.imperium/.commit/" + commitFolderName).length());
                    if (stat(p.path().c_str(), &s) == 0)
                    {
                        if (s.st_mode & S_IFDIR)
                            continue;
                        else if (s.st_mode & S_IFREG)
                        {
                            struct stat a;
                            std::string rootPath = root + relPath;
                            if (stat(rootPath.c_str(), &a) == 0)
                            {
                                if (compareFiles(rootPath, p.path()) == 0)
                                    remove(rootPath.c_str());
                                else
                                {
                                    std::ofstream ofile(rootPath, std::ofstream::out | std::ofstream::app);
                                    ofile << "\n\nTHIS FILE WAS CREATED IN THE REVERTED COMMIT,\nhowever your changes in the succeeesing commits are saved here\n\n";
                                    std::cout << "\033[1;31mMERGE CONFLICT IN : \033[0m" << rootPath << "\n";
                                }
                            }
                        }
                    }
                }
            }

            mergeConflict();
        }
        else
        {
            std::cout << "\033[1;31mYou've changes that need to be commited or stashed.\033[0m\n";
        }
    }
    else
    {
        std::cout << "Incorrect Hash provided"
                  << "\n";
        return;
    }
}

int conflictCheck()
{
    std::ifstream cfile;
    std::string line;
    cfile.open(root + "/.imperium/conflict");
    std::getline(cfile, line);
    cfile.close();
    if (line == "true")
        return 1;
    return 0;
}

void status()
{
    struct stat buf;
    std::ifstream readAddLog;
    std::string line, headHash;
    std::vector<std::string> staged;
    std::vector<std::string> type;
    std::vector<std::string> notstaged;
    std::ifstream readCommitLog;
    std::string stagedPath;

    readCommitLog.open(root + "/.imperium/commit.log");
    std::getline(readCommitLog, headHash);

    headHash = headHash.substr(0, 40);
    readCommitLog.close();

    if (stat((root + "/.imperium/.add").c_str(), &buf) == 0)
    {
        readAddLog.open(root + "/.imperium/add.log");

        while (std::getline(readAddLog, line))
        {
            stagedPath = line.substr(root.length() + 1, line.length() - root.length() - 4);
            if (stat((root + "/.imperium/.add/" + stagedPath).c_str(), &buf) == 0)
            {
                if ((stat((root + "/.imperium/.commit/" + headHash + stagedPath).c_str(), &buf) != 0) || (stat((root + "/.imperium/.commit").c_str(), &buf) != 0))
                {
                    type.push_back("created: ");
                    staged.push_back(stagedPath);
                }
                if (std::find(staged.begin(), staged.end(), stagedPath) == staged.end())
                {
                    type.push_back("modified: ");
                    staged.push_back(stagedPath);
                }
                if (stat((root + stagedPath).c_str(), &buf) != 0)
                {
                    notstaged.push_back("deleted: " + stagedPath);
                }
                else
                {
                    if (compareFiles(root + stagedPath, root + "/.imperium/.add" + stagedPath))
                    {
                        notstaged.push_back("modified: " + stagedPath);
                    }
                }
            }
        }
    }
    readAddLog.close();
    struct stat s;
    if (stat((root + "/.imperium/.commit").c_str(), &buf) == 0)
    {
        for (auto &p : fs::recursive_directory_iterator(root))
        {
            if (stat(p.path().c_str(), &s) == 0)
            {
                if (s.st_mode & S_IFREG)
                {
                    std::string rootPath = p.path();

                    if (toBeIgnored(p.path().c_str(), 1))
                        continue;

                    std::string commitPath = root + "/.imperium/.commit/" + headHash + rootPath.substr(root.length());

                    if (stat((commitPath).c_str(), &s) != 0 && (std::find(staged.begin(), staged.end(), rootPath.substr(root.length())) == staged.end()))
                    {
                        for (int i = 0; i < staged.size(); i++)
                        {
                            std::cout << staged[i] << "\n";
                        }
                        notstaged.push_back("created: " + rootPath.substr(root.length()));
                    }
                    else
                    {
                        if (compareFiles(rootPath, commitPath))
                        {
                            if (std::find(staged.begin(), staged.end(), rootPath.substr(root.length())) == staged.end() && std::find(notstaged.begin(), notstaged.end(), stagedPath) == notstaged.end())
                            {
                                notstaged.push_back("modified: " + rootPath.substr(root.length()));
                            }
                        }
                    }
                }
                else
                {
                    continue;
                }
            }
        }
        for (auto &p : fs::recursive_directory_iterator(root + "/.imperium/.commit/" + headHash))
        {
            struct stat s;
            if (stat(p.path().c_str(), &s) == 0)
            {
                if (s.st_mode & S_IFREG)
                {
                    std::string commitPath = p.path();
                    std::string rootPath = root + commitPath.substr(root.length() + 59);

                    if (toBeIgnored(rootPath.c_str(), 1))
                        continue;

                    if (stat((rootPath).c_str(), &s) != 0)
                    {
                        // if(std::find(notstaged.begin(), notstaged.end(), rootPath.substr(root.length())) != notstaged.end())
                        notstaged.push_back("deleted: " + rootPath.substr(root.length()));
                    }
                }
                else
                {
                    continue;
                }
            }
        }
    }
    if (stat((root + "/.imperium/.commit").c_str(), &buf) != 0)
    {
        for (auto &p : fs::recursive_directory_iterator(root))
        {
            struct stat s;
            if (stat(p.path().c_str(), &s) == 0)
            {
                if (s.st_mode & S_IFREG)
                {
                    std::string rootPath = p.path();

                    if (toBeIgnored(rootPath.c_str(), 1))
                        continue;

                    if (stat((root + "/.imperium/.add").c_str(), &buf) != 0 || (std::find(staged.begin(), staged.end(), rootPath.substr(root.length())) == staged.end()))
                    {
                        notstaged.push_back("created: " + rootPath.substr(root.length()));
                    }
                }
                else
                {
                    continue;
                }
            }
        }
    }
    if (notstaged.size())
        std::cout << "Changes not staged for commit:\n\n";
    for (int i = 0; i < notstaged.size(); i++)
    {
        std::cout << notstaged[i] << "\n";
    }
    if (staged.size())
        std::cout << "\nChanges staged for commit:\n\n";
    for (int i = 0; i < staged.size(); i++)
    {
        std::cout << type[i] << staged[i] << "\n";
    }
    if (!notstaged.size() && !staged.size())
        std::cout << "Up to date\n";
    staged.clear();
    notstaged.clear();
}

void resolve()
{
    std::ofstream cfile;
    cfile.open(root + "/.imperium/conflict", std::ofstream::trunc);
    cfile << "false\n";
    cfile.close();
}

void checkout(char **argv)
{
    if (argv[2] == "")
    {
        std::cout << "Please provide a Hash"
                  << "\n";
        return;
    }
    std::string checkoutCommitHash = argv[2];
    std::string commitFolderName = checkoutCommitHash.substr(0, 40);
    bool directoryExists = false;
    std::ifstream commitLog;
    std::string commitLine;
    commitLog.open(root + "/.imperium/commit.log");
    while (std::getline(commitLog, commitLine))
    {
        //std::cout << commitLine.substr(0, 40) << "\n";
        if (commitFolderName == commitLine.substr(0, 40))
        {
            directoryExists = true;
            break;
        }
    }
    commitLog.close();
    if (!directoryExists)
    {
        std::cout << "Incorrect Hash provided"
                  << "\n";
        return;
    }
    std::string commitFolderPath = root + "/.imperium/.commit/" + commitFolderName;
    fs::copy(commitFolderPath, root + "/", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
}

int main(int argc, char **argv)
{
    const char *dir = getenv("dir");
    root = dir;
    if (strcmp(argv[1], "resolve") == 0)
    {
        resolve();
    }
    else if (conflictCheck() == 0)
    {
        if (strcmp(argv[1], "init") == 0)
        {
            init(dir);
        }
        if (strcmp(argv[1], "add") == 0)
        {
            add(argv);
        }
        else if (strcmp(argv[1], "log") == 0)
        {
            getCommitLog();
        }

        else if (strcmp(argv[1], "commit") == 0)
        {
            commit(argv);
        }
        else if (strcmp(argv[1], "checkout") == 0)
        {
            checkout(argv);
        }
        else if (strcmp(argv[1], "revert") == 0)
        {
            revert(argv);
        }
        else if (strcmp(argv[1], "status") == 0)
        {
            status();
        }
    }
    else
    {
        std::cout << "\033[1;31mYou've to make a commit, after resolving any conflicts, if there, type 'imperium resolve'\033[0m\n";
    }
}
