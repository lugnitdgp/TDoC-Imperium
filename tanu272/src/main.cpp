#include <cstdio>
#include <iostream>
#include <utility>
#include <fstream>
#include <cstring>
#include <bits/stdc++.h>
#include <sys/stat.h>
#include <experimental/filesystem>

using namespace std;
namespace fs = experimental::filesystem;

string root = "";

void init(string path)
{
    struct stat sb;
    string path_new = path + "/.imperium";
    if (stat(path_new.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
        printf("Repository already initialised.\n");
    } 
    else {

        string ignorepath = path + "/.imperiumignore";
        ofstream ignore(ignorepath.c_str());

        ignore<<".gitignore\n.imperium\n.git\n.imperiumignore\n.node_modules\n";
        ignore.close();
        
        path+="/.imperium";
        //cout<<path;
        int created = mkdir(path.c_str(), 0777);
        
        if(created==0)
        {
            string commitlog= path + "/commit.log";
            ofstream commit(commitlog.c_str());
            commit.close();

            string addlog= path + "/add.log";
            ofstream add(addlog.c_str());
            add.close();
            
            string conflictlog= path + "/conflict.log";
            ofstream conflict(conflictlog.c_str());
            conflict.close();

            cout<<"Initialize a new directory"<<path<<"\""<<"\n";
        }
        else
        {
            cout<<"ERROR\n";
        }
    }
}

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

int main(int argc, char* argv[])
{
    const string dir = getenv("dir");
    root=dir;
    if(strcmp(argv[1],"init")==0)
        init(root);
    if(strcmp(argv[1],"add")==0)
        add(argv);
    return 0;
}
