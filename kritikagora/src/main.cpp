#include<bits/stdc++.h>
#include<utility>
#include<algorithm>
#include<fstream>
#include<string>
#include<string.h>
#include<sys/stat.h>
#include<chrono>
#include<ctime>
#include<openssl/sha.h>
#include<unistd.h>
#include<experimental/filesystem>

namespace fs = std::experimental::filesystem;

std::string root = "";

void init(std::string path)
{
	struct stat buffer;
	if(stat((path+"/.imperium").c_str(), &buffer) == 0)
		std::cout<<"Repository has already initialized\n";
	
	else
	{
		std::string ignorepath = path + "/.imperiumignore";
		std::ofstream ignore(ignorepath.c_str());
		
		ignore<<"/.gitignore\n.imperium/\n.git/\n/.imperiumignore\nnode_modules/\n";
		ignore.close();

		path+="/.imperium";
		int created = mkdir(path.c_str(), 0777);
		
		if(created == 0)
		{
			std::string commitlog = path + "/commitlog";
			std::ofstream commit(commitlog.c_str());
			commit.close();

			std::string addlog = path + "/addlog";
			std::ofstream add(addlog.c_str());
			add.close();

			std::string conflictlog = path + "/conflicttlog";
			std::ofstream conflict(conflictlog.c_str());
			conflict.close();

			std::cout<<"Initialized an empty repository\n";
		}
		else
			std::cout<<"ERROR\n";
	}
}

bool ignoreFolder(std::string path, std::vector<std::string>dirname)
{
	for(auto dir : dirname)
	{
		dir.pop_back();
		if(path.find(dir) != std::string::npos)
			return true;
	}
	return false;
}

void addToCache(std::string path, char type)
{
	struct stat buffer;
	if(stat((root + "/.imperium/.add").c_str(), &buffer) != 0)
		mkdir((root + "/.imperium/.add").c_str(), 0777);
	
	if(type == 'f')
	{
		std::string filename = path.substr(root.length());
		std::string filerel = root + "/.imperium/.add" + filename.substr(0, filename.find_last_of("/"));
		
		struct stat buffer2;
		if(stat(filerel.c_str(), &buffer2) != 0)
			fs::create_directories(filerel);
		fs::copy_file(path, root + "/.imperium/.add" + filename, fs::copy_options::overwrite_existing);
	}
	else if(type == 'd')
	{
		std::string filename = path.substr(root.length());
		std::string filerel = root + "/.imperium/.add" + filename;
		
		struct stat buffer3;
		if(stat(filerel.c_str(), &buffer3) != 0)
			fs::create_directories(filerel);
	}
}

bool toBeIgnored(std::string path, int onlyImperiumIgnore = 0)
{
	std::ifstream addFile, ignoreFile;
	std::string file_or_dir;
	std::vector<std::string>filenames;
	std::vector<std::pair<std::string, char>>addedFileNames;
	std::vector<std::string>ignoreDir;
	
	ignoreFile.open(root + "/.imperiumignore");
	addFile.open(root + "/.imperium/addlog");
	
	if(ignoreFile.is_open())
	{
		while(!ignoreFile.eof())
		{
			std::getline(ignoreFile, file_or_dir);
			auto i = file_or_dir.end();
			i--;
			if(*i == '/')
				ignoreDir.push_back(file_or_dir);
			else
				filenames.push_back(root + file_or_dir);
		}
	}
	ignoreFile.close();
	
	if(onlyImperiumIgnore == 0)
	{
		if(addFile.is_open())
		{
			while(!addFile.eof())
			{
				std::getline(addFile, file_or_dir);
				if(file_or_dir.length() > 4)
					addedFileNames.push_back(std::make_pair(file_or_dir.substr(file_or_dir.length() - 4), file_or_dir.at(file_or_dir.length() - 1)));
				
			}
		}
		addFile.close();
		
		for(auto FileEntry : addedFileNames)
		{
			if(path.compare(FileEntry.first) == 0)
			{
				addToCache(path, FileEntry.second);
				std::cout<<"Updated: "<<path<<"\n";
				return true;
			}
		}
	}
	if(std::find(filenames.begin(), filenames.end(), path) != filenames.end() or ignoreFolder(path, ignoreDir))
		return true;
	return false;
}

