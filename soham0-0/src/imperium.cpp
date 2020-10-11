
#include "imperium.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h> 
#include <sys/types.h>
#include <filesystem>
#include <cstring>
#include <string>
#include <ctime>
#include <openssl/sha.h>
#include <vector>
#include <algorithm>

bool imperium::frozen(){
    std::fstream fileReader;
    fileReader.open((root+"/.imperium/conflict").c_str(), std::fstream::in);
    std::string query;
    std::getline(fileReader, query);
    if(query == "true") return true;
    return false;
}

void imperium::resolve(){
    std::cout << "Are you sure that you have resolved the conflict?(y/n)" << std::endl;
    char c; std::cin >> c;
    if(c != 'y')    return ;
    std::fstream fileWriter;
    fileWriter.open ((root+"/.imperium/conflict").c_str(), std::fstream::out | std::fstream::trunc);
    fileWriter << "false\n";
    fileWriter.close();
}

imperium::imperium(){
    if(getenv("dir")){
        root = getenv("dir");
    }else{
        std::cout << "No environment variable found!" << std::endl;
        exit(0);
    }
}

void imperium::getHelp(std::string helpQuery){
    bool allHelp = false;
    if(helpQuery == ""){
        allHelp = true;
	std::cout << "Usage: imperium [--help <query>] [init] [add <arguments>] [commit \"<mesasge>\"] [log [--oneline]] [checkout <commit_hash>/HEAD~#] [revert <commit_hash>/HEAD~#] [status]" << std::endl;
    }
    if(allHelp || helpQuery == "init"){
	    std::cout << "Start a working area: \n\tinit\t\tInitialize an Empty repositorty in current or specified folder.\n\tUsage:\t\timperium init <optional relative directory path>" << std::endl;
    }
    if(allHelp || helpQuery == "add"){
	    std::cout << "Work on current change:\n\tadd\t\tAdd current state to index.\n\tUsage:\t\timperium add [.] [list of arguments]" << std::endl;
    }
    if(allHelp || helpQuery == "commit"){
	    std::cout << "Commit changes to the history:\n\tcommit\t\tRecord changes to the repository.\n\tUsage:\t\timperium commit <message>" <<std::endl;
    }
    if(allHelp || helpQuery == "status"){
	    std::cout << "Check status of current working area:\n\tstatus\t\tCheck status of changes in the repository\n\tUsage:\t\timperium status" <<std::endl;
    }
    if(allHelp || helpQuery == "checkout"){
	    std::cout << "Travel back to a particular point of time:\n\tcheckout\tGo back to the version of repository at a previous commit.\n\tUsage:\t\timperium checkout <commit_hash>/HEAD~#\t(HEAD~# is equivalent to the hash of # commits before HEAD)" <<std::endl;
    }
    if(allHelp || helpQuery == "log"){
	std::cout << "View the log of commits:\n\tlog\t\tPrint the log of commits to the repository.\n\tUsage:\t\timperium log [--oneline]\t\t(use the --oneline flag to shrink the information horizontally)" <<std::endl;
    }
    if(allHelp || helpQuery == "revert"){
	    std::cout << "Undo/Remove changes made at a particular time:\n\trevert\t\tRevert the changes made in a particular commit\n\tUsage:\t\timperium revert <commit_hash>/HEAD~#\t(HEAD~# is equivalent to the hash of # commits before HEAD)" <<std::endl;
    }
    if(allHelp || helpQuery == "resolve"){
	std::cout << "In case there is a conflict the user needs to resolve it and then call this function to restore the normal functions, until then all commands are frozen.\n\tresolve\t\tUnfreeze the commands.\n\tUsage:\t\timperium resolve" <<std::endl;
    }
    return ;
}

std::string imperium::headHunter(std::string hashQuery){
    isRepo();
    
    if(hashQuery.size()>5 && hashQuery.substr(0,5) == "HEAD~"){
        long int headNum = 1 + std::stoi(hashQuery.substr(5));
        if(headNum>0){
            std::fstream fileReader;
            fileReader.open((root+"/.imperium/commit.log" ).c_str(), std::fstream::in);
            std::string commitEntry;
            while (headNum-- && std::getline (fileReader, commitEntry));
            std::string hash = ""; 
            if(commitEntry.size()>53){
                hash = commitEntry.substr(7,47);
            }
            fileReader.close();
            return hash;
        }
    }
    else if(hashQuery.size() == 40 || hashQuery.size() == 6){
        return hashQuery;
    }
    return "";
}

