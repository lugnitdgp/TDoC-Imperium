#include <iostream>
#include <fstream>
#include <sys/stat.h> 
#include <sys/types.h>
#include <experimental/filesystem>
#include <cstring>
#include <ctime>
#include <openssl/sha.h>
#include <vector>
#include <algorithm>

using namespace std;
namespace fs = experimental::filesystem;

class imperium {
    string root;
    string doesExist(string);
    bool isRepo();
    string getTime();
    string relativePath(string);
    bool isIgnored(string);
    void addToLog(string);
    void addToCache(string);
    string getHash(string);
    void purgeAdd();
    void updateCommitLog(string, string);
    bool isSame(string, string);
    public:
		imperium();

		void getHelp(string);

		void setRoot(string);

		
		void init();

		void add(string);

		void commit(string);

		void getCommitLog(string);

		void checkout(string);

		void revert(string);

		void getStatus();
};

int main(int argc, char **argv){
    if(argc<2){
        cout << "Too few arguments!\n";
        return -1;
    }

    imperium repository;

    if(!strcmp(argv[1], "--help")){
        if(argc >= 3) repository.getHelp(argv[2]);
        else repository.getHelp("");
    }
    else if(!strcmp(argv[1], "init")){
        if(argc==3) repository.setRoot(argv[2]);
        repository.init();
    }
    else if(!strcmp(argv[1], "add")){
        int currentArgumentNumber = 2;
        for(; currentArgumentNumber<argc; currentArgumentNumber++){
            repository.add(argv[currentArgumentNumber]);
        }
    }
    else if(!strcmp(argv[1], "commit")){
        if(argc<3 || argv[2] == ""){
            cout << "Please add your commit message" << endl;
            return -1;
        }
        repository.commit(argv[2]);
    }
    else if(!strcmp(argv[1], "log")){
        if(argc==3 && !strcmp(argv[2], "--oneline"))    repository.getCommitLog(argv[2]);
        else if(argc == 2)                              repository.getCommitLog("");
        else {
            cout << "Flag not recognized!" << endl;
            repository.getHelp("log");
        }
    }
    else if(!strcmp(argv[1], "checkout")){
        if(argc<3 || (strlen(argv[2])!=6 && strlen(argv[2])!=40)){
            cout << "Please enter the commit hash in correct format." << endl;
            repository.getHelp("checkout");
            return -1;
        }
        repository.checkout(argv[2]);
    }
    else if(!strcmp(argv[1], "status")){
        repository.getStatus();
    }
    else if(!strcmp(argv[1], "revert")){
        if(argc<3 || (strlen(argv[2])!=6 && strlen(argv[2])!=40)){
            cout << "Please enter the commit hash in correct format." << endl;
            repository.getHelp("checkout");
            return -1;
        }
        repository.revert(argv[2]);
    }
    else {
        cout << "Fatal Error: Command not recognized." << endl;
        repository.getHelp("");
    }
    return 0;
}


imperium::imperium(){
    if(getenv("dir")){
        root = getenv("dir");
    }else{
        cout << "No environment variable found!" << endl;
        exit(0);
    }
}

void imperium::getHelp(string helpQuery){
    bool allHelp = false;
    if(helpQuery == ""){
        allHelp = true;
	cout << "Usage: imperium [--help <query>] [init] [add <arguments>] [commit \"<mesasge>\"]" << endl;
    }
    if(allHelp || helpQuery == "init"){
	    cout << "Start a working area: \n\tinit\tInitialize an Empty repositorty in current or specified folder.\n\tUsage:\timperium init <optional relative directory path>" << endl;
    }
    if(allHelp || helpQuery == "add"){
	cout << "Work on current change:\n\tadd\tAdd current state to index.\n\tUsage:\timperium add [.] [list of arguments]" << endl;
    }
    if(allHelp || helpQuery == "commit"){
	cout << "Grow, mark and tweak your common history\n\tcommit\tRecord changes to the repository.\n\tUsage:\timperium commit <message>" <<endl;
    }
    return ;
}

string imperium::relativePath(string path){
    int itr = root.size();
    if(path.substr(0, itr) == root){
        return path.substr(itr+1);
    }
    return path;
}

