#include <fstream>
#include <vector>
#include <iostream>
#include <cstring>
#include <utility>
#include <sys/types.h>
#include <sys/stat.h>
#include <experimental/filesystem>

using namespace std;
using namespace std::experimental::filesystem;

string root = "";
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

//add function

void addtoCache(const path &currentPath)
{
    int len = root.length();

    string relativePathString = currentPath.string().substr(len);
    
    path relativePath(relativePathString);

    path rootPath(root);
    path rootPathInUserDirectory(root);

    rootPath /= ".imperium";
    rootPath /= ".add";

    for (const auto &cur : relativePath)
    {
        if (cur == "/")
            continue;
        rootPath /= cur;
        rootPathInUserDirectory /= cur;

        
        if (is_directory(rootPathInUserDirectory))
        {
            
            if (!exists(rootPath))
            {
                create_directory(rootPath.string().c_str());
            }
        }
        else
        {
            
            const auto copyOptions = copy_options::update_existing;
            if (!exists(rootPath))
            {
                cout << currentPath << " Initialized.\n";
            }
            else
            {
                cout << currentPath << " Updated.\n";
            }
            copy_file(currentPath.string(), rootPath.string(), copyOptions);
        }
    }

    
}

bool searchString(ifstream &inputFile, const string &str)
{

   
    string line;
    while (getline(inputFile, line))
    {
        if (line.find(str) != string::npos)
            return true;
    }
    return false;
}

void toBeAdded(const path &currentPath)
{

    string addLogPath = root + "/.imperium/addlog";
    ifstream addLogFile(addLogPath.c_str());

    if (!searchString(addLogFile, currentPath.u8string()))
    {
        
        ofstream fout(addLogPath.c_str(), ios::app);
        fout << currentPath << "\n";
        fout.close();
    }
    else
    {
        
    }
    addLogFile.close();
}

bool toBeIgnored(const path &currentPath)
{

    string ignorePath = root + "/.imperiumIgnore";

    string currentPathString = currentPath.string();
    string line;
    ifstream inputFile(ignorePath);

    while (getline(inputFile, line))
    {
        string Directory = root + line;
        if (line.back() == '/')
        {
            Directory.pop_back();
            if (currentPathString.find(Directory) != string::npos)
            {
                
                return true;
            }
        }
        else
        {
            
            if (currentPathString == Directory)
                return true;
        }
    }
    inputFile.close();
    return false;
}
void addUtil(const path &currentPath)
{

    string imperiumFolder = root + "/.imperium";
    string addFolder = imperiumFolder + "/.add";

    if (!toBeIgnored(currentPath))
    {
        toBeAdded(currentPath);

        
        mkdir(addFolder.c_str(), 0777);
        addtoCache(currentPath);
    }
    
}

void add(vector<string> &argv)
{

    string imperiumFolder = root + "/.imperium";
    string addFolder = imperiumFolder + "/.add";
    string addLogPath = imperiumFolder + "/addlog";
    string ignorePath = imperiumFolder + "/ignorelog";

    if (!exists(addLogPath))
    {
        cout << "Please initialize the repository first.\n";
        return;
    }

    for (auto &str : argv)
    {

        if (str == ".")
        {
            str = "";
        }
        path currentPath(root);
        currentPath /= str;
        if (!exists(currentPath))
        {
            cout << "Sorry the file: " << currentPath << " dosen't exist.\n";
            continue;
        }

        if (is_regular_file(currentPath))
        {
            addUtil(currentPath);
            continue;
        }
        
        for (auto &p : recursive_directory_iterator(currentPath))
        {
           
            addUtil(p.path());
        }
    }
}

int main(int argc, char *argv[])
{
    root = getenv("dir");
 if (argc == 1)
        std::cout << "Hello Welcome to my world of Imperium";
    else if (!strcmp(argv[1], "init"))
    {
        init(root);
    }
    else if (!strcmp(argv[1], "add"))
    {
        
        vector<string> Args;
        for (int i = 2; i < argc; i++)
        {
            Args.push_back(argv[i]);
        }
        add(Args);
    }
	 else if (strcmp(argv[1], "--help")==0)
            std::cout<<"Help is always provided just look for it.May the source be with you.";
    return 0;
}
