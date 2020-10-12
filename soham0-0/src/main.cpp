#include <iostream>
#include <cstring>
#include "imperium.h"


int main(int argc, char **argv){
    if(argc<2){
        std::cout << "Too few arguments!\n";
        return -1;
    }

    imperium repository;
    repository.decrypt();
    if(repository.frozen()){
        if(strcmp(argv[1],"resolve"))   std::cout << "The commands have been frozen please fix the merge conflicts and run $imperium resolve to unfrezze" << std::endl;
        else repository.resolve();
        exit(0);
    }
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
        std::string hash = repository.headHunter(argv[2]);
        if(argc<3 || (!hash.size())){
            std::cout << "Please enter the commit hash in correct format." << std::endl;
            repository.getHelp("checkout");
            return -1;
        }
        repository.checkout(hash);
    }
    else if(!strcmp(argv[1], "status")){
        repository.getStatus();
    }
    else if(!strcmp(argv[1], "revert")){
        std::string hash = repository.headHunter(argv[2]);
        if(argc<3 || (!hash.size())){
            std::cout << "Please enter the commit hash in correct format." << std::endl;
            repository.getHelp("revert");
            return -1;
        }
        repository.revert(hash);
    }
    else {
        std::cout << "Fatal Error: Command not recognized." << std::endl;
        repository.getHelp("");
    }
    repository.encrypt();
    return 0;
}