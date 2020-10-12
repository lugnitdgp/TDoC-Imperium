#include<iostream>
#include<sys/stat.h>
#include<utility>
#include <openssl/sha.h>
#include<chrono>
#include<sys/types.h>
#include<fstream>
#include<string>
#include<string.h>
#include<vector>
#include<bits/stdc++.h>
#include<experimental/filesystem>
namespace fs = std::experimental::filesystem;
std::string root= "";
void init(std::string path)
{
	struct stat buffer;
	if(stat((path+"/.imperium").c_str(),&buffer)==0)
	{
		std::cout<<"REPOSITORY ALREADY CREATED\n";
	}
	else{
	std::string ignorepath = path + "/.imperiumignore";
	std::ofstream ignore(ignorepath.c_str());
		ignore<<".gitignore\n.imperium\n.git\n.imperiumignore\n.node_modules\n";
	    ignore.close();
	path+="./imperium";
	int created=mkdir(path.c_str(),0777);
	if(created==0)
	{
		std::string commitlog	=	path +"/commitlog";
		std::ofstream commit(commitlog.c_str());
		commit.close();
		std::string addlog	=	path +"/addlog";
		std::ofstream add(addlog.c_str());
		add.close();
		std::string conflictlog	=	path +"/conflictlog";
		std::ofstream conflict(conflictlog.c_str());
		conflict<<"False\n";
		conflict.close();
		std::cout<<"INITIALISED AN EMPTY REPOSITORY\n";
	}
	else
	{
		std::cout<<"!!ERROR!!\n";
	}
	   }
}
bool ignoreFolder(std::string path,std::vector<std::string> dirname)
{
	for(auto dir:dirname)
	{
		dir.pop_back();
		if(dir.find(path)!=std::string::npos)
		{
			return true;
		}
	}
	return false;
}
void addToCache(std::string path,char type)
{
	struct stat buffer;
	if(stat((root+"/.imperium/.add").c_str(),&buffer)!=0)
	{
		mkdir((root+"/.imperium/.add").c_str(),0777);
	}
	if(type=='f')
	{
		std::string filename = path.substr(root.length());
		std::string filerel = root+"/.imperium/.add"+filename.substr(0,filename.find_last_of("/"));
		struct stat buffer2;
		if(stat(filerel.c_str(),&buffer2)!=0)
		{
			fs::create_directories(filerel);
			
		}
		fs::copy_file(path,root+"/.imperium/.add"+filename,fs::copy_options::overwrite_existing);
	}
	else if(type=='d')
	{
		std::string filename=path.substr(root.length());
		std::string filerel=root+"/.imperium/.add"+filename;
		struct stat buffer3;
		if(stat(filerel.c_str(),&buffer3)!=0)
		{
			fs::create_directories(filerel);
		}
	}
}
bool tobeignored(std::string path,int onlyImperiumIgnore=0)
{
	std::ifstream addFile,ignoreFile;
	std::string file_or_dir;
	std::vector<std::string> filenames;
	std::vector<std::pair<std::string,char>>addedFileNames;
	std::vector<std::string> ignoreDir;
	ignoreFile.open(root+"/.imperiumignore");
	addFile.open(root+"/.imperium/add.log");
	if(ignoreFile.is_open())
	{
		while(!ignoreFile.eof())
		{
			std::getline(ignoreFile,file_or_dir);
			auto i=file_or_dir.end();
			i--;
			if(*i=='/')
			{
				ignoreDir.push_back(file_or_dir);
			}
			else{
				filenames.push_back(root+file_or_dir);
			}
		}
	}
	ignoreFile.close();
	if(onlyImperiumIgnore==0)
	{
		if(addFile.is_open())
		{
			while(!addFile.eof())
			{
				std::getline(addFile,file_or_dir);
				if(file_or_dir.length()>4)
				{
					addedFileNames.push_back(std::make_pair(file_or_dir.substr(file_or_dir.length()-4),file_or_dir.at(file_or_dir.length()-1)));
				}
			}
		}
		addFile.close();
		for(auto fileEntry:addedFileNames)
		{
			if(path.compare(fileEntry.first)==0)
			{
				addToCache(path,fileEntry.second);
				std::cout<<"updated"<<path<<std::endl;
				return true;
			}
		}
	}
	if(std::find(filenames.begin(),filenames.end(),path)!=filenames.end() or ignoreFolder(path,ignoreDir))
		return true;
	return false;
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
	void add(char **argv)
	{
		struct stat buffer;
		if(stat((root+"/.imperium").c_str(),&buffer)==0)
		{
			std::ofstream addFile;
			addFile.open(root+"/.imperium/add.log",std::ios_base::app);
			std::string path=root;
			if(strcmp(argv[2],".")!=0)
			{
				path+="/";
				path+=argv[2];
			}
			struct stat buffer2;
			if(stat(path.c_str(),&buffer2)==0)
			{
				if(buffer2.st_mode&S_IFDIR)
				{
					if(!tobeignored(path))
					{
						addFile<<"\""<<path<<"\""<<"-d\n";
						addToCache(path,'d');
						std::cout<<"Added Directory"<<"\""<<path<<"\""<<"\n";
						
					}
					for(auto &p:fs::recursive_directory_iterator(path))
					{
						if(tobeignored(p.path()))
						{
							continue;
						}
						struct stat buffer3;
						if(stat(p.path().c_str(),&buffer3)==0)
						{
							if(buffer3.st_mode&S_IFDIR)
							{
								addToCache(p.path(),'d');
								addFile<<p.path()<<"-d\n";
								std::cout<<"Added Directory"<<"\""<<p.path()<<"\""<<"\n";
								
							}
							else if(buffer3.st_mode&S_IFREG)
							{
								addToCache(p.path(),'f');
								addFile<<p.path()<<"-f\n";
								std::cout<<"Added File:"<<"\""<<p.path()<<"\""<<"\n";
							}
							else{
								continue;
							}
						}
					}
				}
				else if(buffer2.st_mode&S_IFREG)
				{
					if(!tobeignored(path))
					{
						addFile<<"\""<<"path"<<"\""<<"-f\n";
						addToCache(path,'f');
						std::cout<<"Added the file"<<"\""<<path<<"\""<<"\n";
					}
					
				}
				else{
					std::cout<<"invalid path"<<"\n";
				}
			}
		}
		else{
			std::cout<<"Repository not yet initialised"<<std::endl;
		}
	}
std::string getTime()
{
	auto end=std::chrono::system_clock::now();
	std::time_t end_time=std::chrono::system_clock::to_time_t(end);
	std::string time=std::ctime(&end_time);
	return time;
}
std::string getcommitHash()
{
	std::string commitfilename=getline();
	std::string commithash="";
	char buf[3];
	int length=commitfilename.length();
	unsigned char hash[20];
	unsigned char *val=new unsigned char[length+1];
	strcpy((char*)val,commitfilename.c_str());
	SHA1(val,length,hash);
	for(int i=0;i<20;i++)
	{
		sprintf(buf,"%02x",hash[i]);
		commithash+=buf;
	}
	return commithash;
}
void getcommitlog()
{
	std::string commitlogpath=root+"/.imperium/commit.log";
	std::string commitline;
	std::string commitlog;
	commitlog.open(commitlogpath);
	while(std::getline(commitlog,commitline))
	{
		std::cout<<commitline<<std::endl;
	}
}
void repeatCommit(std::string abspath,char type,std::string commitHash)
{
	mkdir((root+"/.imperium/.commit/"+commitHash).c_str(),0777);
	std::string relpath =abspath.substr(root.length()+19+commitHash.length());
	std::string filepath=root+"/.imperium/.commit/"+commitHash+relpath.substr(0,find_last_of('/'));
	fs::create_directories(filepath);
	if(type=='f')
	{
		fs::copy_file(abspath,root+"/.imperium/.commit/"+commitHash+relpath,fs::copy_options::overwrite_existing);
	}
}
void addcommit(std::string abspath,char type,std::string commitHash)
{
	struct stat s;
	if((stat(root+"/.imperium/.commit/").c_str(),&s)!=0)
	{
		mkdir((root+"/.imperium/.commit/").c_str(),0777);
	}
	if((stat(root+"/.imperium/.commit/"+commitHash).c_str(),&s)!=0)
	{
		mkdir((root+"/.imperium/.commit"+commitHash).c_str(),0777);
	}
	std::string relpath=abspath.substr(root.length()+15);
	std::string filepath=root+"/.imperium/.commit"+commitHash+relpath.substr(0,find_last_of('/'));
	fs::create_directories(filepath);
	if(type=='f')
	{
		fs::copy_file(abspath,root+"/.imperium/.commit"+commitHash+relpath,fs::copy_options::overwrite_existing);
	}
}
void updatecommitlog(std::string commitHash,std::string message)
{
	std::ofstream writeHeadlog;
	writeHeadlog.open(root+"/.imperium/head.log");
	writeHeadlog<<commitHash<<"--"<<message<<std::endl;
	writeHeadlog.close();
	std::ofstream writeCommitlog;
	std::ifstream readCommitlog;
	readCommitlog.open(root+"/.imperium/commit.log");
	writeCommitlog.open(root+"/.imperium/new_commit.log");
	writeCommitlog<<commitHash<<"--"<<message<<"-->HEAD\n";
	std::string line;
	while(std::getline(readCommitlog,line))
	{
		if(line.find("-->HEAD")!=std::string::npos)
		{
			writeCommitlog<<line.substr(0,line.length()-8)<<"\n";
		}
		else{
			writeCommitlog<<line<<"\n";
		}
	}
	remove((root+"/.imperium/commit.log").c_str());
	rename((root+"/.imperium/new_commit.log").c_str(),(root+"/.imperium/commit.log").c_str());
	readCommitlog.close();
	writeCommitlog.close();
	
}
void commit(std::string message)
{
	struct stat buffer;
	if(stat(root+"/.imperium").c_str(),&buffer)!=0)
	{
		std::cout<<"repository hasnt initiated\n";
	}
	std::string commitHash=getcommitHash();
	if(stat((root+"/.imperium/head.log").c_str(),&buffer)==0)
	{
		std::string headHash;
		std::ifstream readCommitlog;
		readCommitlog.open(root+"/.imperium/head.log");
		std::(readCommitlog,headHash);
		headHash=headHash.substr(0,headHash.find("--"));
		for(auto &p	: fs::recursive_directory_iterator(root+"/.imperium/.commit/"+headHash))
		{
			if(stat(p.path().c_str(),&buffer)==0)
			{
				if(buffer.st_mode&S_IFREG)
				{
					repeatCommit(p.path(),'f',commitHash);
				}
				else if(buffer.st_mode&S_IFDIR)
				{
					repeatCommit(p.path(),'d',commitHash);
				}
				else{
					continue;
				}
			}
		}
	}
	
	for(auto &p	: fs::recursive_directory_iterator(root+"/.imperium/.add"))
	{
		struct stat s;
		if(stat(p.path().c_str(),&s)==0)
		{
			if(s.st_mode&S_IFREG)
			{
				addcommit(p.path(),'f',commitHash);
			}
			else if(s.st_mode&S_IFREG)
			{
				addcommit(p.path(),'d',commitHash);
			}
		}
	}
	fs::remove_all((root+"/.imperium/.add").c_str());
	remove((root+"/.imperium/add.log").c_str());
	updatecommitlog(commitHash,message);
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
void revert(string passedHash){
	string headHash = getHead();
	vector<string> conflict;
	if(headHash=="ncf") {cout<<"No commits found\n"; return;}
	for(auto &p:fsnew::recursive_directory_iterator(root)){
		string path=p.path();
		struct stat buffer;
		if(stat(path.c_str(),&buffer)==0){
			if(buffer.st_mode & S_IFREG){
				string filerel = path.substr(root.length());
				string checkPath = root + "/.imperium/.commit/"+headHash+filerel;
				struct stat buffer1;
				if(stat(checkPath.c_str(),&buffer1)!=0) continue;
				if(comparefiles(path,checkPath)) {cout<<"please commit latest changes first\nFile "<<path<<" is modified and unstaged\n"; return;}
			}
		}
	}
	string lastHash; 
	ifstream commitLog;
	commitLog.open(root+"/.imperium/commit.log");
	bool flag=0;
	if(commitLog.is_open()){
		while(!commitLog.eof()){
			string temp;
			getline(commitLog,temp);
			if(temp.size()<20) continue;
			lastHash=temp;
			int index=-1; while(lastHash[++index]!=' ');
			lastHash = lastHash.substr(0,index);
			if(flag){ break;}
			if(lastHash==passedHash) flag=1;
		}
	}
	else {cout<<"Could not open commit log\n"; return;}
	commitLog.close();
	if(flag == 0) {cout<<"Invalid hash passed\n"; return;}
	if(lastHash==passedHash) {
		for(auto &p:fsnew::recursive_directory_iterator(string(root + "/.imperium/.commit/"+passedHash))){
			string path = p.path();
			struct stat buffer; string parent=root + "/.imperium/.commit/"+passedHash;
			string filerel = path.substr(parent.length());
			if(stat(path.c_str(),&buffer)==0) {
				if(buffer.st_mode & S_IFREG){
					string checkPath = root + "/.imperium/.commit/"+headHash+filerel;
					if(comparefiles(path,checkPath)){
						ofstream mergeConflict;
						mergeConflict.open(root+filerel,std::ios_base::app);
						mergeConflict<<"This file was created in the reverted commit and subsequent changes have been preserved\n";
						conflict.pb(root+filerel);
						mergeConflict.close();
					}
					else{
						string deletePath = root + filerel;
						if(remove(deletePath.c_str())) {cout<<"Error in deleting files\n"; return;}
					}
				}
				else if(buffer.st_mode & S_IFDIR) addCheckout(path,'d');
			}
		}
	}
	else if(passedHash==headHash){
		for(auto &p:fsnew::recursive_directory_iterator(string(root+"/.imperium/.commit/"+lastHash)))
		{
			string path = p.path();
			struct stat buffer;
			if(stat(path.c_str(),&buffer)==0){
				if(buffer.st_mode & S_IFDIR){
					addCheckout(path,'d');
				}
				else if(buffer.st_mode & S_IFREG){
					addCheckout(path,'f');
				}
			}
		}
		for(auto &p:fsnew::recursive_directory_iterator(root)){
			string path = p.path();
			string filerel = path.substr(root.length());
			struct stat buffer; 
			if(stat(path.c_str(),&buffer)==0){
				if(buffer.st_mode & S_IFDIR) continue;
			}
			string path1 = root + "/.imperium/.commit/"+lastHash+filerel;
			string path2 = root + "/.imperium/.commit/"+headHash+filerel;
			struct stat buffer1;
			if(stat(path1.c_str(),&buffer1)!=0){
				struct stat buffer2;
				if(stat(path2.c_str(),&buffer2)==0) remove(path.c_str());
			}
		}
	}
	else{
		for(auto &p:fsnew::recursive_directory_iterator(string(root+"/.imperium/.commit/"+lastHash)))
		{
			string path = p.path();
			string parent = root+"/.imperium/.commit/"+lastHash;
			string filerel =path.substr(parent.length());
			struct stat buffer;
			if(stat(path.c_str(),&buffer)==0){
				if(buffer.st_mode & S_IFREG) {
					string checkPath = root + "/.imperium/.commit/"+headHash+filerel;
					struct stat buffer2;
					if(stat(checkPath.c_str(),&buffer2)==0){
						if(comparefiles(path,checkPath)){
							ofstream mergeConflict;
							mergeConflict.open(root+filerel,std::ios_base::app);
							mergeConflict<<"Merge conflict, current file has been updated. Preserving changes.\n";
							conflict.pb(root+filerel);
							mergeConflict.close();
						}
						else addCheckout(path,'f');
					}
					else addCheckout(path,'f');
				}
			}
		}
		for(auto &p:fsnew::recursive_directory_iterator(string(root+"/.imperium/.commit/"+passedHash))){
			string path = p.path();
			string parent  = root+"/.imperium/.commit/"+passedHash;
			string filerel = path.substr(parent.length());
			struct stat buffer;
			if(stat(path.c_str(),&buffer)==0){
				if(buffer.st_mode & S_IFREG){
					string checkPath = root + "/.imperium/.commit/"+headHash+filerel;
					struct stat buffer2;
					if(stat(checkPath.c_str(),&buffer2)==0){
						if(comparefiles(path,checkPath)){
							checkPath=root + "/.imperium/.commit/"+lastHash+filerel;
							struct stat buffer3;
							if(stat(checkPath.c_str(),&buffer3)!=0)
							{
								ofstream mergeConflict;
								mergeConflict.open(root+filerel,std::ios_base::app);
								mergeConflict<<"This file was created in the reverted commit and subsequent changes have been preserved\n";
								conflict.pb(root+filerel);
								mergeConflict.close();
							}
						}
						else remove(string((root+filerel)).c_str());
					}
				}
				else remove(string((root+filerel)).c_str());
			}
		}
	}
	if(conflict.size()>0){
		cout<<"Merge conflicts found with : \n";
		string conflictPath = root + "/.imperium/conflict.log";
		remove(conflictPath.c_str());
		ofstream conflictLog(conflictPath.c_str());
		conflictLog<<"True\n";
		for(string s:conflict){
			cout<<s<<endl;
			conflictLog<<s<<"\n";
		}
		conflict.clear();
		conflictLog.close();
	}
}
bool conflict(){
	string conflictPath=root+"/.imperium/conflict.log";
	ifstream conflictLog;
	conflictLog.open(conflictPath);
	if(conflictLog.is_open()){
		string line;
		if(conflictLog.eof()) {cout<<"Conflict log empty\n"; exit(0);}
		else getline(conflictLog,line);
		return (line=="True");
	}
}
    
int main(int argc,char **argv)
{
	const std::string dir = getenv("dir");
	root=dir;
	if(strcmp(argv[1],"init")==0)
	{
		init(root);
	}
	else if(strcmp(argv[1],"add")==0)
	{
		add(argv);
	}
	else if(strcmp(argv[1],"commit")==0)
	{
		commit(argv[2]);
	}
	else if(strcmp(argv[1],"log")==0)
	{
		getcommitlog();
	}
	else if(strcmp(argv[1],"status")==0)
	{
		status();
	}
	 else if (strcmp(argv[1], "checkout") == 0){
        if (argc == 3){
            checkout(argv[2]);
        }
        else{
            cout << "ERROR. Please enter a proper commit hash.\n";
        }
    }
	else if(strcmp(argv[1],"revert")==0){
		if(argc<3) {cout<<"Please enter a revert hash\n"; return 0;}
		string revertHash=argv[2]; 
		revert(revertHash);
	}
	std::cout<<"SUCCESS"<<std::endl;
	return 0;
}
