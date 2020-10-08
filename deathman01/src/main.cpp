#include <bits/stdc++.h>
#include<fstream> 
#include<utility>
#include<sys/stat.h>
#include<experimental/filesystem>
#include<filesystem>
#include<chrono>
#include<ctime>
#include<unistd.h>
#include<openssl/sha.h>
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
	ignore<<".gitignore\n.imperium/\n.git\n.imperiumignore\n.node_modules\n";
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
	checkPath+="/add.log";
	struct stat buff2;
	if(stat(checkPath.c_str(),&buff2)!=0) { ofstream addLog(checkPath.c_str()); addLog.close(); }
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
string timeHash(){
	auto timenow = chrono :: system_clock :: to_time_t(chrono::system_clock::now());
	unsigned char result [20] = {};
	SHA1((const unsigned char *)&timenow,sizeof(timenow),result);
	string ans;
	for(int i=0;i<20;i++){
		char temp; int val= sprintf(&temp,"%x",(unsigned int)result[i]); ans+=temp;
	}
	return ans;
}
void addCommit(string path,char type,string commitHash)
{
	struct stat checkCommit;
	if(stat((root+"/.imperium/.commit").c_str(),&checkCommit)!=0) mkdir((root+"/.imperium/.commit").c_str(),0777);
	string commitHashPath=root+"/.imperium/.commit/"+commitHash;
	struct stat commitHashCheck;
	if(stat(commitHashPath.c_str(),&commitHashCheck)!=0){
	if(mkdir(commitHashPath.c_str(),0777)) {cout<<"ERROR couldnot create commit hash folder\n"; return;}}
	string filterPath=root+"/.imperium/";
	if(type =='d'){
		string filename=path.substr(filterPath.length());
		bool flag=(filename[1]!='c');
		while(1) {
			if(filename[0]=='/'){ if(flag) break; flag=1;}
			filename.erase(filename.begin());
		}
		string filerel=commitHashPath+filename;
		struct stat buffer;
		if(stat(filerel.c_str(),&buffer)!=0) fsnew::create_directories(filerel);
	}
	else if(type=='f'){
		string filename=path.substr(filterPath.length());
		bool flag=(filename[1]!='c');
		while(1) {
			if(filename[0]=='/'){ if(flag) break; flag=1;}
			filename.erase(filename.begin());
		}
		string filerel=commitHashPath+filename.substr(0,filename.find_last_of("/"));
		struct stat buffer2;
		if(stat(filerel.c_str(),&buffer2)!=0) fsnew::create_directories(filerel);
		fsnew::copy_file(path,commitHashPath+filename,fsnew::copy_options::overwrite_existing);
	}
}
void updateCommitLog(string message,string commitHash){
	string commitLogPath=root+"/.imperium/commit.log";
	struct stat buffer;
	if(stat(commitLogPath.c_str(),&buffer)!=0) {ofstream commitLog(commitLogPath.c_str()); commitLog.close();}
	ofstream tempCommitLog((root+"/.imperium/tempCommit.log").c_str());
	tempCommitLog<<commitHash<<" -- "<<message<<" --> HEAD\n";
	ifstream originalCommit; originalCommit.open(commitLogPath);
	if(originalCommit.is_open()){
		bool flag=0; string line;
		while(!originalCommit.eof()){
			getline(originalCommit,line);
			if(!flag) { line=line.substr(0,line.length()-9); flag=1; }
			tempCommitLog<<line<<"\n";
		}
	}
	else {cout<<"Cannot open commit.log\n"; return;}
	tempCommitLog.close(); originalCommit.close();
	remove(commitLogPath.c_str());
	rename((root+"/.imperium/tempCommit.log").c_str(),commitLogPath.c_str());
}
void commit(char *argv[],int argc){
	struct stat checkImperium;
	if(stat((root+"/.imperium").c_str(),&checkImperium)!=0) {cout<<"Repository has not been initialized yet\n"; return;}
	if(strcmp(argv[2],"-m")!=0) {cout<<"Please write some message\n"; return;}
	string message;
	for(int i =3 ;i<argc;i++) {message+=argv[i]; message+=" "; }
	string addPath=root+"/.imperium/.add";
	struct stat addCheck; 
	if(stat(addPath.c_str(),&addCheck)!=0) {cout<<".add folder not created\n"; return;}
	string commitHash=timeHash();
	for(auto &absPath:fsnew::recursive_directory_iterator(addPath))
	{
		struct stat buffer;
		if(stat(absPath.path().c_str(),&buffer)!=0) {cout<<"ERROR path doesnot exist\n"; return;}
		char type;
		if(buffer.st_mode & S_IFDIR) type='d';
		else if(buffer.st_mode & S_IFREG) type='f';
		else continue;
		addCommit(absPath.path(),type,commitHash);
	}
	ifstream checkCommit((root+"/.imperium/commit.log").c_str());
	if(checkCommit.is_open()){
		string lastCommit; bool flag=0;
		while(!checkCommit.eof()){
			getline(checkCommit,lastCommit);
			if(lastCommit.length()<9) break;
			lastCommit=lastCommit.substr(0,lastCommit.length()-9);
			flag=1;
			int index; int len=lastCommit.length();
			for(index=len-1;index>=0;index--){
				if(lastCommit[index]==' ') { index++; break;}
			}
			lastCommit=lastCommit.substr(index);
			break;
		}
		checkCommit.close();
		if(flag)
		{
			lastCommit=root+"/.imperium/.commit/"+lastCommit;
		for(auto &paths:fsnew::recursive_directory_iterator(lastCommit)){
			struct stat bufferLastCommit;
			if(stat(paths.path().c_str(),&bufferLastCommit)!=0) {cout<<"some path does nor exist\n"; return;}
			string str=paths.path(); str=str.substr(lastCommit.length());
			str=root+str;
			struct stat checkStr;
			if(stat(str.c_str(),&checkStr)!=0) continue;
			if(bufferLastCommit.st_mode & S_IFDIR) addCommit(paths.path(),'d',commitHash);
			else if(bufferLastCommit.st_mode & S_IFREG) addCommit(paths.path(),'f',commitHash);
		}}
}
	fsnew::remove_all(addPath.c_str());
	mkdir(addPath.c_str(),0777);
	addPath=addPath.substr(0,addPath.find_last_of("/")); addPath+="/add.log";
	remove(addPath.c_str())!=0;
	ofstream addLog(addPath.c_str());
	addLog.close();
	updateCommitLog(commitHash,message);
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
	else if(strcmp(argv[1],"commit")==0){
		if(argc<4) {cout<<"invalid commit\n"; return 0;}
		commit(argv,argc);
	}
	return 0;
}