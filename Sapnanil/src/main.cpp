#include <fstream>
#include <vector>
#include <iostream>
#include <cstring>
#include <utility>
#include <sys/types.h>
#include <sys/stat.h>
#include <experimental/filesystem>
#include <openssl/sha.h>
#include <filesystem>

using namespace std;
using namespace std::experimental::filesystem;

string root = "";
void init(std::string path)
{
	struct stat buffer;
	if(stat((path+"/.imperium").c_str(),&buffer)==0){
	   std::cout<<"Repository has already been initialized\n";}
	   else
	   { 
	
	std::string ignorepath = path + "/.imperiumignore";
	std::ofstream ignore(ignorepath.c_str());
		   ignore<< ".gitignore\n.imperium\n.git\n.imperiumignore\n.node_modules\n";
		   ignore.close();
	path+="/.imperium";
	int created = mkdir(path.c_str(),0777);
	
	if(created==0)
		{std::string commitlog = path +"/commitlog";
		std::ofstream commit(commitlog.c_str());
	 commit.close();
		
		 std::string addlog = path +"/addlog";
		std::ofstream add(addlog.c_str());
	 add.close();
		  std::string conflictlog = path +"/conflict";
		std::ofstream conflict(conflictlog.c_str());
		 conflict<<"False\n";
		 conflict.close();
		 std::cout<<"Initialized an empty repository\n";
     }
	else
	{
		std::cout<<"ERROR\n";
	}
}
}

//add function

void addtoCache(const path &currentPath)
{
    int len = root.length();

    string relativePathString = currentPath.string().substr(len);
    
    path relativePath(relativePathString);

    path rootPath(root);
    path rootPathInUserDirectory(root);

    rootPath /= ".imperium";
    rootPath /= ".add";

    for (const auto &cur : relativePath)
    {
        if (cur == "/")
            continue;
        rootPath /= cur;
        rootPathInUserDirectory /= cur;

        
        if (is_directory(rootPathInUserDirectory))
        {
            
            if (!exists(rootPath))
            {
                create_directory(rootPath.string().c_str());
            }
        }
        else
        {
            
            const auto copyOptions = copy_options::update_existing;
            if (!exists(rootPath))
            {
                cout << currentPath << " Initialized.\n";
            }
            else
            {
                cout << currentPath << " Updated.\n";
            }
            copy_file(currentPath.string(), rootPath.string(), copyOptions);
        }
    }

    
}

bool searchString(ifstream &inputFile, const string &str)
{

   
    string line;
    while (getline(inputFile, line))
    {
        if (line.find(str) != string::npos)
            return true;
    }
    return false;
}

void toBeAdded(const path &currentPath)
{

    string addLogPath = root + "/.imperium/addlog";
    ifstream addLogFile(addLogPath.c_str());

    if (!searchString(addLogFile, currentPath.u8string()))
    {
        
        ofstream fout(addLogPath.c_str(), ios::app);
        fout << currentPath << "\n";
        fout.close();
    }
    else
    {
        
    }
    addLogFile.close();
}

bool toBeIgnored(const path &currentPath)
{

    string ignorePath = root + "/.imperiumIgnore";

    string currentPathString = currentPath.string();
    string line;
    ifstream inputFile(ignorePath);

    while (getline(inputFile, line))
    {
        string Directory = root + line;
        if (line.back() == '/')
        {
            Directory.pop_back();
            if (currentPathString.find(Directory) != string::npos)
            {
                
                return true;
            }
        }
        else
        {
            
            if (currentPathString == Directory)
                return true;
        }
    }
    inputFile.close();
    return false;
}
void addUtil(const path &currentPath)
{

    string imperiumFolder = root + "/.imperium";
    string addFolder = imperiumFolder + "/.add";

    if (!toBeIgnored(currentPath))
    {
        toBeAdded(currentPath);

        
        mkdir(addFolder.c_str(), 0777);
        addtoCache(currentPath);
    }
    
}

void add(vector<string> &argv)
{

    string imperiumFolder = root + "/.imperium";
    string addFolder = imperiumFolder + "/.add";
    string addLogPath = imperiumFolder + "/addlog";
    string ignorePath = imperiumFolder + "/ignorelog";

    if (!exists(addLogPath))
    {
        cout << "Please initialize the repository first.\n";
        return;
    }

    for (auto &str : argv)
    {

        if (str == ".")
        {
            str = "";
        }
        path currentPath(root);
        currentPath /= str;
        if (!exists(currentPath))
        {
            cout << "Sorry the file: " << currentPath << " dosen't exist.\n";
            continue;
        }

        if (is_regular_file(currentPath))
        {
            addUtil(currentPath);
            continue;
        }
        
        for (auto &p : recursive_directory_iterator(currentPath))
        {
           
            addUtil(p.path());
        }
    }
}
void Addit(){
    std::filesystem::remove_all(root + "/.imperium/.add");
    std::fstream fileWriter;   
    fileWriter.open ((root+"/.imperium/add.log").c_str(), std::fstream::out | std::fstream::trunc);
    fileWriter.close();
}

std::string getTime() {
  auto end = std::chrono::system_clock::now();
  std::time_t endTime = std::chrono::system_clock::to_time_t(end);
  std::string time = std::ctime(&endTime);

  return time;
}

/// returns a commit hash
std::string getCommitHash() {
  std::string commitFileName = getTime();
  std::string commitHash;
  char buf[3];
  int length = commitFileName.length();
  unsigned char hash[20];
  unsigned char *val = new unsigned char[length + 1];
  strcpy((char *)val, commitFileName.c_str());
  SHA1(val, length, hash);
  for (int i = 0; i < 20; i++) {
    sprintf(buf, "%02x", hash[i]);
    commitHash += buf;
  }
  // returns hash
  return commitHash;
}

