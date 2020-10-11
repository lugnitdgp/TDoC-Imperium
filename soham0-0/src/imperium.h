#ifndef IMPERIUM_H
#define IMPERIUM_H
#include <string>

class imperium {
    std::string root;
    std::string doesExist(std::string);
    void isRepo();
    std::string getTime();
    std::string relativePath(std::string);
    bool isIgnored(std::string);
    void addToLog(std::string);
    void addToCache(std::string);
    std::string getHash(std::string);
    void purgeAdd();
    void updateCommitLog(std::string, std::string);
    bool isSame(std::string, std::string);
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

    //  Reverts back the passed commit
    void revert(std::string);

    //  Shows status of the tracked items
    void getStatus();

    //  unfreezes the commands
    void resolve();

    std::string headHunter(std::string);

    bool frozen();
    void encrypt();
    void decrypt();
};

#endif