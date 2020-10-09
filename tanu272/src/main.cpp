#include <cstdio>
#include <iostream>
#include <utility>
#include <time.h>
#include <chrono>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <bits/stdc++.h>
#include <sys/stat.h>
#include <openssl/sha.h>
#include <experimental/filesystem>
#define pb push_back

using namespace std;
namespace fs = experimental::filesystem;
namespace fsnew = fs;
string root = "";

/// Displaying help if no commands are given
void displayHelp() {
  cout << "usage: imperium <command> [<args>]\n\n";
  cout << "Start a working area\n";
  cout
      << "\tinit\t\tInitialise an empty repository inside the current folder\n";
  cout << "\tadd\t\tAdd files to staging area\n";
  cout << "\n";
}

/// check if directory exists
bool dirExists(string path) {
  // TODO: Add error checks
  struct stat statcheck;
  return stat(path.c_str(), &statcheck) == 0 && S_ISDIR(statcheck.st_mode);
}

/// create directory at path
void createDir(string path) {
  // TODO: Add error checks
  mkdir(path.c_str(), 0777);
}

/// custom function to write to files
void writeFile(string writePath, string fileContent) {
  ofstream customFileWriter;
  customFileWriter.open(writePath);
  customFileWriter << fileContent;
  customFileWriter.close();
}

/// imperium init function
void init(const char *path) {
  // TODO: Add error checks
  if (dirExists(path)) {
    cout << "Existing imperium repository\n";
  } else {
    cout << path << "\n";
    createDir(path);
    createDir((path) + string("/.add"));
    createDir((path) + string("/.commit"));
    writeFile((path) + string("/conflict"), "false\n");
    writeFile((path) + string("/commit.log"), "\n");
    writeFile((path) + string("/add.log"), "\n");

    cout << "Initialised empty imperium repository at " << path << "\n";
  }
}

void addToCache(string path, char type) {
  struct stat buffer;
  if (stat((root + "/.imperium/.add").c_str(), &buffer) != 0) {
    mkdir((root + "/.imperium/.add").c_str(), 0777);
  }
  if (type == 'f') {
    string filename = path.substr(root.length());
    string filerel = root + "/.imperium/.add" +
                          filename.substr(0, filename.find_last_of("/"));

    struct stat buffer2;
    if (stat(filerel.c_str(), &buffer2) != 0) {
      fs::create_directories(filerel);
    }
    fs::copy_file(path, root + "/.imperium/.add" + filename,
                  fs::copy_options::overwrite_existing);
  } else if (type == 'd') {
    string filename = path.substr(root.length());
    string filerel = root + "/.imperium/.add" + filename;

    struct stat buffer3;
    if (stat(filerel.c_str(), &buffer3) != 0) {
      fs::create_directories(filerel);
    }
  }
}

bool ignoreFolder(string path, vector<string> dirname) {
  for (auto dir : dirname) {
    dir.pop_back();

    if (path.find(dir) != string::npos) {
      return true;
    }
  }
  return false;
}

bool toBeIgnored(string path, int onlyImperiumIgnore = 0) {
  ifstream addFile, ignoreFile;
  string file_or_dir;
  vector<string> filenames;
  vector<pair<string, char>> addedFileNames;
  vector<string> ignoreDir;

  ignoreFile.open(root + "/.imperiumIgnore");
  addFile.open(root + "/.imperium/add.log");

  if (ignoreFile.is_open()) {

    while (!ignoreFile.eof()) {
      getline(ignoreFile, file_or_dir);
      auto i = file_or_dir.end();

      i--;
      if (*i == '/') {
        ignoreDir.push_back(file_or_dir);
      } else {
        filenames.push_back(root + "/" + file_or_dir);
      }
    }
  }
  ignoreFile.close();

  if (onlyImperiumIgnore == 0) {
    if (addFile.is_open()) {
      while (!addFile.eof()) {
        getline(addFile, file_or_dir);
        if (file_or_dir.length() > 4) {
          addedFileNames.push_back(
              make_pair(file_or_dir.substr(1, file_or_dir.length() - 4),
                             file_or_dir.at(file_or_dir.length() - 1)));
        }
      }
    }
    addFile.close();

    for (auto fileEntry : addedFileNames) {
      if (path.compare(fileEntry.first) == 0) {
        addToCache(path, fileEntry.second);
        cout << "Updated: " << path << endl;
        return true;
      }
    }
  }
  if ((find(filenames.begin(), filenames.end(), path)) !=
          filenames.end() ||
      ignoreFolder(path, ignoreDir)) {
    return true;
  }
  return false;
}

