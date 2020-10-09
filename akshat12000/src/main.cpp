#include <bits/stdc++.h>
#include<fstream> 
#include<utility>
#include<sys/stat.h>
#include<experimental/filesystem>
#include<filesystem>
#include<algorithm>
#include <openssl/sha.h>
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

string getTime()
{
    auto end=chrono::system_clock::now();
	time_t end_time=chrono::system_clock::to_time_t(end);
	string time = std::ctime(&end_time);
	return time;
}

string getcommitHash()
{
	string commitFilename=getTime();
	string commitHash="";
	
	char buf[3];
	int length=commitFilename.length();
	unsigned char hash[20];
	unsigned char *val=new unsigned char[length+1];
	strcpy((char*)val,commitFilename.c_str());
	
	SHA1(val,length,hash);
	for(int i=0;i<20;i++)
	{
		sprintf(buf,"%02x",hash[i]);
		commitHash+=buf;
	}
	return commitHash;
}

void getCommitlog()
{
    string commitlogpath=root+"/.imperium/commit.log";
	string commitLine;
	ifstream commitLog;
	commitLog.open(commitlogpath);
	while(getline(commitLog,commitLine))
	{
		cout<<commitLine<<"\n";
	}
}

void repeatCommit(string abspath,char type,string commitHash)
{
	
	mkdir((root+"/.imperium/.commit/"+commitHash).c_str(),0777);
	string relpath= abspath.substr(root.length()+19+commitHash.length());
	string filepath= root+"/.imperium/.commit/"+commitHash+relpath.substr(0,relpath.find_last_of('/'));
	
	fs::create_directories(filepath);
	if(type=='f')
	{
		fs::copy_file(abspath,root+"/.imperium/.commit/"+commitHash+relpath,fs::copy_options::overwrite_existing);
	}

}

void addcommit(string abspath,char type,string commitHash)
{
	struct stat s;
	if(stat((root+"/.imperium/.commit").c_str(),&s)!=0)
	{
		mkdir((root+"/.imperium/.commit").c_str(),0777);
    }
	if(stat((root+"/.imperium/.commit/"+commitHash).c_str(),&s)!=0)
	{
		mkdir((root+"/.imperium/.commit/"+commitHash).c_str(),0777);
	}
	string relpath= abspath.substr(root.length()+15);
	string filepath= root+"/.imperium/.commit/"+commitHash+relpath.substr(0,relpath.find_last_of('/'));
	
	fs::create_directories(filepath);
	if(type=='f')
	{
		fs::copy_file(abspath,root+"/.imperium/.commit/"+commitHash+relpath,fs::copy_options::overwrite_existing);
	}

}

void updateCommitLog(string commitHash,string message)
{
	ofstream writeHeadlog;
	writeHeadlog.open(root+"/.imperium/head.log");
	writeHeadlog<<commitHash<<" -- "<<message<<endl;
	writeHeadlog.close();
	
	ofstream writeCommitlog;
	ifstream readCommitlog;
	readCommitlog.open(root+"/.imperium/commit.log");
	writeCommitlog.open(root+"/.imperium/new_commit.log");
	
	writeCommitlog<<commitHash<<" -- "<<message<<" -->HEAD\n";
	
	string line;
	while(getline(readCommitlog,line))
	{
		if(line.find(" -->HEAD")!=string::npos)
		{
			writeCommitlog<<line.substr(0,line.length()-8)<<"\n";
		}
		else
		{
			writeCommitlog<<line<<"\n";
		}
	}
	remove((root+"/.imperium/commit.log").c_str());
	rename((root+"/.imperium/new_commit.log").c_str(),(root+"/.imperium/commit.log").c_str());
	readCommitlog.close();
	writeCommitlog.close();
}

void commit(string message)
{
	  struct stat buffer;
	if(stat((root+"/.imperium").c_str(),&buffer)!=0)
	{
		cout<<"Repository has not been initialized\n";
	}
	string commitHash=getcommitHash();
	
	if(stat((root+"/.imperium/head.log").c_str(),&buffer)==0)
	{
		string headHash;
		ifstream readCommitlog;
		readCommitlog.open(root+"/.imperium/head.log");
		getline(readCommitlog,headHash);
		headHash=headHash.substr(0,headHash.find(" -- "));
		
		for(auto &p: fs::recursive_directory_iterator(root+"/.imperium/.commit/"+headHash))
		{
			if(stat(p.path().c_str(),&buffer)==0)
			{
				if(buffer.st_mode & S_IFREG)
				{
					repeatCommit(p.path(),'f',commitHash);
                }
				else if(buffer.st_mode & S_IFDIR)
				{
					repeatCommit(p.path(),'d',commitHash);
				}
				else
					continue;
			}
		}
	}
	for(auto &p: fs::recursive_directory_iterator(root+"/.imperium/.add"))
	{
		struct stat s;
		if(stat(p.path().c_str(),&s)==0)
		{
			if(s.st_mode & S_IFREG)
			{
				addcommit(p.path(),'f',commitHash);
			}
			if(s.st_mode & S_IFDIR)
			{
				addcommit(p.path(),'d',commitHash);
			}
		}
	}
	fs::remove_all((root+"/.imperium/.add").c_str());
	remove((root+"/.imperium/add.log").c_str());
	updateCommitLog(commitHash,message);
}

int BUFFER_SIZE;

