#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <fstream>
#include <filesystem>  

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
			struct stat b;
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

void add(char* argv[])
{
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

int main(int argc, char* argv[]) {
	const char* path = getenv("dir");
	root = path;
	if(strcmp(argv[1], "init") == 0)
		init(path);
	else if(strcmp(argv[1], "add") == 0)
		add(argv);
	return 0;
}