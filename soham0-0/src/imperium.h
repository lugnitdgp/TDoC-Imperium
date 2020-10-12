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

    /*
        Prints help for the required section, if none specified, prints all help.
        @param helpQuery: Required help section name
    */
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

    /*
        Commits the tracked changes
        @param message: Entered commit messagee
    */
    void commit(std::string);

    /*
        Get Log of all commits
        @param flag: optional flag (--oneline), if passed, shortens the output.
    */
    void getCommitLog(std::string);

    /*  
        Changes the current contents of the repo to that of a previous commit
        @param hash: commit hash of the desired commit 
    */
    void checkout(std::string);

    /*
        Reverts the changes made in the specified commit
        @param hash: commit hash of the desired commit
    */
    void revert(std::string);

    //  Shows status of files in the repository
    void getStatus();

    //  Unfreezes the commands
    void resolve();

    /*
        Finds the appropriate hash using the passed argument
        @param hashQuery: argument entered
    */
    std::string headHunter(std::string);

    //  checks if the repository is frozen due to conflict
    bool frozen();

    //  Encrypts the imperium files
    void encrypt();

    //  Decrypts the imperium files
    void decrypt();
};

#endif