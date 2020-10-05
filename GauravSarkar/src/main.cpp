#include <bits/stdc++.h>
#include <fstream>
#include <string>
#include <string.h>
#include <sys/stat.h>
#include <experimental/filesystem>


namespace fs = std::experimental::filesystem;
std::string root= "";
void init(std::string path)
{
	struct stat buffer;
	if(stat((path+"/.imperium").c_str(),&buffer)==0){
	   std::cout<<"Repository has already been initialized\n";}
	   else
	   { 
	
	std::string ignorepath = path + "/.imperiumignore";
	std::ofstream ignore(ignorepath.c_str());
		   ignore<< ".gitignore\n.imperium\n.git\n.imperiumignore\n.node_modules\n";
		   ignore.close();
	path+="/.imperium";
	int created = mkdir(path.c_str(),0777);
	
	if(created==0)
		{std::string commitlog = path +"/commitlog";
		std::ofstream commit(commitlog.c_str());
	 commit.close();
		
		 std::string addlog = path +"/addlog";
		std::ofstream add(addlog.c_str());
	 add.close();
		  std::string conflictlog = path +"/conflict";
		std::ofstream conflict(conflictlog.c_str());
		 conflict<<"False\n";
		 conflict.close();
		 std::cout<<"Initialized an empty repository\n";
     }
	else
	{
		std::cout<<"ERROR\n";
	}
}
}

void add(std::string path)
{
	//add functionality
	
}
int main(int argc, char const *argv[])
{
    
    const char *Directory = getenv("dir");

    if (argc == 1)
        std::cout << "Hello Welcome to my world of Imperium";
    else if (argc == 2)
    {
        if (strcmp(argv[1], "init")==0)
            init(Directory);
		f (strcmp(argv[1], "add")==0)
            add(Directory);
        else if (strcmp(argv[1], "--help")==0)
            std::cout<<"Help is always provided just look for it.May the source be with you.";
    }
    return 0;
}