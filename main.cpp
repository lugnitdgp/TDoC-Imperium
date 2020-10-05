#include <bits/stdc++.h>
#include<fstream> 
#include<utility>
#include<sys/stat.h>
#include<experimental/filesystem>
#include<filesystem>
#define pb push_back
using namespace std;
namespace fs=std::experimental::filesystem;
namespace fsnew=std::filesystem;
string root;
void init(string &path){
	struct stat buffer;
	if(stat((path+"/.imperium").c_str(),&buffer)==0){
		cout<<"Repository has been already initialized\n"; return;
	}
	
	string ignorePath=path+"/.imperiumignore";
	ofstream ignore(ignorePath.c_str());
	ignore<<".gitignore\n.imperium\n.git\n.imperiumignore\n.node_modules\n";
	ignore.close();
	path+="/.imperium";
	if(mkdir(path.c_str(),0777)!=0) {cout<<"_ERROR_\n"; return;}
	
	string commitLog=path+"/commit.log";
	ofstream commit(commitLog.c_str());
	commit.close();
	
	string addLog=path+"/add.log";
	ofstream add(addLog.c_str());
	add.close();
	
	string conflictLog=path+"/conflict.log";
	ofstream conflict(conflictLog.c_str());
	conflict<<"False\n";
	conflict.close();

	cout<<"Initialized an empty repository\n";
}
void addtoCache(string addPath,char type){
	struct stat buffer;
	if(stat((root+"/.imperium/.add").c_str(),&buffer)) {
		if(mkdir((root+"/.imperium/.add").c_str(),0777)) {cout<<"Error could not create cache folder\n"; return;}
	}
	if(type=='d'){
		string filename=addPath.substr(root.length());
		string filerel=root+"/.imperium/.add"+filename;
		struct stat buffer3;
		if(stat(filerel.c_str(),&buffer3)!=0)
			fsnew::create_directories(filerel);
	}
	else if(type=='f'){
		string filename=addPath.substr(root.length());
		string filerel=root+"/.imperium/.add" + filename.substr(0,filename.find_last_of("/"));
		struct stat buffer2;
		if(stat(filerel.c_str(),&buffer2)!=0) 
			fsnew::create_directories(filerel);
		fsnew::copy_file(addPath,root+"/.imperium/.add"+filename,fsnew::copy_options::overwrite_existing);
	}
}
bool ignoreFolder(string addPath,vector<string> &ignoreDir)
{
	for(auto dir:ignoreDir){
		if(dir.find(addPath)!=string::npos) return true;
		if(addPath.find(dir)!=string::npos) return true;
	}
	return false;
}
bool toBeIgnored(string addPath,int onlyImperiumIgnore=0)
{
	ifstream addFIle, ignoreFile;
	string file_or_dir;
	vector<string> ignorefilenames;
	vector<pair<string,char>> addedFileNames;
	vector<string> ignoreDir;
	ignoreFile.open(root+"/.imperiumignore");
	addFIle.open(root+"/.imperium/add.log");
	if(ignoreFile.is_open()){
		while(!ignoreFile.eof()){
			getline(ignoreFile,file_or_dir);
			if(file_or_dir.back()=='/') ignoreDir.push_back(root+"/"+file_or_dir);
			else ignorefilenames.push_back(root+"/"+file_or_dir);
		}
	}
	ignoreFile.close();
	if(onlyImperiumIgnore==0){
		if(addFIle.is_open()){
			while(!addFIle.eof()){
				getline(addFIle,file_or_dir);
				int len=file_or_dir.length();
				if(len>4){
					addedFileNames.pb({file_or_dir.substr(1,len-4),file_or_dir.back()});
				}
			}
		}
		addFIle.close();
		for(auto fileEntry:addedFileNames){
			if(addPath.compare(fileEntry.first)==0){
				addtoCache(addPath,fileEntry.second);
				cout<<"Updated "<<addPath<<endl;
				return true;
			}
		}
	}
	if(find(ignorefilenames.begin(),ignorefilenames.end(),addPath)!=ignorefilenames.end()||ignoreFolder(addPath,ignoreDir)) return true;
	else return false;
}

void add(string &addPath){
	string checkPath=root+"/.imperium";
	struct stat buf;
	if(stat(checkPath.c_str(),&buf)) {
		cout<<"repository has not been initialized yet\n"; return;
	}
	ofstream addFIle;
	addFIle.open(root+"/.imperium/add.log",std::ios_base::app);
	struct stat buffer;
	if(stat(addPath.c_str(),&buffer)!=0) {cout<<"ERROR: path doesnot exists.\n"; return;}
	if(buffer.st_mode & S_IFDIR) {
		if(!toBeIgnored(addPath)) {
			 addFIle<<"\""<<addPath<<"\""<<"-d\n";
			 addtoCache(addPath,'d');
			 cout<<"Added directory "<<"\""<<addPath<<"\""<<endl;
		}
		for(auto &p: fsnew::recursive_directory_iterator(addPath))
		{
			if(toBeIgnored(p.path())) { continue;}
			struct stat buffer2;
			if(stat(p.path().c_str(),&buffer2)==0){
				if(buffer2.st_mode & S_IFDIR) {
					 addFIle<<p.path()<<"-d\n";
					 addtoCache(p.path(),'d');
					 cout<<"Added Directory "<<p.path()<<endl;
				}
				else if(buffer2.st_mode & S_IFREG){
					 addFIle<<p.path()<<"-f\n";
					 addtoCache(p.path(),'f');
					 cout<<"Added File "<<p.path()<<endl;
				}
				else continue;
			}
		}	
	}
	else if(buffer.st_mode & S_IFREG){
		if(toBeIgnored(addPath)) return;
		addFIle<<"\""<<addPath<<"\""<<"-f\n";
		addtoCache(addPath,'f');
		cout<<"Added File "<<addPath<<endl;
	}
	else cout<<"Error : Invalid Path\n";
	addFIle.close();
}
int main(int argc,char* argv[]) {
	if(argc==1) {cout<<"Imperium\n"; return 0;}
	root=getenv("dir");
	if(strcmp(argv[1],"init")==0)
	{
        init(root);
	}
	else if(strcmp(argv[1],"add")==0){
		string addPath=root; 
		if(strcmp(argv[2],".")!=0) {addPath+="/"; addPath+=argv[2];}
		add(addPath);
	}
	return 0;
}