std::string imperium::relativePath(std::string path){
    int itr = root.size();
    if(path.substr(0, itr) == root){
        return path.substr(itr+1);
    }
    return path;
}

std::string imperium::doesExist(std::string path){
    struct stat sb;
    if(!stat(path.c_str(), &sb)){
        if(S_ISDIR(sb.st_mode)){
            return "directory";
        }
        else if(S_ISREG(sb.st_mode)){
            return "file";
        }
    }
    return "\0";
}

void imperium::isRepo(){
    bool flag = false;
    if(doesExist(root + "/.imperium")!="directory")             flag = true;
    if(doesExist(root + "/.imperium/conflict")!="file")         flag = true;
    if(doesExist(root + "/.imperium/commit.log")!="file")       flag = true;
    if(doesExist(root + "/.imperium/add.log")!="file")          flag = true;
    if(doesExist(root + "/.imperiumIgnore")!="file")            flag = true;
    if(flag){
        std::cout << "Fatal Error: Not An Imperium Repository" << std::endl;
        exit(0);
    }
    return ;
}

void imperium::setRoot(std::string path){
    root += "/" + path;
    if((mkdir((root).c_str(), 0777))==-1){
        std::cerr << "Error: " << strerror(errno) << std::endl;
    } 
}

void imperium::init(){
    if(doesExist(root+"/.imperium")!="\0"){
        std::cout<<"Repository has already been initialized" << std::endl;
        return ;
    }

    std::filesystem::create_directories(root+"/.imperium"); 

    std::fstream fileWriter;
    fileWriter.open ((root+"/.imperiumIgnore").c_str(), std::fstream::out | std::fstream::app);
    fileWriter << "/.imperium\n/.imperiumIgnore\n/.node_modules\n/.env\n";
    fileWriter.close(); 

    fileWriter.open ((root+"/.imperium/conflict").c_str(), std::fstream::out | std::fstream::app);
    fileWriter << "false\n";
    fileWriter.close(); 

    fileWriter.open ((root+"/.imperium/commit.log").c_str(), std::fstream::out | std::fstream::app);
    fileWriter << "";
    fileWriter.close(); 

    fileWriter.open ((root+"/.imperium/add.log").c_str(), std::fstream::out | std::fstream::app);
    fileWriter << "";
    fileWriter.close();

    std::cout << "Initialized empty imperium repository at " << root << std::endl;
    return ;
}

bool imperium::isIgnored(std::string path){
    std::fstream fileReader;
    fileReader.open((root+"/.imperiumIgnore" ).c_str(), std::fstream::in);
    std::string ignoredPath;
    path = relativePath(path);
    while (std::getline (fileReader, ignoredPath)) {
        if(ignoredPath == "/" + path.substr(0, ignoredPath.size()-1))   return true;
    }
    fileReader.close();
    return false;
}

void imperium::addToLog(std::string path){
    std::fstream fileReaderWriter;
    fileReaderWriter.open((root+"/.imperium/add.log" ).c_str(),std::fstream::in);
    std::string loggedPath;
    while (std::getline (fileReaderWriter, loggedPath)) {
        if(loggedPath == path) return ;
    } 
    fileReaderWriter.close();
    fileReaderWriter.open((root+"/.imperium/add.log" ).c_str(),std::fstream::app);
    fileReaderWriter << path << std::endl;
    fileReaderWriter.close();
}

void imperium::addToCache(std::string path){
    if(doesExist(root+"/.imperium/.add")!= "directory"){
        std::filesystem::create_directories(root+"/.imperium/.add");
    }
    if(doesExist(path) == "directory"){
        if(doesExist(root + "/.imperium/.add/" + relativePath(path))!="directory"){
            std::filesystem::create_directories(root + "/.imperium/.add/" + relativePath(path));
        }
    }else{
        std::filesystem::path p1 = root + "/.imperium/.add/" + relativePath(path);
        std::filesystem::create_directories(p1.parent_path());
        std::filesystem::copy(path, p1, std::filesystem::copy_options::update_existing);
    }
    return ;
}

