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
            string commitlog= path + "/commitlog";
            ofstream commit(commitlog.c_str());
            commit.close();

            string addlog= path + "/addlog";
            ofstream add(addlog.c_str());
            add.close();
            
            string conflictlog= path + "/conflictlog";
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

bool ignorefolder( string path, vector<string> dirname)
{
    for(auto dir: dirname)
    {
        dir.pop_back();
        if(dir.find(path) != string::npos)
            return true;
    }
    return false;
}

void addtocache( string path, char type)
{
    struct stat buffer;
    if(stat((root+"/.imperium/.add").c_str(), &buffer) !=0)
        mkdir(((root+"/.imperium/.add").c_str()), 0777);

    if(type== 'f')
    {
        string filename = path.substr(root.length());
        string filerel = root+ "/.imperium/.add" + filename.substr(0, filename.find_last_of("/"));

        struct stat buf;
        if(stat(filerel.c_str(), &buf) != 0)
        {
            fs::create_directories(filerel);
        }
        fs::copy_file(path, root+"/.imperium/.add" + filename,fs::copy_options::overwrite_existing);
    }
    else if(type=='d')
    {
        string filename = path.substr(root.length());
        string filerel = root+ "/.imperium/.add" + filename;

        struct stat buf1;
        if(stat(filerel.c_str(), &buf1) != 0)
        {
            fs::create_directories(filerel);
        }
    }   
}

bool tobeignored(string path, int onlyimperiumignore=0)
{
    ifstream ignore, add;
    ignore.open((root+"/.imperiumignore").c_str());
    add.open((root+"/.imperium/addlog").c_str());
    string file_or_dir;
    vector<string> filename;
    vector<pair<string, char>> addfilename;
    vector<string> ignoredir;

    if(ignore.is_open())
    {
        while(!ignore.eof())
        {
            getline(ignore, file_or_dir);
            auto i = file_or_dir.end();
            i--;            

            if(*i == '/')
            {
                ignoredir.push_back(file_or_dir);
            }
            else
            {
                filename.push_back(root+file_or_dir);
            }
            
        }
    }
    ignore.close();

    if(onlyimperiumignore==0)
    {
        if(add.is_open())
        {
            while(!add.eof())
            {
                getline(add, file_or_dir);
                if(file_or_dir.length()>4 )
                {
                    addfilename.push_back(make_pair(file_or_dir.substr(file_or_dir.length()-4), file_or_dir.at(file_or_dir.length()-1)));
                }
            }
        }
        add.close();

        for(auto file: addfilename)
        {
            if(path.compare(file.first)==0)
            {
                addtocache(path, file.second);
                cout<<"Updated: "<<path << endl;
                return true;
            }
        }
    }
    if(find(filename.begin(), filename.end(), path) != filename.end() or ignorefolder(path, ignoredir))
        return true;
    return false;
}

void add(string path, string arg1){
    struct stat sb;
    
    if(stat((root+"/.imperium").c_str(), &sb)==0)
    {
        ofstream add;
        add.open((root+"/.imperium/addlog").c_str(), std::ios_base::app);
        string addpath = root;
        if(strcmp(arg1.c_str() , ".")!=0){
            addpath+=("/"+arg1);
        }
        
        //using recursive_directory_iterator = fs::recursive_directory_iterator;
        struct stat buffer;
        if (stat(addpath.c_str(), &buffer) == 0 ) 
        {
            if(buffer.st_mode & S_IFDIR)
            {
                if(!tobeignored(addpath))
                {
                    add<<"\""<<addpath<<"\""<<"-d\n";
                    addtocache(path, 'd');
                    cout<<"Added directory"<<"\""<<addpath<<"\""<<"\n";
                }
            
                for (auto& p: fs::recursive_directory_iterator(addpath)){
                    if(tobeignored(p.path()))
                        continue;
                    struct stat sb1;
                    if(stat(p.path().c_str(), &sb1)==0)
                    {
                        if(sb1.st_mode & S_IFDIR)
                        {
                            addtocache(p.path(), 'd');
                            add<<p.path()<<"-d\n";
                            cout<<"Added directory"<<"\""<<p.path()<<"\""<<"\n";
                        }
                        else if(sb1.st_mode & S_IFREG)
                        {
                            addtocache(p.path(), 'f');
                            add<<p.path()<<"-f\n";
                            cout<<"Added file"<<"\""<<p.path()<<"\""<<"\n";
                        }
                        else
                        {
                            continue;
                        }
                    }
                }  
            } 
            else if(buffer.st_mode & S_IFREG)
            {
                if(!tobeignored(addpath))
                {
                    add<<"\""<<path<<"\""<<"-f\n";
                    addtocache(path, 'f');
                    cout<<"Added file: "<<"\""<<path<<"\""<<"\n";
                }
            }
            else
            {
                cout<<"invalid path!"<<"\n";
            }
        }
        add.close();
    }
    else
    {
        cout<<"Repository has not been initialised!\n";
    }
    
}

int main(int argc, char* argv[])
{
    const string dir = getenv("dir");
    root=dir;
    if(strcmp(argv[1],"init")==0)
        init(root);
    if(strcmp(argv[1],"add")==0)
        add(root, argv[2]);
    return 0;
}
