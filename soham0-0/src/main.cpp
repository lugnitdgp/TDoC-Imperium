#include <iostream> 
#include <fstream>
#include <sys/stat.h> 
#include <sys/types.h>
#include <string>
#include <cstring>

class imperium {
    std::string root;

    /*
        Checks if the provided path exists or not, if it returns its type.
        @param pathArgument passed path 
    */
    std::string doesExist(std::string pathArgument);
    public:
    /*
        constructor
        Sets root to present working directory.
    */
    imperium();

    /*
        Sets the root for repo directory if provided.
        @param pathArgument path of the repo directory relative to present working directory.
    */
    void setRoot(std::string pathArgument);

    //  Initializes empty repository at root directory
    void init();

    //  Adds current state to the staging area
    void add(std::string pathArgument);

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
        int currentArgumentNumber = 0;
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

std::string imperium::doesExist(std::string pathArgument){
    struct stat sb;
    if(!stat(pathArgument.c_str(), &sb)){
        if(S_ISDIR(sb.st_mode)){
            return "directory";
        }
        else if(S_ISREG(sb.st_mode)){
            return "file";
        }
    }
    return "\0";
}

void imperium::setRoot(std::string pathArgument){
    root += "/" + pathArgument;
    if((mkdir((root).c_str(), 0777))==-1){
        std::cerr << "Error: " << strerror(errno) << std::endl;
    } 
}

void imperium::init(){
    if(doesExist(root+"/.imperium")!="\0"){
        std::cout<<"Repository has already been initialized" << std::endl;
        return ;
    }
    
    if((mkdir((root+"/.imperium").c_str(), 0777))==-1){
        std::cerr << "Error: " << strerror(errno) << std::endl;
    }  
    
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

void imperium::add(std::string pathArgument){
    if(pathArgument == "."){
        pathArgument = "";
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