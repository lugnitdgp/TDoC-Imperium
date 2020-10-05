#include <bits/stdc++.h>
#include <utility>
#include <fstream>
#include <iostream>
#include <string>
#include <string.h>
#include <sys/stat.h>
#include <experimental/filesystem>


namespace fs = std::experimental::filesystem;

std::string root = "";

void init(std::string path)
{
	struct stat buffer;
	
	if(stat((path+"/.imperium").c_str(),&buffer)==0)
	{
       std::cout<<"Repository has already been initialised\n";
	}
	
	else
	{
		std::string ignorepath = path + "/.imperiumignore";
	    std::ofstream ignore(ignorepath.c_str());
	    
		ignore<<"/.gitignore\n.imperium/\n.git/\n/.imperiumignore\n.node_modules/\n";
		ignore.close();
		
        path+="/.imperium";
	    int created = mkdir(path.c_str(),0777);
	
	    if(created==0)
	    {
           std::string commitlog = path + "/commitlog";
	       std::ofstream commit(commitlog.c_str());
	       commit.close();
		
	       std::string addlog = path + "/addlog";
	       std::ofstream add(addlog.c_str());
	       add.close();
		
	       std::string conflictlog = path + "/conflict";
	       std::ofstream conflict(conflictlog.c_str());
	       conflict<<"false\n";
	       conflict.close();
		
	       std::cout<<"initialised and empty repository\n";
	   
	    }
	    else
	    {
           std::cout<<"Error\n";
	    }
	} 
	
}

bool ignoreFolder(std::string path, std::vector<std::string> dirname)
{
    for(auto dir : dirname)
	{
        dir.pop_back();
		if(path.find(dir)!= std::string::npos)
			return true;
	}
	return false;
}

void addToCache(std::string path, char type)
{
	struct stat buffer;
	if(stat((root + "/.imperium/.add").c_str(), &buffer) !=0)
		mkdir((root + "/.imperium/.add").c_str(), 0777);
	
	if(type == 'f')
	{
		std::string filename = path.substr(root.length());
		std::string filerel = root + "/.imperium/.add" + filename.substr(0,filename.find_last_of("/"));
		
		struct stat buffer2;
		if(stat(filerel.c_str(), &buffer2) != 0)
			fs::create_directories(filerel);
		fs::copy_file(path, root + "/.imperium/.add"+filename, fs::copy_options::overwrite_existing);
		
	}
	else if(type == 'd')
	{
		std::string filename = path.substr(root.length());
		std::string filerel = root +"/.imperium/.add" + filename;
		
		struct stat buffer3;
		if(stat(filerel.c_str(), &buffer3) !=0 )
		{
			fs::create_directories(filerel);
		}
	}
	   
}

bool toBeIgnored(std::string path, int onlyImperiumIgnore=0)
{
	std::ifstream addFile, ignoreFile;
	std::string file_or_dir;
	std::vector<std::string> filenames;
	std::vector<std::pair<std::string, char>> addedFileNames;
	std::vector<std::string> ignoreDir;
	
	ignoreFile.open(root + "/.imperiumignore");
	addFile.open(root + "/.imperium/addlog");
	
	if(ignoreFile.is_open())
	{
		while(!ignoreFile.eof())
		{
            std::getline(ignoreFile, file_or_dir);
			auto i = file_or_dir.end();
			
			i--;
			if(*i == '/')
			{
				ignoreDir.push_back(file_or_dir);
			}
			else
			{
				filenames.push_back(root+file_or_dir);
			}
		}
	}
	ignoreFile.close();
	
	if(onlyImperiumIgnore == 0)
	{
		if(addFile.is_open())
		{
			while(!addFile.eof())
			{
				std::getline(addFile, file_or_dir);
				if(file_or_dir.length() > 4)
				{
					addedFileNames.push_back(std::make_pair(file_or_dir.substr(file_or_dir.length()-4), file_or_dir.at(file_or_dir.length()-1)));
				}
				
			}
		}
		addFile.close();
		
		for(auto fileEntry : addedFileNames)
		{
			if(path.compare(fileEntry.first)==0)
			{
				addToCache(path, fileEntry.second);
				std::cout<< "Updated :" << path << std::endl;
				return true;
			}
		}
	}
	if(std::find(filenames.begin(),filenames.end(), path) !=filenames.end() or ignoreFolder(path, ignoreDir))
		return true;
	return false;
	
}

void add(char **argv)
{
    struct stat buffer;
	if(stat((root + "/.imperium").c_str(), &buffer)==0)
	{
         std::ofstream addFile;
		 addFile.open(root + "/.imperium/addlog", std::ios_base::app);
		 std::string path = root;
		 if(strcmp(argv[2], ".")!=0)
		 {
             path += "/";
			 path += argv[2];
         }
		 struct stat buffer2;
		 if(stat(path.c_str(), &buffer2)==0)
		 {
            if(buffer2.st_mode & S_IFDIR)
			{
				if(!toBeIgnored(path))
				{
                    addFile << "\"" << path << "\"" << "-d\n";
					addToCache(path, 'd');
					std::cout << "Added Directory" << "\"" << path << "\"" << "\n";
				}
				for(auto &p : fs::recursive_directory_iterator(path))
				{
					if(toBeIgnored(p.path()))
				         continue;
					struct stat buffer3;
					if(stat(p.path().c_str(), &buffer3)==0)
					{
						if(buffer3.st_mode & S_IFDIR)
						{
							addToCache(p.path(), 'd');
							addFile << p.path() << "-d\n";
							std::cout<< "Added Directory: " << "\"" << p.path() << "\"" << "\n";
						}
						else if(buffer3.st_mode & S_IFREG)
						{
							addToCache(p.path(), 'f');
							addFile << p.path() << "-f\n";
							std::cout << "Added File: " << "\"" << p.path() << "\"" << "\n";
						
						}
						else
						{
							continue;
						}
					}
				}
			}
			else if(buffer2.st_mode & S_IFREG)
			{
				if(!toBeIgnored(path))
				{
					addFile << "\"" << path << "\"" << "-f\n";
					addToCache(path, 'f');
					std::cout << "Added File: " << "\"" << path << "\"" << "\n";
				}
			}
			 else 
	        {
		          std::cout << "Invalid Path" << "\n";
	        }
		}
		
    }
	else 
	{
        std::cout<<"Repository has not been initialised yet."<<std::endl;
	}
}

int main(int argc, char **argv)
{
	const std::string dir = getenv("dir");
	root=dir;
	
		if(strcmp(argv[1],"init")==0)
		{
            init(root);
		}
		
		else if(strcmp(argv[1], "add")==0)
		{
            add(argv);
		}
	return 0;
}