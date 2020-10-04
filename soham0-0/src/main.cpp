#include <iostream> 
#include <fstream>
#include <sys/stat.h> 
#include <sys/types.h>
#include <string>
#include <cstring>

class imperium {
    std::string path;
    public:
    /*
        constructor
        Sets path to present working directory.
    */
    imperium();

    /*
        Sets the path for repo directory if provided.
        @param pathQuery path of the repo directory relative to present working directory.
    */
    void setPath(std::string pathQuery);

    //  Initializes empty repository at path directory
    void init();

    //  Adds current state to the staging area
    void add();

    //  Commits the tracked changes
    void commit();

    //  Changes current branch
    void checkout();

    //  Reverts back to last commit
    void revert();

    //  Shows status of the 
    void getStatus();
};

int main(int argc, char **argv){
    if(argc<2){
        std::cout << "Too few arguments!\n";
        return -1;
    }

    imperium repository;

    if(!strcmp(argv[1], "init")){
        if(argc==3) repository.setPath(argv[2]);
        repository.init();
    }
    return 0;
}

imperium::imperium(){
    if(getenv("dir")){
        path = getenv("dir");
    }else{
        std::cout << "No environment variable found!" << std::endl;
        exit(0);
    }
}

void imperium::setPath(std::string pathQuery){
    path += "/" + pathQuery;
    if((mkdir((path).c_str(), 0777))==-1){
        std::cerr << "Error: " << strerror(errno) << std::endl;
    } 
}

void imperium::init(){
    struct stat sb;
    if(!stat((path+"/.imperium").c_str(), &sb) && S_ISDIR(sb.st_mode)){
        std::cout<<"Repository has already been initialized" << std::endl;
        return ;
    }
    
    if((mkdir((path+"/.imperium").c_str(), 0777))==-1){
        std::cerr << "Error: " << strerror(errno) << std::endl;
    }  
    
    std::fstream fileWriter;
    fileWriter.open ((path+"/.imperiumIgnore").c_str(), std::fstream::out | std::fstream::app);
    fileWriter << "/.imperium\n/.imperiumIgnore\n/.node_modules\n/.env\n";
    fileWriter.close(); 

    fileWriter.open ((path+"/.imperium/conflict").c_str(), std::fstream::out | std::fstream::app);
    fileWriter << "false\n";
    fileWriter.close(); 
    
    fileWriter.open ((path+"/.imperium/commit.log").c_str(), std::fstream::out | std::fstream::app);
    fileWriter << "";
    fileWriter.close(); 

    fileWriter.open ((path+"/.imperium/add.log").c_str(), std::fstream::out | std::fstream::app);
    fileWriter << "";
    fileWriter.close();
    
    std::cout << "Initialized empty imperium repository at " << path << std::endl;
    return ;
}

void imperium::add(){
    //  to do
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