void add(char **argv)
{
	struct stat buffer;
	if(stat((root + "/.imperium").c_str(), &buffer) == 0)
	{
		std::ofstream addFile;
		addFile.open(root + "/.imperium/addlog", std::ios_base::app);
		std::string path = root;
		if(strcmp(argv[2], ".") != 0)
		{
			path += "/";
			path += argv[2];
		}
		struct stat buffer2;
		if(stat(path.c_str(), &buffer2) == 0)
		{
			if(buffer2.st_mode && S_IFDIR)
			{
				if(!toBeIgnored(path))
				{
					addFile<<"\""<<path<<"\""<<"-d\n";
					addToCache(path, 'd');
					std::cout<<"Added Directory"<<"\""<<path<<"\""<<"\n";
				}
				for(auto &p : fs::recursive_directory_iterator(path))
				{
					if(toBeIgnored(p.path()))
						continue;
					struct stat buffer3;
					if(stat(p.path().c_str(), &buffer3) == 0)
					{
						if(buffer3.st_mode & S_IFDIR)
						{
							addToCache(p.path(), 'd');
							addFile<<p.path()<<"-d\n";
							std::cout<<"Added Directory"<<"\""<<p.path()<<"\""<<"\n";
						}
						else if(buffer3.st_mode & S_IFREG)
						{
							addToCache(p.path(), 'f');
							addFile<<p.path()<<"-f\n";
							std::cout<<"Added File: "<<"\""<<p.path()<<"\""<<"\n";
						}
						else
							continue;
					}
				}
			}
			else if(buffer2.st_mode & S_IFREG)
			{
				if(!toBeIgnored(path))
				{
					addFile<<"\""<<path<<"\""<<"-f\n";
					addToCache(path, 'f');
					std::cout<<"Added File"<<"\""<<path<<"\""<<"\n";
				}
				else
					std::cout<<"Invalid Path\n";
			}
		}
	}
	else
		std::cout<<"Repository has not been initialized yet\n";
}

std::string getTime()
{
	auto end  = std::chrono::system_clock::now();
	std::time_t end_time = std::chrono::system_clock::to_time_t(end);
	std::string time = std::ctime(&end_time);
	
	return time;
}

std::string getcommitHash()
{
	std::string commitFilename = getTime();
	std::string commitHash = "";
	
	char buffer[3];
	int length = commitFilename.length();
	unsigned char hash[20];
	unsigned char *val = new unsigned char[length + 1];
	strcpy((char *)val, commitFilename.c_str());
	
	SHA1(val, length, hash);
	for(int i = 0; i < 20; i++)
	{
		sprintf(buffer, "%02x", hash[i]);
		commitHash += buffer;
	}
	return commitHash;
}
	
void getCommitLog()
{
	std::string commitlogpath = root + "/.imperium/commitlog";
	std::string commitLine ;
	std::ifstream commitLog;
	
	commitLog.open(commitlogpath);
	while(std::getline(commitLog, commitLine))
		std::cout<<commitLine<<"\n";
}

void repeatCommit(std::string absPath, char type, std::string commitHash)
{
	mkdir((root + "/.imperium/.commit/" + commitHash).c_str(), 0777);
	
	std::string relpath = absPath.substr(root.length() + 19 + commitHash.length());
	std::string filepath = root + "/.imperium/.commit/" + commitHash + relpath.substr(0, relpath.find_last_of('/'));
	
	fs::create_directories(filepath);
	
	if(type == 'f')
		fs::copy_file(absPath, root + "/.imperium/.commit" + commitHash + relpath, fs::copy_options::overwrite_existing);
}

void addcommit(std::string absPath, char type, std::string commitHash)
{
	struct stat buffer;
	if(stat((root + "/.imperium/.commit").c_str(), &buffer) != 0)
		mkdir((root + "/.imperium/.commit").c_str(), 0777);
	if(stat((root + "/.imperium/.commit/" + commitHash).c_str(), &buffer) != 0)
		mkdir((root + "/.imperium/.commit/" + commitHash).c_str(), 0777);
	
	std::string relpath = absPath.substr(root.length() + 15);
	std::string filepath = root + "/.imperium/.commit/" + commitHash + relpath.substr(0, relpath.find_last_of('/'));
	
	fs::create_directories(filepath);
	
	if(type == 'f')
		fs::copy_file(absPath, root + "/.imperium/.commit/" + commitHash + relpath, fs::copy_options::overwrite_existing);
}

