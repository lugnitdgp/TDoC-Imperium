#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <vector>
#include <algorithm>
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
			string folderrel = imperiumrel.substr(0, imperiumrel.find_last_of("/"));
			struct stat b;
			if (stat(folderrel.c_str(), &b) != 0 || S_ISDIR(b.st_mode) == 0)
				fs::create_directories(folderrel);
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
	fout<<"commit "<<hash<<" // "<<time<<" + "<<message<<" --> HEAD"<<"\n";
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
		mkdir(commithash.c_str(), 0777);
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
		fs::copy(root + "/.imperium/.add", s1 + "/" + str, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
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

void commitlog()
{
	ifstream fin;
	fin.open(root + "/.imperium/commit.log");
	string s;
	while (getline(fin, s))
	{
		int found = s.find("-->"); 
		if(found!=string::npos)
			s = s.substr(0, found-1);
		int f = s.find("//"); 
		string a;
		if(f!=string::npos)
			a = s.substr(0, f-1);
		cout<<a<<"\n";
		int b = s.find("+");
		string c = s.substr(f+2, b-f-2);
		cout<<"Date: "<<c<<"\n";
		string d = s.substr(b+2);
		cout<<"Message: "<<d<<"\n\n";
	}
	fin.close();
}

void checkout(int argc, char* argv[])
{
	if(argc < 3){
		cout<<"Please enter the hash.\n";
		return;
	}
	ifstream fin;
	fin.open(root + "/.imperium/commit.log");
	string s;
	bool directoryExists = false;
	while (getline(fin, s))
	{
		int found = s.find(argv[2]);
		if(found!=string::npos)
			directoryExists = true;
	}
	if(directoryExists)
	{
		string commitFolderPath = root + "/.imperium/.commit/" + argv[2];
		fs::copy(commitFolderPath, root, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
	}
	else
	{
		cout<<"Please enter a valid hash.\n";
		return;
	}
	fin.close();
}

bool compareFiles(const string& filename1, const string& filename2) //returns false if files have differences in them
{
	std::ifstream file1(filename1, std::ifstream::ate | std::ifstream::binary); //open file at the end
    std::ifstream file2(filename2, std::ifstream::ate | std::ifstream::binary); //open file at the end
    const std::ifstream::pos_type fileSize = file1.tellg();

    if (fileSize != file2.tellg()) {
        return false; //different file size
    }

    file1.seekg(0); //rewind
    file2.seekg(0); //rewind

    std::istreambuf_iterator<char> begin1(file1);
    std::istreambuf_iterator<char> begin2(file2);

    return std::equal(begin1,std::istreambuf_iterator<char>(),begin2); //Second argument is end-of-range iterator
}

void status()
{
	vector<string> staged, notStaged;
	ifstream fin;
	string line;
	struct stat b;
	if(stat((root + "/.imperium/.commit").c_str(), &b) == 0 && S_ISDIR(b.st_mode))
	{
		fin.open(root + "/.imperium/commit.log");
		getline(fin, line);
		int a = line.find(" ")+1;
		int b = line.find("//")-1;
		line = line.substr(a, b-a);
		fin.close();
		struct stat buf;
	}
	struct stat buffer;
	if(stat((root + "/.imperium/.add").c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode))
	{		
		fin.open(root + "/.imperium/add.log");
		string s;
		while (getline(fin, s))
		{
			string relative = s.substr(root.length());
			string prevcommit = root + "/.imperium/.commit/" + line + relative;
			struct stat buffer1;
			if(stat((root + "/.imperium/.add" + relative).c_str(), &buffer1) == 0 && S_ISDIR(buffer1.st_mode) == 0)
			{
				struct stat buf;
				if(stat(prevcommit.c_str(), &buf) != 0 || stat((root + "/.imperium/.commit").c_str(), &buf) != 0)
					staged.push_back("created: " + relative);
				if(find(staged.begin(), staged.end(), ("created: " + relative)) == staged.end())
					staged.push_back("modified: " + relative);
				else{
					if(compareFiles(root + relative, root + "/.imperium/.add" + relative) == false)
						notStaged.push_back("modified: " + relative);
				}
			}
		}
	}
	struct stat buff;
	if(stat((root + "/.imperium/.commit").c_str(), &buff) == 0 && S_ISDIR(buff.st_mode))
	{
		for (auto& p: fs::recursive_directory_iterator(root))
		{
			string s = p.path();
			if(ignoreCheck(s))
				continue;
			string relative = s.substr(root.length());
			string prevcommit = root + "/.imperium/.commit/" + line + relative;
			struct stat buffer1;
			if(stat(s.c_str(), &buffer1) == 0 && S_ISDIR(buffer1.st_mode) == 0)
			{
				struct stat buf;
				if((stat(prevcommit.c_str(), &buf) != 0) && find(staged.begin(), staged.end(), ("created: " + relative)) == staged.end())
					notStaged.push_back("created: " + relative);
				else{
					if(stat(prevcommit.c_str(), &buf) == 0){
						if(compareFiles(s, prevcommit) == false){
							if(find(staged.begin(), staged.end(), ("modified: " + relative)) == staged.end())
								notStaged.push_back("modified: " + relative);
						}
					}
				}
			}
		}
	}
	else
	{
		for (auto& p: fs::recursive_directory_iterator(root))
		{
			string s = p.path();
			if(ignoreCheck(s))
				continue;
			string relative = s.substr(root.length());
			struct stat buf;
			if(stat((root + "/.imperium/.add" + relative).c_str(), &buf) != 0)
				notStaged.push_back("created: " + relative);
		}
	}
	if(staged.empty())
		cout<<"No changes staged for commit.\n";
	else{
		cout<<"Changes staged for commit:\n\n";
		for(auto i=staged.begin(); i!=staged.end();i++) 
        cout<<*i<<"\n";
	}
	if(notStaged.empty()==false){
		cout<<"\nChanges not staged for commit:\n\n";
		for(auto i=notStaged.begin(); i!=notStaged.end();i++) 
        cout<<*i<<"\n";
	}
}

void mergeConflictTrue()
{
	ofstream fout;
	fout.open(root + "/.imperium/conflict", ios::trunc);
	fout<<"true\n";
	fout.close();
}

void resolve()
{
	ofstream fout;
	fout.open(root + "/.imperium/conflict", ios::trunc);
	fout<<"false\n";
	fout.close();
}

void revert(int argc, char* argv[])
{
	if(argc < 3){
		cout<<"Please enter the hash.\n";
		return;
	}
	string passedhash = argv[2];
	string headhash,lasthash;
	ofstream fout;
	ifstream fin;
    fin.open(root+"/.imperium/commit.log");
    getline(fin, headhash);
	headhash = headhash.substr(7, 40);
	fin.close();
	fin.open(root + "/.imperium/commit.log");
	string s;
	bool directoryExists = false;
	while (getline(fin, s))
	{
		int found = s.find(passedhash);
		if(found!=string::npos){
			directoryExists = true;
			break;
		}
	}
	if(directoryExists)
	{
		bool flag = true;
		for (auto& p: fs::recursive_directory_iterator(root))
		{
			string s = p.path();
			if(ignoreCheck(s))
				continue;
			string relative = s.substr(root.length());
			struct stat buf;
			if(stat(s.c_str(), &buf) == 0 && S_ISDIR(buf.st_mode) == 0)
			{
				struct stat buff;
				if(stat((root + "/.imperium/.commit/" + headhash + relative).c_str(), &buff) == 0 &&  S_ISDIR(buff.st_mode) == 0)
				{
					if(compareFiles(s, (root + "/.imperium/.commit/" + headhash + relative)) == false) //file was modified after latest commit
						flag = false;
				}
				else //file was created after latest commit
					flag = false;
			}
		}
		if(flag) //No uncommited changes
		{
			if(getline(fin, s)){
				fin.close();
				if(headhash == passedhash){
					cout<<"Reverting the latest commit.\n";
					lasthash = s.substr(7, 40);
					string commitFolderPath = root + "/.imperium/.commit/" + lasthash;
					fs::copy(commitFolderPath, root, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
					for (auto& p: fs::recursive_directory_iterator(root + "/.imperium/.commit/" + headhash))
					{
						string s = p.path();
						string relative = s.substr((root + "/.imperium/.commit/" + headhash).length());
						struct stat buf;
						if(stat(s.c_str(), &buf) == 0) //path exists in headhash
						{
							if(S_ISDIR(buf.st_mode) == 0) //isnt a directory
							{
								struct stat buff;
								if(stat((commitFolderPath + relative).c_str(), &buff) != 0) //path doesnt exist in lasthash
									remove((root + relative).c_str()); //delete file from root
							}
						}
					}
				}
				else{
					cout<<"Reverting a commit before the latest commit, but after the first one.\n";
					lasthash = s.substr(7, 40);
					string commitFolderPath = root + "/.imperium/.commit/" + lasthash;
					for (auto& p: fs::recursive_directory_iterator(commitFolderPath))
					{
						string s = p.path();
						string relative = s.substr(commitFolderPath.length());
						struct stat buf;
						if(stat(s.c_str(), &buf) == 0 && S_ISDIR(buf.st_mode) == 0)
						{
							struct stat buff;
							if((stat((root + "/.imperium/.commit/" + headhash + relative).c_str(), &buff) == 0 &&  S_ISDIR(buff.st_mode) == 0) && (compareFiles(s, (root + "/.imperium/.commit/" + headhash + relative)) == false))
							{
								//merge conflict Subsequence B, Type III.
								fin.open(s);
								fout.open(root + relative, ios::app);
								fout<<"\nMERGE CONFLICT, CURRENT FILE.\n\n";
								fout<<fin.rdbuf(); 
								fout.close();
								cout<<"This file has a merge conflict: "<< relative <<"\n";
								mergeConflictTrue();
							} 
							else{
								struct stat buffer2;
								if(stat((root + "/.imperium/.commit/" + passedhash + relative).c_str(), &buffer2) != 0)
									fs::copy_file(s, root + relative, fs::copy_options::overwrite_existing);
							}
						}
					}
					for (auto& p: fs::recursive_directory_iterator(root + "/.imperium/.commit/" + passedhash))
					{
						string s = p.path();
						string relative = s.substr((root + "/.imperium/.commit/" + passedhash).length());
						struct stat buf;
						if(stat(s.c_str(), &buf) == 0 && S_ISDIR(buf.st_mode) == 0) //if it exists and is a file
						{
							struct stat buff;
							if(stat((root + "/.imperium/.commit/" + lasthash + relative).c_str(), &buff) != 0) //if it doesnt exist in lasthash
							{
								struct stat buffer;
								if(stat((root + "/.imperium/.commit/" + headhash + relative).c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode) == 0)
								{
									if(compareFiles(s, (root + "/.imperium/.commit/" + headhash + relative)) == false){
										// Merge Conflict (Sequence B, part I)
										fout.open(root + relative, ios::app);
										fout<<"\nThese are the Current Files.\n";
										fout<<"This file was created in your reverted commit, "<< passedhash <<" , but your subsequent changes have been preserved.\n";
										fout.close();
										cout<<"This file has a merge conflict: "<< relative <<"\n";
										mergeConflictTrue();
									}
									else
										remove((root + relative).c_str());
								}
							}
						}
					}
				}
			}
			else{
				cout<<"Commit hash provided is the first commit.\n";
				for (auto& p: fs::recursive_directory_iterator(root + "/.imperium/.commit/" + passedhash))
				{
					string s = p.path();
					string relative = s.substr((root + "/.imperium/.commit/" + passedhash).length());
					struct stat buf;
					if(stat(s.c_str(), &buf) == 0 && S_ISDIR(buf.st_mode) == 0) //if it exists and is a file
					{
						struct stat buffer;
						if(stat((root + "/.imperium/.commit/" + headhash + relative).c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode) == 0)
						{
							if(compareFiles(s, (root + "/.imperium/.commit/" + headhash + relative)))
								remove((root + relative).c_str());
							else{
								//Merge Conflict Subsequence B, part II
								fout.open(root + relative, ios::app);
								fout<<"\nThese are the Current Files.\n";
								fout<<"This file was created in your reverted commit, "<< passedhash <<" , but your subsequent changes have been preserved.\n";
								fout.close();
								cout<<"This file has a merge conflict: "<< relative <<"\n";
								mergeConflictTrue();
							}
						}
					}
				}
			}
		}
		else
			cout<<"Please stash/commit all uncommitted changes.\n";
	}
	else
	{
		cout<<"Please enter a valid hash.\n";
		return;
	}
}

void stash(int argc, char* argv[])
{
	if(argc < 3){
		string s = root + "/.imperium/.stash";
		struct stat buf;
		if (stat(s.c_str(), &buf) != 0 || S_ISDIR(buf.st_mode) == 0)
			mkdir(s.c_str(), 0777);
		else{
			fs::remove_all(root + "/.imperium/.stash");
			mkdir(s.c_str(), 0777);
		}
		for (auto& p: fs::recursive_directory_iterator(root))
		{
			string s = p.path();
			if(ignoreCheck(s))
				continue;
			string relative = s.substr(root.length());
			struct stat buf;
			if(stat(s.c_str(), &buf) == 0)
			{
				if(S_ISDIR(buf.st_mode))
				{
					fs::create_directories(root + "/.imperium/.stash" + relative);
				}
				else
				{
					fs::copy_file(s, root + "/.imperium/.stash" + relative, fs::copy_options::overwrite_existing);
				}
			}
		}
		string headhash;
		ifstream fin;
		fin.open(root+"/.imperium/commit.log");
		getline(fin, headhash);
		headhash = headhash.substr(7, 40);
		fin.close();
		string commitFolderPath = root + "/.imperium/.commit/" + headhash;
		fs::copy(commitFolderPath, root, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
		
	}
	else if(strcmp(argv[2], "apply") == 0){
		string s = root + "/.imperium/.stash";
		struct stat buf;
		if (stat(s.c_str(), &buf) != 0 || S_ISDIR(buf.st_mode) == 0)
			cout<<"Please stash the changes first.\n";
		else{
			fs::copy(root + "/.imperium/.stash", root, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
		}
	}
	else
		cout<<"Please enter a valid command.\n";
}

void rm(int argc, char* argv[])
{
	if(argc < 3){
		cout<<"Please enter a valid command.\n";
	}
	else if(strcmp(argv[2], "--cached") == 0){
		string path;
		if (strcmp(argv[3], ".") == 0)
			path = root + "/.imperium/.add";
		else
			path = root + "/.imperium/.add/" + argv[3];
		struct stat buf;
		if (stat(path.c_str(), &buf) == 0)
		{
			if(S_ISDIR(buf.st_mode))
			{
				fs::remove_all(path);
			}
			else
			{
				remove((path).c_str());
			}
			struct stat buffer;
			if(stat((root + "/.imperium/.add").c_str(), &buffer) != 0)
				remove((root + "/.imperium/add.log").c_str());
			else{
				ofstream fout;
				fout.open(root + "/.imperium/add.log", ios::trunc);
				string addpath = root + "/.imperium/.add";
				for (auto& p: fs::recursive_directory_iterator(addpath))
				{
					string s = p.path();
					string relative = s.substr(addpath.length());
					struct stat buffer1;
					if(stat(s.c_str(), &buffer1) == 0)
					{
						fout<<root + relative<<"\n";
					}
				}
				fout.close();
			}
		}
		else
			cout<<"Enter the name of a staged path/file.\n";
	}
	else if(strcmp(argv[2], "-r") == 0){
		string path = root + "/" + argv[3];
		struct stat buf;
		if (stat(path.c_str(), &buf) == 0)
		{
			if(S_ISDIR(buf.st_mode))
			{
				fs::remove_all(path);
			}
			else
				cout<<"Enter the path of a directory.\n";
		}
		else
			cout<<"Enter a valid path.\n";
	}
	else{
		string path = root + "/" + argv[2];
		struct stat buf;
		if (stat(path.c_str(), &buf) == 0)
		{
			if(S_ISDIR(buf.st_mode) == 0)
			{
				remove((path).c_str());
			}
			else
				cout<<"Enter the path of a file.\n";
		}
		else
			cout<<"Enter a valid path.\n";
	}
}

int main(int argc, char* argv[]) {
	const char* path = getenv("dir");
	root = path;
	ifstream fin;
	fin.open(root + "/.imperium/conflict");
	string line;
	getline(fin, line);
	fin.close();
	if(strcmp(argv[1], "init") == 0)
		init(path);
	else if(strcmp(argv[1], "resolve") == 0)
		resolve();
	else if(line == "false"){
		if(strcmp(argv[1], "add") == 0)
			add(argc, argv);
		else if(strcmp(argv[1], "commit") == 0)
			commit(argc, argv);
		else if(strcmp(argv[1], "log") == 0)
			commitlog();
		else if(strcmp(argv[1], "checkout") == 0)
			checkout(argc, argv);
		else if(strcmp(argv[1], "status") == 0)
			status();
		else if(strcmp(argv[1], "revert") == 0)
			revert(argc, argv);
		else if(strcmp(argv[1], "stash") == 0)
			stash(argc, argv);
		else if(strcmp(argv[1], "rm") == 0)
			rm(argc, argv);
	}
	else
		cout<<"Please solve the merge conflict before using any other commands.\n";
	return 0;
}