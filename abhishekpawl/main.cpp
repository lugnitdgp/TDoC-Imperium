#include <utility>
#include <fstream>
#include <string>
#include <string.h>
#include <sys/stat.h>
#include <iostream>
#include <experimental/filesystem>
#include <bits/stdc++.h>
using namespace std;

namespace fs = experimental::filesystem;

std::string root = "";

void init(string path) {
	struct stat buffer;
	
	if(stat((path + "/.imperium").c_str(), &buffer) == 0) {
		std::cout<<"Repository has already been initialized\n";
	} else {
		std::string ignorepath = path + "/.imperiumignore";
		std::ofstream ignore(ignorepath.c_str());
		ignore<<".gitignore\n.imperium\n.git\n.imperiumignore\n.node_modules\n";
		ignore.close();

		path += "/.imperium";
		int created = mkdir(path.c_str(), 0777);

		if(created == 0) {
			std::string commitlog = path + "/commitlog";
			std::ofstream commit(commitlog.c_str());
			commit.close();

			std::string addlog = path + "/addlog";
			std::ofstream add(addlog.c_str());
			add.close();

			std::string conflictlog = path + "/conflictlog";
			std::ofstream conflict(conflictlog.c_str());
			conflict<<"False\n";
			conflict.close();

			std::cout<<"Initialized an empty repository\n";
		} else {
			std::cout<<"ERROR\n";
		}
	}
}

bool isFile(std::string path) {
	struct stat info;
	
	if(info.st_mode & S_IFREG) {
		return true;
	}
	return false;
}

bool ignoreFolder(string path, vector <string> dirname) {
	for(auto dir: dirname) {
		dir.pop_back();
		if(dir.find(path) != string::npos) {
			return true;
		}
	}
	return false;
}

void addToCache(string path, char type) {
	struct stat buffer;
	if(stat((root + "/.imperium/.add").c_str(), &buffer) != 0) {
		mkdir((root + "/.imperium/.add").c_str(), 0777);
	}
	
	if(type == 'f') {
		string filename = path.substr(root.length());
		string filerel = root + "/.imperium/.add" + filename.substr(0, filename.find_last_of("/"));
		
		struct stat buffer2;
		if(stat(filerel.c_str(), &buffer2) != 0) {
			fs::create_directories(filerel);
		}
		fs::copy_file(path, root + "/.imperium/.add" + filename, fs::copy_options::overwrite_existing);
	} else if(type == 'd') {
		string filename = path.substr(root.length());
		string filerel = root + "/.imperium/.add" + filename;
		
		struct stat buffer3;
		if(stat(filerel.c_str(), &buffer3) != 0) {
			fs::create_directories(filerel);
		}
	}
}

bool toBeIgnored(string path, int onlyImperiumIgnore = 0) {
	ifstream addFile, ignoreFile;
	string file_or_dir;
	vector <string> filenames;
	vector <pair <string, char>> addedFileNames;
	vector <string> ignoreDir;
	
	ignoreFile.open((root + "/.imperiumignore").c_str());
	addFile.open((root + "/.imprerium/add.log").c_str());
	
	if(ignoreFile.is_open()) {
		while(!ignoreFile.eof()) {
			getline(ignoreFile, file_or_dir);
			auto i = file_or_dir.end();
			
			i--;
			if(*i == '/') {
				ignoreDir.push_back(file_or_dir);
			} else {
				filenames.push_back(root + file_or_dir);
			}
		}
	}
	ignoreFile.close();
	
	if(onlyImperiumIgnore == 0) {
		if(addFile.is_open()) {
			while(!addFile.eof()) {
				getline(addFile, file_or_dir);
				if(file_or_dir.length() > 4) {
					addedFileNames.push_back(make_pair(file_or_dir.substr(file_or_dir.length() - 4), file_or_dir.at(file_or_dir.length()-1)));
				}
			}
		}
		addFile.close();
		
		for(auto fileEntry : addedFileNames) {
			if(path.compare(fileEntry.first) == 0) {
				addToCache(path, fileEntry.second);
				cout<<"Updated: "<<path<<endl;
				return true;
			}
		}
	}
	if(find(filenames.begin(), filenames.end(), path) != filenames.end() or ignoreFolder(path, ignoreDir)) {
		return true;
	}
	return false;
}

void add(char **argv) {
	struct stat buffer;
	if(stat((root + "/.imperium").c_str(), &buffer) == 0) {
		ofstream addFile;
		addFile.open((root + "/.addFile/add.log").c_str(), ios_base::app);
		string path = root;
		if(strcmp(argv[2], ".") != 0) {
			path += "/";
			path += argv[2];
		}
		struct stat buffer2;
		if(stat(path.c_str(), &buffer2) == 0) {
			if(buffer2.st_mode & S_IFDIR) {
				if(toBeIgnored(path)) {
					addFile<<"\""<<path<<"\""<<"-d\n";
					addToCache(path, 'd');
					cout<<"Added Directory"<<"\""<<path<<"\""<<"\n";
				}
				for(auto &p : fs::recursive_directory_iterator(path.c_str())) {
					if(toBeIgnored(p.path())) {
						continue;
					}
					struct stat buffer3;
					if(stat(p.path().c_str(), &buffer3) == 0) {
						if(buffer3.st_mode & S_IFDIR) {
							addToCache(p.path(), 'd');
							addFile<<p.path()<<"-d\n";
							cout<<"Added Directory: "<<"\""<<p.path()<<"\""<<"\n";
						} else if(buffer3.st_mode & S_IFREG) {
							addToCache(p.path(), 'f');
							addFile<<p.path()<<".f\n";
							cout<<"Added File: "<<"\""<<p.path()<<"\""<<"\n";
						} else {
							continue;
						}
					}
				}
			} else if(buffer2.st_mode & S_IFREG) {
				if(toBeIgnored(path)) {
					addFile<<"\""<<path<<"\""<<"-f\n";
					addToCache(path, 'f');
					cout<<"Added File: "<<"\""<<path<<"\""<<"\n";
				}
			} else {
				cout<<"Invalid path\n";
			}
		}
	} else {
		cout<<"Repository has not been initialized yet"<<"\n";
	}
}

int main(int argc, char **argv) {
	const std::string dir = getenv("dir");
	root = dir;
		
	if(strcmp(argv[1], "init") == 0) {
		init(root);
	} else if(strcmp(argv[1], "add") == 0) {
		add(argv);
    }
	
	return 0;
}