void updateCommitLog(std::string commitHash, std::string message)
{
	std::ofstream writeHeadLog;
	
	writeHeadLog.open(root + "/.imperium/headlog");
	writeHeadLog<<commitHash<<"--"<<message<<"\n";
	writeHeadLog.close();
	
	std::ofstream writeCommitLog;
	std::ifstream readCommitLog;
	
	readCommitLog.open(root + "/.imperium/commitlog");
	writeCommitLog.open(root + "/.imperium/new_commitlog");
	
	writeCommitLog<<commitHash<<"--"<<message<<"-->HEAD\n";
	
	std::string line;
	while(std::getline(readCommitLog, line));
	{
		if(line.find("-->HEAD") != std::string::npos)
			writeCommitLog<<line.substr(0, line.length() - 8)<<"\n";
		else
			writeCommitLog<<line<<"\n";
	}
	remove((root + "/.imperium/commitlog").c_str());
	rename((root + "/.imperium/new_commitlog").c_str(), (root + "/.imperium/commitlog").c_str());
	
	readCommitLog.close();
	writeCommitLog.close();
}

void commit(std::string message)
{
	struct stat buffer;
	if(stat((root + "/.imperium").c_str(), &buffer) != 0)
		std::cout<<"Repository has not been initialized\n";
	
	std::string commitHash = getcommitHash();
	
	if(stat((root + "/.imperium/headlog").c_str(), &buffer) == 0)
	{
		std::string headHash;
		std::ifstream readCommitLog;
		
		readCommitLog.open(root + "/.imperium/headlog");
		
		std::getline(readCommitLog, headHash);
		headHash = headHash.substr(0, headHash.find("--"));
		
		for(auto &p : fs::recursive_directory_iterator(root + "/.imperium/.commit/" + headHash))
		{
			if(stat(p.path().c_str(), &buffer) == 0)
			{
				if(buffer.st_mode & S_IFREG)
					repeatCommit(p.path(), 'f', commitHash);
				else if(buffer.st_mode & S_IFDIR)
					repeatCommit(p.path(), 'd', commitHash);
				else
					continue;
			}
		}
	}
	
	for(auto &p : fs::recursive_directory_iterator(root + "/.imperium/.add"))
	{
		struct stat buffer2;
		if(stat(p.path().c_str(), &buffer2) == 0)
		{
			if(buffer2.st_mode & S_IFREG)
				addcommit(p.path(), 'f', commitHash);
			else if(buffer2.st_mode & S_IFDIR)
				addcommit(p.path(), 'd', commitHash);
			else
				continue;
		}
	}
	fs::remove_all((root + "/.imperium/.add").c_str());
	remove((root + "/.imperium/addlog").c_str());
	updateCommitLog(commitHash, message);
}

int BUFFER_SIZE  = 8192;
int compareFiles(std::string path1, std::string path2)
{
	std::ifstream lFile(path1.c_str(), std::ifstream::in | std::ifstream::binary);
	std::ifstream rFile(path2.c_str(), std::ifstream::in | std::ifstream::binary);
	
	if(!lFile.is_open() || !rFile.is_open())
		return 1;
	
	char *lbuffer = new char[BUFFER_SIZE]();
	char *rbuffer = new char[BUFFER_SIZE]();
	
	do
	{
		lFile.read(lbuffer, BUFFER_SIZE);
		rFile.read(rbuffer, BUFFER_SIZE);
		int numberofRead = lFile.gcount();
		if(numberofRead != rFile.gcount())
			return 1;
		
		if(memcmp(lbuffer, rbuffer, numberofRead) != 0)
			return 1;
	}while(lFile.good() || rFile.good());
		
	delete[] lbuffer;
	delete[] rbuffer;
	return 0;
}