void imperium::add(std::string path){
    std::string type;
    isRepo();
    if(path!=".") {
        type = doesExist(root + "/" + path);
    }
    else {
        purgeAdd();
        path = "";
        type = "directory";
    }
    if(type=="\0"){
        std::cout << path << "No such file/directory exits!" << std::endl;
        return ;
    }
    if(isIgnored(path)) return ; 
    if(type == "directory"){
        for(auto &subDir : std::filesystem::recursive_directory_iterator(root +"/"+ path)){
            if(!isIgnored(subDir.path()) && subDir.path()!=root){
                addToLog(subDir.path());
                addToCache(subDir.path());
            }
        }
    }
    else if(type == "file"){
        addToLog(root + "/" + path);
        addToCache(root + "/" + path);
    }

    return ;
}

void imperium::purgeAdd(){
    std::filesystem::remove_all(root + "/.imperium/.add");
    std::fstream fileWriter;   
    fileWriter.open ((root+"/.imperium/add.log").c_str(), std::fstream::out | std::fstream::trunc);
    fileWriter.close();
}

std::string imperium::getTime(){
        time_t now = time(0);
	    tm *gmtm = gmtime(&now);
	    return asctime(gmtm);
    }

std::string imperium::getHash(std::string input){
    unsigned char str[500];
	strcpy( (char *)( str ), input.c_str() );
	unsigned char hash[20]; 
	SHA1(str, strlen((char *)str), hash);
	char ans[41]; ans[40]='\0';
	for(int i=0; i<20; i++){
		sprintf(&ans[2*i], "%02x", hash[i]);
	}
    return ans;
}

void imperium::updateCommitLog(std::string message, std::string commitHash){
    std::fstream fileReader;
    fileReader.open((root+"/.imperium/commit.log" ).c_str(), std::fstream::in);
    std::fstream fileWriter;   
    fileWriter.open ((root+"/.imperium/temp.log").c_str(), std::fstream::in | std::fstream::app);
    std::string commitEntry;
    fileWriter << "commit " + commitHash + " HEAD " + message + "\n";
    while (std::getline (fileReader, commitEntry)) {
        fileWriter << commitEntry.substr(0, 48) + "     " + commitEntry.substr(53) + "\n";
    }
    if(rename((root+"/.imperium/temp.log").c_str(), (root+"/.imperium/commit.log" ).c_str()) != 0){
        std::cerr << "Commit Failed! Error: " << strerror(errno) << std::endl;
        exit(0);
    }
    fileWriter.close();
}