void add(char *argv) {
  struct stat buffer;
  if (stat((root + "/.imperium").c_str(), &buffer) == 0) {
    ofstream addFile;
    addFile.open(root + "/.imperium/add.log", ios_base::app);
    string path = root;
    if (strcmp(argv, ".") != 0) {
      path = path + "/" + argv;
    }
    struct stat buffer2;
    if (stat(path.c_str(), &buffer2) == 0) {
      if (buffer2.st_mode & S_IFDIR) {
        if (!toBeIgnored(path)) {
          addFile << "\"" << path << "\""
                  << "-d\n";
          addToCache(path, 'd');
          cout << "Added directory: "
                    << "\"" << path << "\""
                    << "\n";
        }
        for (auto &p : fs::recursive_directory_iterator(path)) {
          if (toBeIgnored(p.path()))
            continue;
          struct stat buffer3;
          if (stat(p.path().c_str(), &buffer3) == 0) {

            if (buffer3.st_mode & S_IFDIR) {
              addToCache(p.path(), 'd');
              addFile << p.path() << "-d\n";
              cout << "Added directory: " << p.path() << "\n";
            } else if (buffer3.st_mode & S_IFREG) {
              addToCache(p.path(), 'f');
              addFile << p.path() << "-f\n";
              cout << "Added file: " << p.path() << "\n";
            } else {
              continue;
            }
          }
        }
      } else if (buffer2.st_mode & S_IFREG) {
        if (!toBeIgnored(path)) {
          addToCache(path, 'f');
          addFile << "\"" << path << "\""
                  << "-f\n";
          cout << "Added file: "
                    << "\"" << path << "\""
                    << "\n";
        }
      } else {
        cout << "Invalid path!\n";
      }
    }
  } else {
    cout << "Repository has not been initialised yet.\n";
  }
}

string getTime() {
  auto end = chrono::system_clock::now();
  time_t endTime = chrono::system_clock::to_time_t(end);
  string time = ctime(&endTime);

  return time;
}

/// returns a commit hash
string getCommitHash() {
  string commitFileName = getTime();
  string commitHash;
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
  string commitLogPath = root + "/.imperium/commit.log";
  string commitLine;
  ifstream commitLog;
  commitLog.open(commitLogPath);
  while (getline(commitLog, commitLine)) {
    cout << commitLine << "\n";
  }
}

