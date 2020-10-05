#include <iostream>
#include <fstream>
#include <sys/stat.h> 
#include <sys/types.h>
#include <filesystem>
#include <cstring>

class imperium {
    std::string root;
    std::string doesExist(std::string);
    std::string relativePath(std::string);
    bool isIgnored(std::string);
    void addToLog(std::string);
    void addToCache(std::string);
    public:
    /*
        Constructor
        Sets root to present working directory.
    */
    imperium();

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
    void commit();

    //  Changes current branch
    void checkout();

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
    if(!strcmp(argv[1], "init")){
        if(argc==3) repository.setRoot(argv[2]);
        repository.init();
    }
    else if(!strcmp(argv[1], "add")){
        int currentArgumentNumber = 2;
        for(; currentArgumentNumber<argc; currentArgumentNumber++){
            repository.add(argv[currentArgumentNumber]);
        }
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
    if(path!=".") {
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

void imperium::commit(){
    //  to do
    return ;
}

void imperium::checkout(){
    //  to do
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