string imperium::doesExist(string path){
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

bool imperium::isRepo(){
    if(doesExist(root + "/.imperium")!="directory")             return false;
    if(doesExist(root + "/.imperium/conflict")!="file")         return false;
    if(doesExist(root + "/.imperium/commit.log")!="file")       return false;
    if(doesExist(root + "/.imperium/add.log")!="file")          return false;
    if(doesExist(root + "/.imperiumIgnore")!="file")            return false;
    return true;
}

void imperium::setRoot(string path){
    root += "/" + path;
    if((mkdir((root).c_str(), 0777))==-1){
        cerr << "Error: " << strerror(errno) << endl;
    } 
}

void imperium::init(){
    if(doesExist(root+"/.imperium")!="\0"){
        cout<<"Repository has already been initialized" << endl;
        return ;
    }

    fs::create_directories(root+"/.imperium"); 

    fstream fileWriter;
    fileWriter.open ((root+"/.imperiumIgnore").c_str(), fstream::out | fstream::app);
    fileWriter << "/.imperium\n/.imperiumIgnore\n/.node_modules\n/.env\n";
    fileWriter.close(); 

    fileWriter.open ((root+"/.imperium/conflict").c_str(), fstream::out | fstream::app);
    fileWriter << "false\n";
    fileWriter.close(); 

    fileWriter.open ((root+"/.imperium/commit.log").c_str(), fstream::out | fstream::app);
    fileWriter << "";
    fileWriter.close(); 

    fileWriter.open ((root+"/.imperium/add.log").c_str(), fstream::out | fstream::app);
    fileWriter << "";
    fileWriter.close();

    cout << "Initialized empty imperium repository at " << root << endl;
    return ;
}

bool imperium::isIgnored(string path){
    fstream fileReader;
    fileReader.open((root+"/.imperiumIgnore" ).c_str(), fstream::in);
    string ignoredPath;
    path = relativePath(path);
    while (getline (fileReader, ignoredPath)) {
        if(ignoredPath == "/" + path.substr(0, ignoredPath.size()-1))   return true;
    }
    fileReader.close();
    return false;
}

void imperium::addToLog(string path){
    fstream fileReaderWriter;
    fileReaderWriter.open((root+"/.imperium/add.log" ).c_str(),fstream::in);
    string loggedPath;
    while (getline (fileReaderWriter, loggedPath)) {
        if(loggedPath == path) return ;
    } 
    fileReaderWriter.close();
    fileReaderWriter.open((root+"/.imperium/add.log" ).c_str(),fstream::app);
    fileReaderWriter << path << endl;
    fileReaderWriter.close();
}

void imperium::addToCache(string path){
    if(doesExist(root+"/.imperium/.add")!= "directory"){
        fs::create_directories(root+"/.imperium/.add");
    }
    if(doesExist(path) == "directory"){
        if(doesExist(root + "/.imperium/.add/" + relativePath(path))!="directory"){
            fs::create_directories(root + "/.imperium/.add/" + relativePath(path));
        }
    }else{
        fs::path p1 = root + "/.imperium/.add/" + relativePath(path);
        fs::create_directories(p1.parent_path());
        fs::copy(path, p1, fs::copy_options::update_existing);
    }
    return ;
}

void imperium::add(string path){
    string type;
    if(!isRepo()){
        cout << "Fatal Error: Not An Imperium Repository" << endl;
        exit(0);
    }
    if(path!=".") {
        type = doesExist(root + "/" + path);
    }
    else {
        purgeAdd();
        path = "";
        type = "directory";
    }
    if(type=="\0"){
        cout << path << "No such file/directory exits!" << endl;
        return ;
    }
    if(isIgnored(path)) return ; 
    if(type == "directory"){
        for(auto &subDir : fs::recursive_directory_iterator(root +"/"+ path)){
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
    fs::remove_all(root + "/.imperium/.add");
    fstream fileWriter;   
    fileWriter.open ((root+"/.imperium/add.log").c_str(), fstream::out | fstream::trunc);
    fileWriter.close();
}

string imperium::getTime(){
        time_t now = time(0);
	    tm *gmtm = gmtime(&now);
	    return asctime(gmtm);
    }

string imperium::getHash(string input){
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

void imperium::updateCommitLog(string message, string commitHash){
    fstream fileReader;
    fileReader.open((root+"/.imperium/commit.log" ).c_str(), fstream::in);
    fstream fileWriter;   
    fileWriter.open ((root+"/.imperium/temp.log").c_str(), fstream::in | fstream::app);
    string commitEntry;
    fileWriter << "commit " + commitHash + " HEAD " + message + "\n";
    while (getline (fileReader, commitEntry)) {
        fileWriter << commitEntry.substr(0, 48) + "     " + commitEntry.substr(53) + "\n";
    }
    if(rename((root+"/.imperium/temp.log").c_str(), (root+"/.imperium/commit.log" ).c_str()) != 0){
        cerr << "Commit Failed! Error: " << strerror(errno) << endl;
        exit(0);
    }
    fileWriter.close();
}

void imperium::commit(string message){
    if(!isRepo()){
        cout << "Fatal Error: Not An Imperium Repository" << endl;
        exit(0);
    }
    if(doesExist(root + "/.imperium/.add") != "directory"){
        cout << "Nothing to commit. No staged files/folders." << endl;
        return ;
    }
    string commitHash = getHash(message), temp, previousCommit = "";
    fstream fileReader;
    fileReader.open((root+"/.imperium/commit.log" ).c_str(),fstream::in);
    getline(fileReader, previousCommit);
    fileReader.close();
    fs::create_directories(root + "/.imperium/.commit/" + commitHash);
    if(previousCommit != ""){
        previousCommit = previousCommit.substr(7, 40);
        fs::copy(root + "/.imperium/.commit/" + previousCommit, root + "/.imperium/.commit/" + commitHash, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    }
    fs::copy(root + "/.imperium/.add", root + "/.imperium/.commit/" + commitHash, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    updateCommitLog(message, commitHash);
    purgeAdd();
    return ;
}

void imperium::getCommitLog(string flag){
    if(!isRepo()){
        cout << "Fatal Error: Not An Imperium Repository" << endl;
        exit(0);
    }
    fstream fileReader;
    fileReader.open((root+"/.imperium/commit.log" ).c_str(), fstream::in);
    string commitEntry;
    while (getline (fileReader, commitEntry)) {
        if(flag == "--oneline"){
            commitEntry = commitEntry.substr(7, 6) + " " + commitEntry.substr(48, 4) + " " + commitEntry.substr(53);
        }
        cout << commitEntry << endl;
    }
    fileReader.close();
    return ;
}

void imperium::checkout(string hash){
    if(!isRepo()){
        cout << "Fatal Error: Not An Imperium Repository" << endl;
        exit(0);fstream fileReader;
    }

    fstream fileReader;
    fileReader.open((root+"/.imperium/commit.log" ).c_str(), fstream::in);
    string commitEntry, hashFolder = "";
    while (getline (fileReader, commitEntry)) {
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
        cout << "Commit hash mismatch. Please Enter a correct commit Hash." << endl;
        exit(0);
    }
    fs::copy((root + "/.imperium/.commit/" + hashFolder).c_str(), root.c_str(),fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    purgeAdd();

    fileReader.open((root+"/.imperium/commit.log" ).c_str(), fstream::in);
    fstream fileWriter;   
    fileWriter.open ((root+"/.imperium/temp.log").c_str(), fstream::in | fstream::app);
    while (getline (fileReader, commitEntry)) {
        if(hashFolder == commitEntry.substr(7, 40)){
            fileWriter << commitEntry.substr(0, 47) + " HEAD " + commitEntry.substr(53) + "\n";
        }
        else fileWriter << commitEntry.substr(0, 48) + "     " + commitEntry.substr(53) + "\n";
    }
    fileWriter.close();
    fileReader.close();
    if(rename((root+"/.imperium/temp.log").c_str(), (root+"/.imperium/commit.log" ).c_str()) != 0){
        cerr << "Checkout Failed! Error: " << strerror(errno) << endl;
        exit(0);
    }
    cout << "HEAD set to " << hashFolder << " stage cleared." << endl;
    return ;
}

bool imperium::isSame(string p1, string p2) {

    if(doesExist(p1)=="directory" && doesExist(p2)=="directory"){
        return true;
    }

    ifstream f1(p1, ifstream::in|ifstream::ate);
    ifstream f2(p2, ifstream::in|ifstream::ate);

    if (f1.fail() || f2.fail()) {
    return false;
    }

    if (f1.tellg() != f2.tellg()) {
    return false;
    }

    f1.seekg(0, ifstream::beg);
    f2.seekg(0, ifstream::beg);
    return equal(  istreambuf_iterator<char>(f1.rdbuf()),
                        istreambuf_iterator<char>(),
                        istreambuf_iterator<char>(f2.rdbuf()));
}

void imperium::getStatus(){
    if(!isRepo()){
        cout << "Fatal Error: Not An Imperium Repository" << endl;
        exit(0);
    }
    vector <string> staged, notStaged;
    fstream fileReader;
    fileReader.open((root+"/.imperium/commit.log" ).c_str(), fstream::in);
    string commitEntry, headHash = "%%%";
    while (getline (fileReader, commitEntry)) {
        if(commitEntry.substr(48, 4)=="HEAD"){
            headHash = commitEntry.substr(7, 40);
            break;
        }
    }
    fileReader.close();

    fileReader.open((root + "/.imperium/add.log").c_str(), fstream::in);
    string addEntry;
    while(getline(fileReader, addEntry)){
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
        for(auto &subDir : fs::recursive_directory_iterator(root)){
            if(!isIgnored(subDir.path())){
                if(doesExist(root + "/.imperium/.commit/" + headHash + "/" + relativePath(subDir.path())) != "\0" && doesExist(root + "/.imperium/.add/" + addEntry) == "\0"){
                    if(!isSame(subDir.path(), root + "/.imperium/.commit/" + headHash + "/" + relativePath(subDir.path()))){
                        notStaged.push_back("modified: " + relativePath(subDir.path()));
                    }
                }
            }
        }
    }

    for(auto &subDir : fs::recursive_directory_iterator(root)){
        if(!isIgnored(subDir.path())){
            if(doesExist(root + "/.imperium/.add/" + relativePath(subDir.path())) == "\0" && doesExist(root + "/.imperium/.commit/" + headHash + "/" + relativePath(subDir.path())) == "\0"){
                notStaged.push_back("created: " + relativePath(subDir.path()));
            }
        }
    }

    sort(staged.begin(), staged.end());
    sort(notStaged.begin(), notStaged.end());

    cout << "Staged:\n-------" << endl;
    for(auto s: staged){
        cout << s << endl;
    }
    cout << endl << "Not Staged:\n----------" << endl;
    for(auto n: notStaged){
        cout << n << endl;
    }
    return ;
} 

void imperium::revert(string passedHash){
    if(!isRepo()){
        cout << "Fatal Error: Not An Imperium Repository" << endl;
        exit(0);
    }
    if(doesExist(root + "/.imperium/.add")=="directory"){
        cout << "Error: Reverting will overwrite the uncommited changes. Commit the changes and try again.\nAborting!" << endl;
        exit(0);
    }
    ifstream fileReader;
    fileReader.open((root+"/.imperium/commit.log" ).c_str(), fstream::in) ;
    string reader = "", lastHash = "", revertHash = "", headHash = "";
    while (getline (fileReader, reader)) {
        lastHash = reader.substr(7, 40);
        if(reader.substr(48, 4)=="HEAD") headHash = lastHash;
        if(revertHash.size() ==  40 && (revertHash == passedHash || revertHash.substr(0, 6) == passedHash)){
            break;
        }
        // cout << "." << endl;
        revertHash = lastHash;
    }
    fileReader.close();

    if(lastHash == ""){
        cout << "Error: No commits made previously." << endl;
        exit(0);
    }
    else if(!(revertHash.size() ==  40 && (revertHash == passedHash || revertHash.substr(0, 6) == passedHash))){
        cout << "Error: Commit Hash mismatch." << endl;
    }

    if(revertHash == lastHash){
        for(auto &subDir: fs::recursive_directory_iterator(root + "/.imperium/.commit/" + revertHash)){
            if(subDir.path() != root + "/.imperium/.commit/" + revertHash && doesExist(subDir.path())=="file"){
                string relPath = "/" + relativePath(subDir.path()).substr(58);
                cout << relPath << endl;
                if(isSame(root + relPath, subDir.path())){
                    cout << "Removing " + root + relPath << endl;
                    fs::remove_all(root + relPath);
                }
                else{
                    cout << "Found conflict in " + root + relPath << endl;
                    fstream fileWriter;
                    fileWriter.open(root + relPath, fstream::app);
                    fileWriter << "## Current Files\n";
                    fileWriter << "## This file was created in your reverted commit, passedHash , but your subsequent changes have been preserved.\n";
                    fileWriter.close();
                    fileWriter.open((root + "/.imperium/conflict").c_str(), fstream::out | fstream::trunc);
                    fileWriter << "true\n";
                    fileWriter.close();
                }
            }
        }
    }
    else if(revertHash == headHash){
        fs::copy((root + "/.imperium/.commit/" + lastHash).c_str(), (root).c_str(), fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        for(auto &subDir: fs::recursive_directory_iterator(root)){
            if(!isIgnored(subDir.path()) && subDir.path()!=root){
                string temPath = root + "/.imperium/.commit/" + lastHash + "/" + relativePath(subDir.path());
                if(doesExist(subDir.path())!=doesExist(temPath)){
                    cout << subDir.path() << endl;
                    fs::remove_all(subDir.path());
                }
            }
        }
    }
    else{
        for(auto &subDir: fs::recursive_directory_iterator(root + "/.imperium/.commit/" + lastHash)){
            if(subDir.path() != root + "/.imperium/.commit/" + lastHash){
                string relPath = "/" + relativePath(subDir.path()).substr(58);
                // cout
                if(doesExist(subDir.path())=="file" && !isSame(root + relPath, subDir.path())){
                    cout << "Found conflict in " + root + relPath << endl;
                    fstream fileReader, fileWriter;
                    fileWriter.open((root + relPath).c_str(), fstream::app);
                    fileReader.open(subDir.path().c_str(), fstream::in);
                    fileWriter << "## MERGE CONFLICT, CURRENT FILE ##\n";
                    string tempcontents;
                    while(getline(fileReader, tempcontents)){
                        fileWriter << tempcontents + "\n";
                    }
                    fileReader.close();
                    fileWriter.close();
                    fileWriter.open((root + "/.imperium/conflict").c_str(), fstream::out | fstream::trunc);
                    fileWriter << "true\n";
                    fileWriter.close();
                }
                else{
                    fs::copy((root + "/.imperium/.commit/" + lastHash).c_str(), (root + relPath).c_str(), fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                }
            }
        }
        for(auto &subDir: fs::recursive_directory_iterator(root + "/.imperium/.commit/" + revertHash)){
            if(subDir.path() == root + "/.imperium/.commit/" + revertHash){
                string relPath = "/" + relativePath(subDir.path()).substr(58);
                bool existsInLastHash = (doesExist(subDir.path())==doesExist(root + "/.imperium/.commit/" + lastHash + relPath)),
                     existsInHeadHash = (doesExist(subDir.path())==doesExist(root + "/.imperium/.commit/" + headHash + relPath));
                if(!existsInLastHash && existsInHeadHash && !isSame(subDir.path(), root + "/.imperium/.commit/" + headHash + relPath)){
                    cout << "Found conflict in " + root + relPath << endl;fstream fileWriter;
                    fileWriter.open(root + relPath, fstream::app);
                    fileWriter << "## Current Files ##\n";
                    fileWriter << "## This file was created in your reverted commit " +revertHash+ " , but your subsequent changes have been preserved. ##\n";
                    fileWriter.close();
                    fileWriter.open((root + "/.imperium/conflict").c_str(), fstream::out | fstream::trunc);
                    fileWriter << "true\n";
                    fileWriter.close();
                }
                else {
                    cout << "Removing " + root + relPath << endl;
                    fs::remove_all(root + relPath);
                }

            }
        }
    }

    fileReader.open((root+"/.imperium/commit.log" ).c_str(), fstream::in);
    fstream fileWriter;   
    fileWriter.open ((root+"/.imperium/temp.log").c_str(), fstream::in | fstream::app);
    string commitEntry;
    while (getline (fileReader, commitEntry)) {
        string Hash = commitEntry.substr(7, 40);
        if(Hash == revertHash){
            if(commitEntry.substr(48, 4) == "HEAD"){
                if(getline (fileReader, commitEntry)){
                fileWriter << commitEntry.substr(0, 47) + " HEAD " + commitEntry.substr(53) + "\n";
                };
            }

            continue;
        }
        fileWriter << commitEntry + "\n";
    }
    if(rename((root+"/.imperium/temp.log").c_str(), (root+"/.imperium/commit.log" ).c_str()) != 0){
        cerr << "Commit Failed! Error: " << strerror(errno) << endl;
        exit(0);
    }
    fileWriter.close();
    fs::remove_all(root + "/.imperium/.commit/" + revertHash);
    cout << "Commit " + revertHash + " reverted sucessfully." <<   endl;
    return ;
} 
