#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <fstream>
#include <filesystem>
#include <chrono> 
#include <ctime>
#include <openssl/sha.h>

using namespace std;
namespace fs = std::filesystem;

string root = "";

void init(const char* p)
{
	char path[PATH_MAX];
	strcpy(path, p);
	strcat(path,"/.imperium/");
	struct stat buf;
	if(stat(path, &buf) == 0  && S_ISDIR(buf.st_mode))
	{
		cout<<"Reinitialized existing Imperium repository in "<<path<<"\n";
		return;
	}
	else
	{
		mkdir(path,0777);
		ofstream fout;
		fout.open(root+"/.imperiumignore");
		fout<<"/.env\n";
		fout<<"/.imperium\n";
		fout<<"/goorm.manifest\n";
		fout.close();
		chdir(path);
		fout.open("conflict");
		fout<<"false\n";
		fout.close();
		fout.open("add.log");
		fout.close();
		fout.open("commit.log");
		fout.close();
		cout<<"Initialized empty Imperium repository in "<<path<<"\n";
		return;
	}
}

//Not implemented yet as it has some errors. Will upload again after rectifying.
/*string findImmediateNext(string path) //Function to find the next immediate folder with an imperium repo initialised from anywhere in the filesystem.
{
	while(true)
	{
		int t = path.find_last_of("/");
		path = path.substr(0,t);
		chdir(path.c_str());
		string p = path + "/.imperium/";
		struct stat buf;
		if((stat(p.c_str(), &buf)==0 && S_ISDIR(buf.st_mode)) || t==0)
		break;
	}
	return path;
}*/

void addToCache(string path)
{
	string s = root + "/.imperium/.add";
	struct stat buf;
	if (stat(s.c_str(), &buf) != 0 || S_ISDIR(buf.st_mode) == 0)
		mkdir(s.c_str(), 0777);
	struct stat buffer;
	if(stat(path.c_str(), &buffer) == 0)
	{
		string relative = path.substr(root.length());
		string imperiumrel = root + "/.imperium/.add" + relative;
		if(S_ISDIR(buffer.st_mode))
		{
			struct stat b;
			if (stat(imperiumrel.c_str(), &b) != 0 || S_ISDIR(b.st_mode) == 0)
				fs::create_directories(imperiumrel);
		}
		else
		{
			fs::copy_file(path, imperiumrel, fs::copy_options::overwrite_existing);
		}
	}
}

bool ignoreCheck(string path)
{
	ifstream fin;
	fin.open(root + "/.imperiumignore");
	string s;
	while (getline(fin, s))
	{
		int found = path.find(s); 
		if(found!=string::npos){
			cout << path << " exists in .imperiumignore\n";
			return true;
		}
	}
	fin.close();
	return false;
}

bool toBeIgnored(string path)
{
	ifstream fin;
	fin.open(root + "/.imperium/add.log");
	string s;
	while (getline(fin, s))
	{
		if(s == path){
			addToCache(path);
			cout << path << " exists in add.log\n";
			return true;
		}
	}
	if(ignoreCheck(path))
		return true;
	fin.close();
	return false;
}

void add(int argc, char* argv[])
{
	struct stat b;
	if(stat((root + "/.imperium/").c_str(), &b) != 0 || S_ISDIR(b.st_mode) == 0){
		cout<<"Fatal: not an imperium repository\n";
		return;
	}
	if(argc < 3){
		cout<<"Nothing specified, nothing added.\nMaybe you wanted to say 'imperium add .'?\n";
		return;
	}
	string path;
	if (strcmp(argv[2], ".") == 0)
		path = root;
	else
		path = root + "/" + argv[2];
	struct stat buf;
	if (stat(path.c_str(), &buf) == 0)
	{
		if(toBeIgnored(path))
			return;
		ofstream fout;
		fout.open(root + "/.imperium/add.log", ios::app);
		if(S_ISDIR(buf.st_mode))
		{
			for (auto& p: fs::recursive_directory_iterator(path))
			{
				string s = p.path();
				if(toBeIgnored(s))
					continue;
				fout << s << "\n";
				addToCache(s);
			}
		}
		else
		{
			fout << path << "\n";
			addToCache(path);
		}
		fout.close();
	}
	else
		cout << "Fatal: '" << argv[2] << "' did not match any files\n";
}