void status()
{
	struct stat buffer;
	std::ifstream readAddLog;
	std::string line, headHash;
	std::vector<std::string>staged;
	std::vector<std::string>type;
	std::vector<std::string>notStaged;
	std::ifstream readCommitLog;
	std::string stagedPath;
	
	readCommitLog.open(root + "/.imperium/commitlog");
	std::getline(readCommitLog, headHash);
	
	headHash = headHash.substr(0, 40);
	readCommitLog.close();
	
	if(stat((root + "/.imperium/.add").c_str(), &buffer) == 0)
	{
		readAddLog.open(root + "/.imperium/addlog");
		
		while(std::getline(readAddLog, line))
		{
			stagedPath = line.substr(root.length() + 1, line.length() - root.length() - 4);
			if(stat((root + "/.imperium/.add" + stagedPath).c_str(), &buffer) == 0)
			{
				if((stat((root + "/.imperium/.commit/" + headHash + stagedPath).c_str(), &buffer) != 0) || stat((root + "/.imperium/.commit").c_str(), &buffer) != 0)
				{
					type.push_back("created: ");
					staged.push_back(stagedPath);
					if(std::find(staged.begin(), staged.end(), stagedPath) == staged.end())
						notStaged.push_back("deleted: " + stagedPath);
					else
					{
						if(compareFiles(root + stagedPath, root + "/.imperium/.add" + stagedPath))
							notStaged.push_back("modified: " + stagedPath);
					}
				}
			}
		}
	}
	readAddLog.close();
	struct stat buffer2;
	if(stat((root + "/.imperium/.commit").c_str(), &buffer2) == 0)
	{
		for(auto &p : fs::recursive_directory_iterator(root))
		{
			if(stat(p.path().c_str(), &buffer2) == 0)
			{
				std::string rootPath = p.path();
				
				if(toBeIgnored(p.path().c_str(), 1))
					continue;
				
				std::string commitPath = root + "/.imperium/.commit/" + headHash + rootPath.substr(root.length());
				
				if(stat(commitPath.c_str(), &buffer2) != 0 and (std::find(staged.begin(), staged.end(), rootPath.substr(root.length())) == staged.end()))
				{
					for(int i = 0; i < staged.size(); i++)
						std::cout<<staged[i]<<"\n";
					
					notStaged.push_back("created: " + rootPath.substr(root.length()));
				}
				else
				{
					if(compareFiles(rootPath, commitPath))
					{
						if(std::find(staged.begin(), staged.end(), rootPath.substr(root.length())) == staged.end() and std::find(notStaged.begin(), notStaged.end(), stagedPath) == notStaged.end())
							notStaged.push_back("modified:" + root.substr(root.length())); 
					}
				}
			}
			else
			   continue;
		}
	}
	if(stat((root + "/.imperium/.commit").c_str(), &buffer2) != 0)
	{
		for(auto &p : fs::recursive_directory_iterator(root + "/.imperium/.commit" + headHash))
		{
			struct stat buffer3;
			if(stat(p.path().c_str(), &buffer3) == 0)
			{
				if(buffer3.st_mode & S_IFREG)
				{
					std::string commitPath = p.path();
					std::string rootPath = root + commitPath.substr(root.length() + 59);

					if(toBeIgnored(root.c_str(), 1))
						continue;

					if(stat(rootPath.c_str(), &buffer3) == 0)
						notStaged.push_back("deleted: " + rootPath.substr(root.length()));
				}
				else
					continue;
			}
		}
	}
	if(notStaged.size())
		std::cout<<"Changes not staged for commit: \n\n";
	for(int i = 0; i < notStaged.size(); i++)
		std::cout<<notStaged[i]<<"\n";
	
	if(staged.size())
		std::cout<<"\nChanges staged for commit:\n\n";
	
	for(int i = 0; i < staged.size(); i++)
		std::cout<<type[i]<<staged[i]<<"\n";
	
	if(!notStaged.size() and !staged.size())
		std::cout<<"Up to date\n";
	
	staged.clear();
	notStaged.clear();
}

void checkout(std::string commitHash)
{
	if(commitHash == "")
	{
		std::cout<<"Please provide a hash\n";
		return;
	}
	
	bool hashCheck = false;
	std::ifstream commitLog;
	std::string commitLine;
	
	commitLog.open(root + "/.imperium/commitlog");
	
	while(std::getline(commitLog, commitLine))
	{
		if(commitHash == commitLine.substr(0, 40))
		{
			hashCheck = true;
			break;
		}
	}
	commitLog.close();
	
	if(hashCheck == false)
	{
		std::cout<<"Incorrect hash provided\n";
		return;
	}
	std::string commitFolderPath = root + "/.imperium/.commit" + commitHash;
	fs::copy(commitFolderPath, root + "/", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
}

int main(int argc, char **argv)
{
	const std::string dir = getenv("dir");
	root = dir;
	
	if(strcmp(argv[1], "init") == 0)
		init(root);
	else if(strcmp(argv[1], "add") == 0)
		add(argv);
	else if(strcmp(argv[1], "commit") == 0)
		commit(argv[2]);
	else if(strcmp(argv[1], "log") == 0)
		getCommitLog();
	else if(strcmp(argv[1], "status") == 0)
		status();
	else if(strcmp(argv[1], "checkout") == 0)
		checkout(argv[2]);
	return 0;
}