int compareFiles(string path1,string path2)
{
	ifstream lfile(path1.c_str(),ifstream::in | ifstream::binary);
	ifstream rfile(path2.c_str(),ifstream::in | ifstream::binary);
	
	if(!lfile.is_open()||!rfile.is_open())
		return 1;
	
	char *lbuffer=new char[BUFFER_SIZE]();
	char *rbuffer=new char[BUFFER_SIZE]();
	
	do
	{
		lfile.read(lbuffer,BUFFER_SIZE);
		rfile.read(rbuffer,BUFFER_SIZE);
		int getnumber=lfile.gcount();
		if(getnumber!=rfile.gcount())
			return 1;
		if(memcmp(lbuffer,rbuffer,getnumber)!=0)
			return 1;
		
	}while(lfile.good()||rfile.good());
	
	delete[] lbuffer;
	delete[] rbuffer;
	return 0;
}

int conflictCheck()
{
	ifstream cfile;
	string line;
	cfile.open(root+"/.imperium/conflict");
	getline(cfile,line);
	cfile.close();
	if(line=="true")
		return 1;
	return 0;
}

void status()
{
	struct stat buf;
	ifstream readAddlog;
	string line,headHash;
	vector<string>staged;
	vector<string>type;
	vector<string>notStaged;
	ifstream readCommitlog;
	string stagedPath;
	
	readCommitlog.open(root+"/.imperium/commit.log");
	getline(readCommitlog,headHash);
	
	headHash=headHash.substr(0,40);
	readCommitlog.close();
	
	if(stat((root+"/.imperium/.add").c_str(),&buf)==0)
	{
		readAddlog.open(root+"/.imperium/add.log");
		
		while(getline(readAddlog,line))
		{
			stagedPath=line.substr(root.length()+1,line.length()-root.length()-4);
			if(stat((root+"/.imperium/.add/"+stagedPath).c_str(),&buf)==0)
			{
				if((stat((root+"/.imperium/.commit"+headHash+stagedPath).c_str(),&buf)!=0)||stat((root+"/.imperium/.commit").c_str(),&buf)!=0)
				{
					type.push_back("created: ");
					staged.push_back(stagedPath);
				}
				if(find(staged.begin(),staged.end(),stagedPath)==staged.end())
				{
					notStaged.push_back("deleted: "+stagedPath);
				}
				else{
					if(compareFiles(root+stagedPath,root+"/.imperium/.add/"+stagedPath))
					{
						notStaged.push_back("modified: "+stagedPath);
					}
				}
			}
		}
	}
	readAddlog.close();
    struct stat s;
	if(stat((root+"/.imperium/.commit").c_str(),&buf)==0)
	{
		for(auto &p: fs::recursive_directory_iterator(root))
		{
			if(stat(p.path().c_str(),&s)==0)
			{
				string rootPath=p.path();
				
				if(toBeIgnored(p.path().c_str(),1))
					continue;
				
				string commitPath=root+"/.imperium/.commit"+headHash+rootPath.substr(root.length());
				
				if(stat(commitPath.c_str(),&s)!=0 && (find(staged.begin(),staged.end(),rootPath.substr(root.length()))==staged.end()))
				{
					for(int i=0;i<staged.size();i++)
						cout<<staged[i]<<"\n";
					
					notStaged.push_back("created: "+rootPath.substr(root.length()));
				}
				else
				{
					if(compareFiles(rootPath,commitPath))
					{
						if(find(staged.begin(),staged.end(),rootPath.substr(root.length()))==staged.end() && find(notStaged.begin(),notStaged.end(),commitPath)==notStaged.end())
						{
							notStaged.push_back("modified: "+root.substr(root.length()));
						}
					}
				}
			}
			else
				continue;
		}
	}
	if(stat((root+"/.imperium/.commit").c_str(),&buf)!=0)
	{
	   for(auto &p: fs::recursive_directory_iterator(root+"/.imperium/.commit"+headHash))
	   {
		   struct stat buffer;
		   if(stat(p.path().c_str(),&buffer)==0)
		   {
			   if(buffer.st_mode & S_IFREG)
			   {
				   string commitPath=p.path();
				   string rootPath= root+commitPath.substr(root.length()+59);
				   
				   if(toBeIgnored(root.c_str(),1))
					   continue;
				   if (stat(rootPath.c_str(),&buffer)!=0)
				   {
					   notStaged.push_back("deleted: "+rootPath.substr(root.length()));
				   }
			   }
			   else
				   continue;
		   }
	   }
	}
	if(notStaged.size())
		cout<<"Changes not staged for commit: \n\n";
	for(int i=0;i<notStaged.size();i++)
		cout<<notStaged[i]<<"\n";
	if(staged.size())
		cout<<"/nChanges staged for commit: \n\n";
	for(int i=0;i<staged.size();i++)
		cout<<type[i]<<staged[i]<<"\n";
	if(!notStaged.size() && !staged.size())
		cout<<"Up to date\n";
	staged.clear();
	notStaged.clear();
}

void checkout(string commitHash)
{
	if(commitHash=="")
	{
		cout<<"Please provide a hash\n";
		return;
	}
	bool hashCheck=false;
	ifstream commitLog;
	string commitLine;
	commitLog.open(root+"/.imperium/commit.log");
	
	while(getline(commitLog,commitLine)){
		
		if(commitHash==commitLine.substr(0,40)){
			hashCheck=true;
			break;
		}
	}
	commitLog.close();
	if(hashCheck==false)
	{
		cout<<"Incorrect Hash provided\n";
		return;
	}
	
	string commitFolderPath=root+"/.imperium/.commit"+commitHash;
	fs::copy(commitFolderPath,root+"/",fs::copy_options::recursive|fs::copy_options::overwrite_existing);
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
		commit(argv[2]);
	}
	else if(strcmp(argv[1],"log")==0){
		getCommitlog();
	}
	else if(strcmp(argv[1],"status")==0){
		status();
	}
	else if(strcmp(argv[1],"checkout")==0){
		checkout(argv[2]);
	}
	return 0;
}