/// copy old log over to a new log and a head log
void updateCommitLog(string commitHash, string message) {
  ofstream writeHeadLog;
  writeHeadLog.open(root + "/.imperium/head.log");
  writeHeadLog << commitHash << " -- " << message << "\n";
  writeHeadLog.close();

  ofstream writeCommitLog;
  ifstream readCommitLog;
  readCommitLog.open(root + "/.imperium/commit.log");

  writeCommitLog.open(root + "/.imperium/new_commit.log");
  writeCommitLog << commitHash << " -- " << message << " -->HEAD\n";

  string line;
  while (getline(readCommitLog, line)) {
    if (line.find(" -->HEAD") != string::npos) {
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
void addCommit(string absPath, char type, string commitHash) {
  struct stat check;
  if (stat((root + string("/.imperium/.commit")).c_str(), &check) != 0) {
    mkdir((root + string("/.imperium/.commit")).c_str(), 0777);
  }
  if (stat((root + string("/.imperium/.commit/") + commitHash).c_str(),
           &check) != 0) {
    mkdir((root + string("/.imperium/.commit/") + commitHash).c_str(),
          0777);
  }

  string relpath = absPath.substr(root.length() + 15);
  string filepath = root + "/.imperium/.commit/" + commitHash +
                         relpath.substr(0, relpath.find_last_of("/"));
  fs::create_directories(filepath);

  if (type == 'f') {
    fs::copy_file(absPath, root + "/.imperium/.commit/" + commitHash + relpath,
                  fs::copy_options::overwrite_existing);
  }
}

/// function to repaat commit from previous
void repeatCommit(string absPath, char type, string commitHash) {
  mkdir((root + string("/.imperium/.commit/") + commitHash).c_str(), 0777);

  string relpath =
      absPath.substr(root.length() + 19 + commitHash.length());

  string filepath = root + "/.imperium/.commit/" + commitHash +
                         relpath.substr(0, relpath.find_last_of("/"));
  fs::create_directories(filepath);

  if (type == 'f') {
    fs::copy_file(absPath, root + "/.imperium/.commit/" + commitHash + relpath,
                  fs::copy_options::overwrite_existing);
  }
}

// function to commit
void commit(string message) {
  struct stat repocheck;
  if (stat((root + string("/.imperium")).c_str(), &repocheck) != 0) {
    // if no repo
    cout << "Repository hasn't been initialised\n";
  } else {
    string commitHash = getCommitHash();

    // copy all files from the previous commit

    if (stat((root + string("/.imperium/head.log")).c_str(), &repocheck) !=
        0) {
      string headHash;
      ifstream readCommitLog;
      readCommitLog.open((root + string("/.imperium/commit.log")).c_str());
      getline(readCommitLog, headHash);
      headHash = headHash.substr(0, headHash.find(" -- "));
      for (auto &prevCommit : fs::recursive_directory_iterator(
               root + string("/.imperium/.commit/") + headHash)) {
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
    for (auto &dir : fs::recursive_directory_iterator(
             root + string("/.imperium/.add"))) {
      struct stat filecheck;
      if (stat(dir.path().c_str(), &filecheck) == 0) {
        if (filecheck.st_mode & S_IFREG) {
          addCommit(dir.path(), 'f', commitHash);
        } else if (filecheck.st_mode & S_IFDIR) {
          addCommit(dir.path(), 'd', commitHash);
        }
      }
    }
    fs::remove_all((root + string("/.imperium/.add")).c_str());
    remove((root + string("/.imperium/add.log")).c_str());
    updateCommitLog(commitHash, message);
  }
}

void addCheckout(string path,char type){
	if(type=='d'){
		string parentPath = root + "/.imperium/.commit/";
		path = path.substr(parentPath.length());
		int index=-1;
		while(path[++index]!='/');
		path = path.substr(index+1);
		string destPath = root + "/" +path;
		struct stat buffer;
		if(stat(destPath.c_str(),&buffer) != 0){
			fsnew::create_directories(destPath);
		}
	}
	else if(type == 'f'){
		string parentPath = root + "/.imperium/.commit/";
		string relpath = path.substr(parentPath.length());
		int index=-1;
		while(relpath[++index]!='/');
		relpath = relpath.substr(index+1);
		string destDir = root + "/" + relpath.substr(0,relpath.find_last_of("/"));
		struct stat buffer2;
		if(stat(destDir.c_str(),&buffer2)!=0) fsnew::create_directories(destDir);
		destDir = root + "/" + relpath;
		fsnew::copy_file(path,destDir,fsnew::copy_options::overwrite_existing);
	}
}
void checkout(string commitHash){
	string commitPath = root + "/.imperium/commit.log";
	ifstream commitLog(commitPath.c_str());
	bool commitExists=0;
	if(commitLog.is_open()){
		while(!commitLog.eof()){
			string line; getline(commitLog,line);
			int index=-1;
			while(line[++index]!=' ');
			line=line.substr(0,index);
			if(line==commitHash){ commitExists=1; break;}
		}
	}
	else {cout<<"Could not open commit log\n"; return;}
	commitLog.close();
	if(commitExists){
		commitPath=root+"/.imperium/.commit/"+commitHash+"/";
		for(auto filePath:fsnew::recursive_directory_iterator(commitPath)){
			struct stat filebuffer;
			if(stat(filePath.path().c_str(),&filebuffer)==0){
				if(filebuffer.st_mode & S_IFDIR){
					addCheckout(filePath.path(),'d');
				}
				else if(filebuffer.st_mode & S_IFREG){
					addCheckout(filePath.path(),'f');
				}
			}
		}
		cout<<"Checkout successful\n";
	}
	else cout<<"No commits found\n";
}
bool comparefiles(string lFilePath,string rFilePath){
	int BUFFER_SIZE =1;
	std::ifstream lFile(lFilePath.c_str(), std::ifstream::in | std::ifstream::binary);
    std::ifstream rFile(rFilePath.c_str(), std::ifstream::in | std::ifstream::binary);

    if(!lFile.is_open() || !rFile.is_open())
    {
        return 1;
    }

    char *lBuffer = new char[BUFFER_SIZE]();
    char *rBuffer = new char[BUFFER_SIZE]();

    do {
        lFile.read(lBuffer, BUFFER_SIZE);
        rFile.read(rBuffer, BUFFER_SIZE);

        if (std::memcmp(lBuffer, rBuffer, BUFFER_SIZE) != 0)
        {
            delete[] lBuffer;
            delete[] rBuffer;
            return 1;
        }
    } while (lFile.good() || rFile.good());

    delete[] lBuffer;
    delete[] rBuffer;
    return 0;
}
string getHead(){
	string commitLogPath= root + "/.imperium/commit.log";
	ifstream commitLog;
	commitLog.open(commitLogPath.c_str());
	if(commitLog.is_open()){
		while(!commitLog.eof()){
			string commitHash;
			getline(commitLog,commitHash);
			int index=-1;
			while(commitHash[++index]!=' ');
			commitHash=commitHash.substr(0,index);
			commitLog.close();
			return commitHash;
		}
	}
	else {cout<<"Could not open commit log\n"; exit(0);}
	return string("ncf");
}

void status(){
	vector<string> staged[3]; vector<string> unstaged[3];
	string commitHash=getHead();
	string addLogPath = root + "/.imperium/add.log";
	ifstream addLog;
	addLog.open(addLogPath.c_str());
	if(addLog.is_open()){
		while(!addLog.eof()){
			string filePath;
			getline(addLog,filePath);
			if(filePath.length()<4) continue;
			filePath.erase(filePath.begin());
			filePath.erase(filePath.begin()+filePath.length()-3,filePath.end());
			filePath=filePath.substr(root.length());
			string commitPath = root + "/.imperium/.commit/";
				struct stat buffer;
				if(stat((commitPath+commitHash+filePath).c_str(),&buffer)==0){
					string path1=root + "/.imperium/.add"+filePath;
					string path2=commitPath+commitHash+filePath;
					string path3= root + filePath;
					if(comparefiles(path1,path2)){
						string pushin = root+filePath;
						staged[1].pb(pushin);
					}
					else {
						string pushin= root+filePath;
						staged[2].pb(pushin);
					}
					if(comparefiles(path1,path3)){
						string pushin=root+filePath;
						unstaged[1].pb(pushin);
					}
				}
				else{
					string pushin=root+filePath;
					staged[0].pb(pushin);
				}
		}
		string commitPath = root + "/.imperium/.commit/";
		struct stat commitBuffer; 
		if(stat(commitPath.c_str(),&commitBuffer)==0){
			for(auto p:fsnew::recursive_directory_iterator(root)){
				if(toBeIgnored(p.path(),1)) continue;
				string commitPath=p.path();
				if(commitPath.length()<=root.length()) continue;
				commitPath=commitPath.substr(root.length());
				string relpath=commitPath;
				commitPath=root+"/.imperium/.commit/"+commitHash+commitPath;
				struct stat commitPathBuffer;
				bool commitPathExists = (stat(commitPath.c_str(),&commitPathBuffer)==0);
				if(!commitPathExists&&find(staged[0].begin(),staged[0].end(),root+relpath)==staged[0].end())
				{
					unstaged[0].pb(root+relpath);
				}
				else{
					if(commitPathExists){
						if(comparefiles(root+relpath,commitHash)) unstaged[1].pb(root+relpath);
						else if(find(unstaged[1].begin(),unstaged[1].end(),root+relpath)==unstaged[1].end()) unstaged[2].pb(root+relpath);
					}
				}
			}
		}
	}
	else {cout<<"Could not open add log\n"; return;}
	addLog.close();
	cout<<"Staged : \n";
	cout<<"Created files : \n\n";
	if(staged[0].empty()) cout<<"Empty\n";
	for(string files:staged[0]) cout<<"---> "<<files<<endl;
	cout<<"\nModified Files : \n\n";
	if(staged[1].empty()) cout<<"Empty\n";
	for(string files:staged[1]) cout<<"---> "<<files<<endl;
	cout<<"\nUnchanged Files : \n\n";
	if(staged[2].empty()) cout<<"Empty\n";
	for(string files:staged[2]) cout<<"---> "<<files<<endl;
	cout<<"\nUnstaged : \n";
	cout<<"Created files : \n\n";
	if(unstaged[0].empty()) cout<<"Empty\n";
	for(string files:unstaged[0]) cout<<"---> "<<files<<endl;
	cout<<"\nModified Files : \n\n";
	if(unstaged[1].empty()) cout<<"Empty\n";
	for(string files:unstaged[1]) cout<<"---> "<<files<<endl;
	cout<<"\nUnchanged Files : \n\n";
	if(unstaged[2].empty()) cout<<"Empty\n";
	for(string files:unstaged[2]) cout<<"---> "<<files<<endl;
	cout<<endl;
}

int main(int argc, char **argv) {
  root = getenv("PWD");
  if (argc <= 1) {
    displayHelp();
  } else {
    // $ imperium <command> [<args>]
    /// if init is called
    if (strcmp(argv[1], "init") == 0) {
      if (argc == 2) {
        string ignorePath = string(getenv("PWD"));
        ignorePath = ignorePath + string("/.imperiumIgnore");
        string fileContent =
            "node_modules\n.env\n.imperium/\n.imperiumIgnore";
        writeFile(ignorePath, fileContent);
        const char *path = strcat(getenv("PWD"), "/.imperium");
        init(path);
      }
      if (argc == 3) {
        createDir(argv[2]);
        char *temp = strcat(getenv("PWD"), "/");
        temp = strcat(temp, argv[2]);
        // write .imperiumIgnore here
        string ignorePath = string(temp);
        ignorePath = ignorePath + string("/.imperiumIgnore");
        string fileContent =
            "node_modules\n.env\n.imperium/\n.imperiumIgnore";
        writeFile(ignorePath, fileContent);
        // written .imperiumIgnore here
        temp = strcat(temp, "/.imperium");
        const char *path = temp;
        init(path);
      }
    } else if (strcmp(argv[1], "add") == 0) {
      if (strcmp(argv[2], ".") != 0) {
        for (int i = 2; i < argc; i++) {
          add(argv[i]);
        }
      } else {
        add(argv[2]);
      }
    } else if (strcmp(argv[1], "commit") == 0) {
      commit(argv[2]);
    } else if (strcmp(argv[1], "log") == 0) {
      getCommitLog();
    }
	else if(strcmp(argv[1], "checkout") == 0) {
	  if(argc<3) {cout<<"Please enter a commit hash\n"; return 0;}
      checkout(argv[2]);
    }
	else if(strcmp(argv[1],"status")==0){
		status();
	}
  }
  return 0;
}
