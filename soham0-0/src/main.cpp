#include <iostream>
#include <fstream>
#include <sys/stat.h> 
#include <sys/types.h>
#include <filesystem>
#include <cstring>
#include <ctime>
#include <openssl/sha.h>

class imperium {
    std::string root;
    std::string doesExist(std::string);
    bool isRepo();
    std::string getTime();
    std::string relativePath(std::string);
    bool isIgnored(std::string);
    void addToLog(std::string);
    void addToCache(std::string);
    std::string getHash(std::string);
    void purgeAdd();
    void updateCommitLog(std::string, std::string);
    public:
    /*
        Constructor
        Sets root to present working directory.
    */
    imperium();

    void getHelp(std::string);

    /*
        Sets the root for repo directory if provided.
        @param path: path of the repo directory relative to present working directory.
    */
    void setRoot(std::string);

    //  Initializes empty repository at root directory
    void init();

    /*  
        Adds current state to the staging area.
        @param path: path of file to be added
    */
    void add(std::string);

    //  Commits the tracked changes
    void commit(std::string);

    //  Get Log of all commits
    void getCommitLog(std::string);

    //  Changes current branch
    void checkout(std::string);

    //  Reverts back to last commit
    void revert();

    //  Shows status of the tracked items
    void getStatus();
};

int main(int argc, char **argv){
    if(argc<2){
        std::cout << "Too few arguments!\n";
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
            std::cout << "Please add your commit message" << std::endl;
            return -1;
        }
        repository.commit(argv[2]);
    }
    else if(!strcmp(argv[1], "log")){
        if(argc==3 && !strcmp(argv[2], "--oneline"))    repository.getCommitLog(argv[2]);
        else if(argc == 2)                              repository.getCommitLog("");
        else {
            std::cout << "Flag not recognized!" << std::endl;
            repository.getHelp("log");
        }
    }
    else if(!strcmp(argv[1], "checkout")){
        if(argc<3 || (strlen(argv[2])!=6 && strlen(argv[2])!=40)){
            std::cout << "Please enter the commit hash in correct format." << std::endl;
            repository.getHelp("checkout");
            return -1;
        }
        repository.checkout(argv[2]);
    }
    else {
        std::cout << "Fatal Error: Command not recognized." << std::endl;
        repository.getHelp("");
    }
    return 0;
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
	std::cout << "Usage: imperium [--help <query>] [init] [add <arguments>] [commit \"<mesasge>\"]" << std::endl;
    }
    if(allHelp || helpQuery == "init"){
	    std::cout << "Start a working area: \n\tinit\tInitialize an Empty repositorty in current or specified folder.\n\tUsage:\timperium init <optional relative directory path>" << std::endl;
    }
    if(allHelp || helpQuery == "add"){
	std::cout << "Work on current change:\n\tadd\tAdd current state to index.\n\tUsage:\timperium add [.] [list of arguments]" << std::endl;
    }
    if(allHelp || helpQuery == "commit"){
	std::cout << "Grow, mark and tweak your common history\n\tcommit\tRecord changes to the repository.\n\tUsage:\timperium commit <message>" <<std::endl;
    }
    return ;
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

bool imperium::isRepo(){
    if(doesExist(root + "/.imperium")!="directory")             return false;
    if(doesExist(root + "/.imperium/conflict")!="file")         return false;
    if(doesExist(root + "/.imperium/commit.log")!="file")       return false;
    if(doesExist(root + "/.imperium/add.log")!="file")          return false;
    if(doesExist(root + "/.imperiumIgnore")!="file")            return false;
    return true;
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
    return false;
}

void imperium::addToLog(std::string path){
    std::fstream fileReaderWriter;
    fileReaderWriter.open((root+"/.imperium/add.log" ).c_str(),std::fstream::in);
    std::string loggedPath;
    while (std::getline (fileReaderWriter, loggedPath)) {
        if(loggedPath == "/" + path) return ;
    } 
    fileReaderWriter.close();
    fileReaderWriter.open((root+"/.imperium/add.log" ).c_str(),std::fstream::app);
    fileReaderWriter << "/" + path << std::endl;
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
    if(!isRepo()){
        std::cout << "Fatal Error: Not An Imperium Repository" << std::endl;
        exit(0);
    }
    if(path!=".") {
        purgeAdd();
        type = doesExist(root + "/" + path);
    }
    else {
        path = "";
        type = "directory";
    }
    if(type=="\0"){
        std::cout << path << "No such file/directory exits!" << std::endl;
        return ;
    }
    if(isIgnored(path)) return ; 
    if(type == "directory"){
        addToLog(root + "/"+ path);
        addToCache(root + "/" + path);
        for(auto &subDir : std::filesystem::recursive_directory_iterator(root +"/"+ path)){
            if(!isIgnored(subDir.path())){
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
    if(!isRepo()){
        std::cout << "Fatal Error: Not An Imperium Repository" << std::endl;
        exit(0);
    }
    if(doesExist(root + "/.imperium/.add") != "directory"){
        std::cout << "Nothing to commit. No staged files/folders." << std::endl;
        return ;
    }
    std::string commitHash = getHash(message), temp, previousCommit = "";
    std::fstream fileReader;
    fileReader.open((root+"/.imperium/commit.log" ).c_str(),std::fstream::in);
    while (std::getline(fileReader, temp)){
        previousCommit = temp;
    }
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
    if(!isRepo()){
        std::cout << "Fatal Error: Not An Imperium Repository" << std::endl;
        exit(0);
    }
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
    if(!isRepo()){
        std::cout << "Fatal Error: Not An Imperium Repository" << std::endl;
        exit(0);std::fstream fileReader;
    }
    
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

void imperium::revert(){
    //  to do
    return ;
}

void imperium::getStatus(){
    //  to do
    return ;
} 