void updateCommitLog(string hash, string message, string time)
{
	ofstream fout;
	fout.open(root+"/.imperium/temp.log", ios::app);
	fout<<"commit "<<hash<<" // "<<time<<" "<<message<<" --> HEAD"<<"\n";
	ifstream fin;
	fin.open(root + "/.imperium/commit.log");
	string s;
	while (getline(fin, s))
	{
		int found = s.find("-->"); 
		if(found!=string::npos)
			s = s.substr(0, found-1);
		fout << s << "\n";
	}
	fin.close();
	fout.close();
	remove((root + "/.imperium/commit.log").c_str());
	rename((root+"/.imperium/temp.log").c_str(), (root+"/.imperium/commit.log").c_str());
}

void commit(int argc, char* argv[])
{
	string message;
	struct stat buf;
	if(stat((root + "/.imperium/").c_str(), &buf) != 0 || S_ISDIR(buf.st_mode) == 0){
		cout<<"Fatal: not an imperium repository\n";
		return;
	}
	if(argc < 3 || strlen(argv[2]) < 1){
		cout<<"Aborting commit due to empty commit message.\n";
		return;
	}
	else
		message = argv[2];
	struct stat b;
	if(stat((root + "/.imperium/.add").c_str(), &b) != 0 || S_ISDIR(b.st_mode) == 0){
		cout<<"Nothing to commit, working tree clean.\n";
		return;
	}
	
	auto timenow = chrono::system_clock::to_time_t(chrono::system_clock::now()); 
	string ibuf = ctime(&timenow);
	ibuf.pop_back();
    unsigned char obuf[20];
    SHA1((const unsigned char *)ibuf.c_str(), strlen(ibuf.c_str()), obuf);
	char s[40];
	string str;
    for (int i=0;i<20;i++) {
		sprintf(s, "%02x", obuf[i]);
		str=str+s;
    }
	string s1 = root + "/.imperium/.commit";
	struct stat buffer;
	if (stat(s1.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode))
	{
		string line;
		ifstream fin;
    	fin.open(root+"/.imperium/commit.log");
    	getline(fin, line);
		int a = line.find(" ")+1;
		int b = line.find("//")-1;
		line = line.substr(a, b-a);
		fin.close();
		string headhash = s1 + "/" + line;
		string commithash = s1 + "/" + str;
		for (auto& p: fs::recursive_directory_iterator(headhash))
		{
			string s = p.path();
			string relative = s.substr(headhash.length());
			string snew = root + relative;
			struct stat buffer1;
			if(stat(snew.c_str(), &buffer1) == 0)
			{
				if(S_ISDIR(buffer1.st_mode))
				{
					fs::create_directories(commithash + relative);
				}
				else
				{
					fs::copy_file(s, commithash + relative, fs::copy_options::overwrite_existing);
				}
			}
		}
		string addcopy = root + "/.imperium/.add";
		for (auto& p: fs::recursive_directory_iterator(addcopy))
		{
			string s = p.path();
			string relative = s.substr(addcopy.length());
			struct stat buffer1;
			if(stat(s.c_str(), &buffer1) == 0)
			{
				if(S_ISDIR(buffer1.st_mode))
				{
					fs::create_directories(commithash + relative);
				}
				else
				{
					fs::copy_file(s, commithash + relative, fs::copy_options::overwrite_existing);
				}
			}
		}
	}
	else
	{
		mkdir(s1.c_str(), 0777);
		filesystem::copy(root + "/.imperium/.add", s1 + "/" + str, filesystem::copy_options::recursive);
	}
	remove((root + "/.imperium/add.log").c_str());
	fs::remove_all(root + "/.imperium/.add");
	updateCommitLog(str, message, ibuf);
	
}

int main(int argc, char* argv[]) {
	const char* path = getenv("dir");
	root = path;
	if(strcmp(argv[1], "init") == 0)
		init(path);
	else if(strcmp(argv[1], "add") == 0)
		add(argc, argv);
	else if(strcmp(argv[1], "commit") == 0)
		commit(argc, argv);
	return 0;
}