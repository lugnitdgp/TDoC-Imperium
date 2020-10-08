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
			if(file_or_dir.back()=='/'){ file_or_dir.pop_back(); ignoreDir.push_back(root+"/"+file_or_dir);}
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
	bool addEmpty=1;
	for(auto &absPath:fsnew::recursive_directory_iterator(addPath))
	{
		addEmpty=0;
		struct stat buffer;
		if(stat(absPath.path().c_str(),&buffer)!=0) {cout<<"ERROR path doesnot exist\n"; return;}
		char type;
		if(buffer.st_mode & S_IFDIR) type='d';
		else if(buffer.st_mode & S_IFREG) type='f';
		else continue;
		addCommit(absPath.path(),type,commitHash);
	}
	string lastCommit = getHead();
	if(lastCommit!="ncf")
		{
			lastCommit=root+"/.imperium/.commit/"+lastCommit;
		for(auto &paths:fsnew::recursive_directory_iterator(lastCommit)){
			addEmpty=0;
			struct stat bufferLastCommit;
			if(stat(paths.path().c_str(),&bufferLastCommit)!=0) {cout<<"some path does nor exist\n"; return;}
			string str=paths.path(); str=str.substr(lastCommit.length());
			str=root+str;
			struct stat checkStr;
			if(stat(str.c_str(),&checkStr)!=0) continue;
			if(bufferLastCommit.st_mode & S_IFDIR) addCommit(paths.path(),'d',commitHash);
			else if(bufferLastCommit.st_mode & S_IFREG) addCommit(paths.path(),'f',commitHash);
		}}
	if(addEmpty) {cout<<"Nothing to commit\n"; return;}
	fsnew::remove_all(addPath.c_str());
	mkdir(addPath.c_str(),0777);
	addPath=addPath.substr(0,addPath.find_last_of("/")); addPath+="/add.log";
	remove(addPath.c_str())!=0;
	ofstream addLog(addPath.c_str());
	addLog.close();
	if(!addEmpty) updateCommitLog(message,commitHash);
}
void log(){
	string path = root+"/.imperium/commit.log";
	struct stat buffer;
	if(stat(path.c_str(),&buffer)) {cout<<"Commit log not created yet\n"; return;}
	ifstream commitLog;
	commitLog.open(path.c_str());
	if(commitLog.is_open()){
		string line;
		while(!commitLog.eof()){
			getline(commitLog,line);
			cout<<line<<endl;
}
	}
	else cout<<"Couldnot open commit log\n";
	commitLog.close();
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
	else if(strcmp(argv[1],"log")==0){
		log();
	}
	else if(strcmp(argv[1],"checkout")==0){
		if(argc<3) {cout<<"Please enter a commit hash\n"; return 0;}
		string commitHash=argv[2];
		checkout(commitHash);
	}
	else if(strcmp(argv[1],"status")==0){
		status();
	}
	return 0;
}