void getCommitLog() {
  std::string commitLogPath = root + "/.imperium/commit.log";
  std::string commitLine;
  std::ifstream commitLog;
  commitLog.open(commitLogPath);
  while (std::getline(commitLog, commitLine)) {
    std::cout << commitLine << "\n";
  }
}

/// copy old log over to a new log and a head log
void updateCommitLog(std::string commitHash, std::string message) {
  std::ofstream writeHeadLog;
  writeHeadLog.open(root + "/.imperium/head.log");
  writeHeadLog << commitHash << " -- " << message << "\n";
  writeHeadLog.close();

  std::ofstream writeCommitLog;
  std::ifstream readCommitLog;
  readCommitLog.open(root + "/.imperium/commit.log");

  writeCommitLog.open(root + "/.imperium/new_commit.log");
  writeCommitLog << commitHash << " -- " << message << " -->HEAD\n";

  std::string line;
  while (std::getline(readCommitLog, line)) {
    if (line.find(" -->HEAD") != std::string::npos) {
      writeCommitLog << line.substr(0, line.length() - 8) << "\n";
    } else {
      writeCommitLog << line << "\n";
    }
  }
  remove((root + "/.imperium/commit.log").c_str());

  rename((root + "/.imperium/new_commit.log").c_str(),
         (root + "/.imperium/commit.log").c_str());

  readCommitLog.close();
  writeCommitLog.close();
}

/// function to add commit
void addCommit(std::string absPath, char type, std::string commitHash) {
  struct stat check;
  if (stat((root + std::string("/.imperium/.commit")).c_str(), &check) != 0) {
    mkdir((root + std::string("/.imperium/.commit")).c_str(), 0777);
  }
  if (stat((root + std::string("/.imperium/.commit/") + commitHash).c_str(),
           &check) != 0) {
    mkdir((root + std::string("/.imperium/.commit/") + commitHash).c_str(),
          0777);
  }

  std::string relpath = absPath.substr(root.length() + 15);
  std::string filepath = root + "/.imperium/.commit/" + commitHash +
                         relpath.substr(0, relpath.find_last_of("/"));
  filesystem::create_directories(filepath);

  if (type == 'f') {
    filesystem::copy_file(absPath, root + "/.imperium/.commit/" + commitHash + relpath,
                  filesystem::copy_options::overwrite_existing);
  }
}

/// function to repaat commit from previous
void repeatCommit(std::string absPath, char type, std::string commitHash) {
  mkdir((root + std::string("/.imperium/.commit/") + commitHash).c_str(), 0777);

  std::string relpath =
      absPath.substr(root.length() + 19 + commitHash.length());

  std::string filepath = root + "/.imperium/.commit/" + commitHash +
                         relpath.substr(0, relpath.find_last_of("/"));
  filesystem::create_directories(filepath);

  if (type == 'f') {
    filesystem::copy_file(absPath, root + "/.imperium/.commit/" + commitHash + relpath,
                  filesystem::copy_options::overwrite_existing);
  }
}

/// function to commit
void commit(std::string message) {
  struct stat repocheck;
  if (stat((root + std::string("/.imperium")).c_str(), &repocheck) != 0) {
    // if no repo
    std::cout << "Repository hasn't been initialised\n";
  } else {
    std::string commitHash = getCommitHash();

    // copy all files from the previous commit

    if (stat((root + std::string("/.imperium/head.log")).c_str(), &repocheck) !=
        0) {
      std::string headHash;
      std::ifstream readCommitLog;
      readCommitLog.open((root + std::string("/.imperium/commit.log")).c_str());
      std::getline(readCommitLog, headHash);
      headHash = headHash.substr(0, headHash.find(" -- "));
      for (auto &prevCommit : filesystem::recursive_directory_iterator(
               root + std::string("/.imperium/.commit/") + headHash)) {
        struct stat filecheck;
        if (stat(prevCommit.path().c_str(), &filecheck) == 0) {
          if (filecheck.st_mode & S_IFREG) {
            repeatCommit(prevCommit.path(), 'f', commitHash);
          } else if (filecheck.st_mode & S_IFDIR) {
            repeatCommit(prevCommit.path(), 'd', commitHash);
          }
        }
      }
    }

    // copy all files from staging area for the first time
    for (auto &dir : filesystem::recursive_directory_iterator(
             root + std::string("/.imperium/.add"))) {
      struct stat filecheck;
      if (stat(dir.path().c_str(), &filecheck) == 0) {
        if (filecheck.st_mode & S_IFREG) {
          addCommit(dir.path(), 'f', commitHash);
        } else if (filecheck.st_mode & S_IFDIR) {
          addCommit(dir.path(), 'd', commitHash);
        }
      }
    }
    filesystem::remove_all((root + std::string("/.imperium/.add")).c_str());
    remove((root + std::string("/.imperium/add.log")).c_str());
    updateCommitLog(commitHash, message);
  }
}



int main(int argc, char *argv[])
{
    root = getenv("dir");
 if (argc == 1)
        std::cout << "Welcome to Imperium";
    else if (!strcmp(argv[1], "init"))
    {
        init(root);
    }
	 else if (!strcmp(argv[1], "commit"))
    {
        commit(argv[2]);
    }
	else if (strcmp(argv[1], "log") == 0) {
      getCommitLog();}
    else if (!strcmp(argv[1], "add"))
    {
        
        vector<string> Args;
        for (int i = 2; i < argc; i++)
        {
            Args.push_back(argv[i]);
        }
        add(Args);
    }
	 else if (strcmp(argv[1], "--help")==0)
            std::cout<<"Help is always provided just look for it.May the source be with you.";
    return 0;
}