void imperium::commit(std::string message){
    isRepo();
    if(doesExist(root + "/.imperium/.add") != "directory"){
        std::cout << "Nothing to commit. No staged files/folders." << std::endl;
        return ;
    }
    std::string commitHash = getHash(message), temp, previousCommit = "";
    std::fstream fileReader;
    fileReader.open((root+"/.imperium/commit.log" ).c_str(),std::fstream::in);
    std::getline(fileReader, previousCommit);
    fileReader.close();
    std::filesystem::create_directories(root + "/.imperium/.commit/" + commitHash);
    if(previousCommit != ""){
        previousCommit = previousCommit.substr(7, 40);
        std::filesystem::copy(root + "/.imperium/.commit/" + previousCommit, root + "/.imperium/.commit/" + commitHash, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
    }
    std::filesystem::copy(root + "/.imperium/.add", root + "/.imperium/.commit/" + commitHash, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
    updateCommitLog(message, commitHash);
    purgeAdd();
    return ;
}

void imperium::getCommitLog(std::string flag){
    isRepo();
    std::fstream fileReader;
    fileReader.open((root+"/.imperium/commit.log" ).c_str(), std::fstream::in);
    std::string commitEntry;
    while (std::getline (fileReader, commitEntry)) {
        if(flag == "--oneline"){
            commitEntry = commitEntry.substr(7, 6) + " " + commitEntry.substr(48, 4) + " " + commitEntry.substr(53);
        }
        std::cout << commitEntry << std::endl;
    }
    fileReader.close();
    return ;
}

void imperium::checkout(std::string hash){
    isRepo();
    
    std::fstream fileReader;
    fileReader.open((root+"/.imperium/commit.log" ).c_str(), std::fstream::in);
    std::string commitEntry, hashFolder = "";
    while (std::getline (fileReader, commitEntry)) {
        if(hash.size()==6 && hash == commitEntry.substr(7, 6)){
            hashFolder = commitEntry.substr(7, 40);
            break;
        }
        else if(hash == commitEntry.substr(7, 40)){
            hashFolder = commitEntry.substr(7, 40);
            break;
        }
    }
    fileReader.close();

    if(hashFolder == ""){
        std::cout << "Commit hash mismatch. Please Enter a correct commit Hash." << std::endl;
        exit(0);
    }
    std::filesystem::copy((root + "/.imperium/.commit/" + hashFolder).c_str(), root.c_str(),std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
    purgeAdd();
    
    fileReader.open((root+"/.imperium/commit.log" ).c_str(), std::fstream::in);
    std::fstream fileWriter;   
    fileWriter.open ((root+"/.imperium/temp.log").c_str(), std::fstream::in | std::fstream::app);
    while (std::getline (fileReader, commitEntry)) {
        if(hashFolder == commitEntry.substr(7, 40)){
            fileWriter << commitEntry.substr(0, 47) + " HEAD " + commitEntry.substr(53) + "\n";
        }
        else fileWriter << commitEntry.substr(0, 48) + "     " + commitEntry.substr(53) + "\n";
    }
    fileWriter.close();
    fileReader.close();
    if(rename((root+"/.imperium/temp.log").c_str(), (root+"/.imperium/commit.log" ).c_str()) != 0){
        std::cerr << "Checkout Failed! Error: " << strerror(errno) << std::endl;
        exit(0);
    }
    std::cout << "HEAD set to " << hashFolder << " stage cleared." << std::endl;
    return ;
}

bool imperium::isSame(std::string p1, std::string p2) {

    if(doesExist(p1)=="directory" && doesExist(p2)=="directory"){
        return true;
    }

    std::ifstream f1(p1, std::ifstream::in|std::ifstream::ate);
    std::ifstream f2(p2, std::ifstream::in|std::ifstream::ate);

    if (f1.fail() || f2.fail()) {
    return false;
    }

    if (f1.tellg() != f2.tellg()) {
    return false;
    }

    f1.seekg(0, std::ifstream::beg);
    f2.seekg(0, std::ifstream::beg);
    return std::equal(  std::istreambuf_iterator<char>(f1.rdbuf()),
                        std::istreambuf_iterator<char>(),
                        std::istreambuf_iterator<char>(f2.rdbuf()));
}

void imperium::getStatus(){
    isRepo();
    std::vector <std::string> staged, notStaged;
    std::fstream fileReader;
    fileReader.open((root+"/.imperium/commit.log" ).c_str(), std::fstream::in);
    std::string commitEntry, headHash = "%%%";
    while (std::getline (fileReader, commitEntry)) {
        if(commitEntry.substr(48, 4)=="HEAD"){
            headHash = commitEntry.substr(7, 40);
            break;
        }
    }
    fileReader.close();

    fileReader.open((root + "/.imperium/add.log").c_str(), std::fstream::in);
    std::string addEntry;
    while(std::getline(fileReader, addEntry)){
        addEntry = relativePath(addEntry);
        if(doesExist(root + "/.imperium/.commit/" + headHash + "/" + addEntry)=="\0"){
            staged.push_back("created: " + addEntry);
        }
        else if(!isSame(root + "/.imperium/.add/" + addEntry, root + "/.imperium/.commit/" + headHash + "/" + addEntry)){
            staged.push_back("modified: " + addEntry);
        }

        if(!isSame(root + "/.imperium/.add/" + addEntry, root + "/" + addEntry)){
            notStaged.push_back("modified: " + addEntry);
        }
    }
    fileReader.close();

    if(doesExist(root + "/.imperium/.commit/" + headHash)=="directory"){
        for(auto &subDir : std::filesystem::recursive_directory_iterator(root)){
            if(!isIgnored(subDir.path())){
                if(doesExist(root + "/.imperium/.commit/" + headHash + "/" + relativePath(subDir.path())) != "\0" && doesExist(root + "/.imperium/.add/" + addEntry) == "\0"){
                    if(!isSame(subDir.path(), root + "/.imperium/.commit/" + headHash + "/" + relativePath(subDir.path()))){
                        notStaged.push_back("modified: " + relativePath(subDir.path()));
                    }
                }
            }
        }
    }

    for(auto &subDir : std::filesystem::recursive_directory_iterator(root)){
        if(!isIgnored(subDir.path())){
            if(doesExist(root + "/.imperium/.add/" + relativePath(subDir.path())) == "\0" && doesExist(root + "/.imperium/.commit/" + headHash + "/" + relativePath(subDir.path())) == "\0"){
                notStaged.push_back("created: " + relativePath(subDir.path()));
            }
        }
    }

    sort(staged.begin(), staged.end());
    sort(notStaged.begin(), notStaged.end());

    std::cout << "Staged:\n-------" << std::endl;
    for(auto s: staged){
        std::cout << s << std::endl;
    }
    std::cout << std::endl << "Not Staged:\n----------" << std::endl;
    for(auto n: notStaged){
        std::cout << n << std::endl;
    }
    return ;
} 

void imperium::revert(std::string passedHash){
    isRepo();
    if(doesExist(root + "/.imperium/.add")=="directory"){
        std::cout << "Error: Reverting will overwrite the uncommited changes. Commit the changes and try again.\nAborting!" << std::endl;
        exit(0);
    }
    std::ifstream fileReader;
    fileReader.open((root+"/.imperium/commit.log" ).c_str(), std::fstream::in) ;
    std::string reader = "", lastHash = "", revertHash = "", headHash = "";
    while (std::getline (fileReader, reader)) {
        lastHash = reader.substr(7, 40);
        if(reader.substr(48, 4)=="HEAD") headHash = lastHash;
        if(revertHash.size() ==  40 && (revertHash == passedHash || revertHash.substr(0, 6) == passedHash)){
            break;
        }
        // std::cout << "." << std::endl;
        revertHash = lastHash;
    }
    fileReader.close();

    if(lastHash == ""){
        std::cout << "Error: No commits made previously." << std::endl;
        exit(0);
    }
    else if(!(revertHash.size() ==  40 && (revertHash == passedHash || revertHash.substr(0, 6) == passedHash))){
        std::cout << "Error: Commit Hash mismatch." << std::endl;
    }
    
    if(revertHash == lastHash){
        for(auto &subDir: std::filesystem::recursive_directory_iterator(root + "/.imperium/.commit/" + revertHash)){
            if(subDir.path() != root + "/.imperium/.commit/" + revertHash && doesExist(subDir.path())=="file"){
                std::string relPath = "/" + relativePath(subDir.path()).substr(58);
                if(isSame(root + relPath, subDir.path())){
                    std::filesystem::remove_all(root + relPath);
                }
                else{
                    std::cout << "Found conflict in " + root + relPath << std::endl;
                    std::fstream fileWriter;
                    fileWriter.open(root + relPath, std::fstream::app);
                    fileWriter << "## Current Files\n";
                    fileWriter << "## This file was created in your reverted commit,"<< passedHash <<", but your subsequent changes have been preserved.\n";
                    fileWriter.close();
                    fileWriter.open((root + "/.imperium/conflict").c_str(), std::fstream::out | std::fstream::trunc);
                    fileWriter << "true\n";
                    fileWriter.close();
                }
            }
        }
    }
    else if(revertHash == headHash){
        std::filesystem::copy((root + "/.imperium/.commit/" + lastHash).c_str(), (root).c_str(), std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
        for(auto &subDir: std::filesystem::recursive_directory_iterator(root)){
            if(!isIgnored(subDir.path()) && subDir.path()!=root){
                std::string temPath = root + "/.imperium/.commit/" + lastHash + "/" + relativePath(subDir.path());
                if(doesExist(subDir.path())!=doesExist(temPath)){
                    std::filesystem::remove_all(subDir.path());
                }
            }
        }
    }
    else{
        for(auto &subDir: std::filesystem::recursive_directory_iterator(root + "/.imperium/.commit/" + lastHash)){
            if(subDir.path() != root + "/.imperium/.commit/" + lastHash){
                std::string relPath = "/" + relativePath(subDir.path()).substr(58);
                // std::cout
                if(doesExist(subDir.path())=="file" && !isSame(root + relPath, subDir.path())){
                    std::cout << "Found conflict in " + root + relPath << std::endl;
                    std::fstream fileReader, fileWriter;
                    fileWriter.open((root + relPath).c_str(), std::fstream::app);
                    fileReader.open(subDir.path().c_str(), std::fstream::in);
                    fileWriter << "## MERGE CONFLICT, CURRENT FILE ##\n";
                    std::string tempcontents;
                    while(std::getline(fileReader, tempcontents)){
                        fileWriter << tempcontents + "\n";
                    }
                    fileReader.close();
                    fileWriter.close();
                    fileWriter.open((root + "/.imperium/conflict").c_str(), std::fstream::out | std::fstream::trunc);
                    fileWriter << "true\n";
                    fileWriter.close();
                }
                else{
                    std::filesystem::copy((root + "/.imperium/.commit/" + lastHash).c_str(), (root + relPath).c_str(), std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
                }
            }
        }
        for(auto &subDir: std::filesystem::recursive_directory_iterator(root + "/.imperium/.commit/" + revertHash)){
            if(subDir.path() == root + "/.imperium/.commit/" + revertHash){
                std::string relPath = "/" + relativePath(subDir.path()).substr(58);
                bool existsInLastHash = (doesExist(subDir.path())==doesExist(root + "/.imperium/.commit/" + lastHash + relPath)),
                     existsInHeadHash = (doesExist(subDir.path())==doesExist(root + "/.imperium/.commit/" + headHash + relPath));
                if(!existsInLastHash && existsInHeadHash && !isSame(subDir.path(), root + "/.imperium/.commit/" + headHash + relPath)){
                    std::cout << "Found conflict in " + root + relPath << std::endl;std::fstream fileWriter;
                    fileWriter.open(root + relPath, std::fstream::app);
                    fileWriter << "## Current Files ##\n";
                    fileWriter << "## This file was created in your reverted commit " +revertHash+ " , but your subsequent changes have been preserved. ##\n";
                    fileWriter.close();
                    fileWriter.open((root + "/.imperium/conflict").c_str(), std::fstream::out | std::fstream::trunc);
                    fileWriter << "true\n";
                    fileWriter.close();
                }
                else {
                    std::filesystem::remove_all(root + relPath);
                }

            }
        }
    }
    
    fileReader.open((root+"/.imperium/commit.log" ).c_str(), std::fstream::in);
    std::fstream fileWriter;   
    fileWriter.open ((root+"/.imperium/temp.log").c_str(), std::fstream::in | std::fstream::app);
    std::string commitEntry;
    while (std::getline (fileReader, commitEntry)) {
        std::string Hash = commitEntry.substr(7, 40);
        if(Hash == revertHash){
            if(commitEntry.substr(48, 4) == "HEAD"){
                if(std::getline (fileReader, commitEntry)){
                fileWriter << commitEntry.substr(0, 47) + " HEAD " + commitEntry.substr(53) + "\n";
                };
            }

            continue;
        }
        fileWriter << commitEntry + "\n";
    }
    if(rename((root+"/.imperium/temp.log").c_str(), (root+"/.imperium/commit.log" ).c_str()) != 0){
        std::cerr << "Commit Failed! Error: " << strerror(errno) << std::endl;
        exit(0);
    }
    fileWriter.close();
    std::filesystem::remove_all(root + "/.imperium/.commit/" + revertHash);
    std::cout << "Commit " + revertHash + " reverted sucessfully." <<   std::endl;
    return ;
}

void imperium::encrypt(){
    if(doesExist(root + "/.imperium")!="directory") return ;
    system(("tar cz -P " + root + "/.imperium | openssl enc -e -aes-256-cbc -pbkdf2 -k m4Y7H3SoUrc383WI7hyoU -out .imperium.tar.gz.dat").c_str());
    system(("mv .imperium.tar.gz.dat " + root + "/.imperium.tar.gz.dat").c_str());
    system(("rm -r " + root + "/.imperium").c_str());
}

void imperium::decrypt(){
    if(doesExist(root + "/.imperium.tar.gz.dat")!="file") return ;
    system(("openssl enc -d -aes-256-cbc -pbkdf2 -k m4Y7H3SoUrc383WI7hyoU -in " + root + "/.imperium.tar.gz.dat | tar xz -P").c_str());
    system(("rm -r " + root + "/.imperium.tar.gz.dat").c_str());
}