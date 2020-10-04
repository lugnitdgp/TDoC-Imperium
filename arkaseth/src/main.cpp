#include <iostream>
#include <utility>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <experimental/filesystem>

using namespace std;
namespace fs = experimental::filesystem;

string root = "";

int ignoreIt(string, string);
int toIgnore(string, string);
void add(string, string);
void init(string);
void copyFile(string, string, string);

// 1. Init
void init(string path)
{
	struct stat info;

	if (stat((path + "/.imperium").c_str(), &info) == 0)
	{
		cout << "Repo is already initialized!\n";
	}
	else
	{
		string ignore = path + "/.imperiumignore";
		ofstream ignorePath(ignore.c_str());
		ignorePath << "/.imperiumignore\n/.gitignore\n/node_modules\n/.git\n/.env";
		ignorePath.close();

		path += "/.imperium";
		int createFlag = mkdir(path.c_str(), 0777);
		if (createFlag == 0)
		{
			string commitLog = path + "/commit.log";
			ofstream commit(commitLog.c_str());
			commit.close();

			string addLog = path + "/add.log";
			ofstream add(addLog.c_str());
			add.close();

			string conflictLog = path + "/conflict.log";
			ofstream conflict(conflictLog.c_str());
			conflict.close();

			string addFolder = path + "/.add";
			mkdir(addFolder.c_str(), 0777);

			cout << "Initialised the repo!\n";
		}
		else
		{
			cout << "Error!\n";
		}
	}
}

// 2. Add
void add(string path, string subPath = "")
{
	struct stat info;

	string addPath;
	string copyTo = path + "/.imperium/.add";
	string addLog = path + "/.imperium/add.log";
	if (strcmp(subPath.c_str(), ".") == 0)
	{
		addPath = path;
	}
	else
	{
		addPath = path + "/" + subPath;
	}
	if (stat(addPath.c_str(), &info) != 0)
	{
		cout << "Check the path you have entered! This path does not exist!\n";
	}
	else
	{
		// If it's a file
		if (info.st_mode & S_IFREG)
		{
			// If in .imperiumignore
			if (toIgnore(path, addPath) == 2)
			{
				cout << "File in .imperiumignore!" << endl;
			}
			// If neither in add.log nor .imperiumignore
			else if (toIgnore(path, addPath) == 0)
			{
				ofstream outFile;
				outFile.open(addLog.c_str(), ios_base::app);
				outFile << addPath << endl;
				outFile.close();
				copyFile(path, addPath, copyTo);
			}
		}
		// If it's a directory
		else if (info.st_mode & S_IFDIR)
		{
			struct stat buf;
			for (auto i = fs::recursive_directory_iterator(addPath); i != fs::recursive_directory_iterator(); ++i)
			{
				if ((stat(i->path().string().c_str(), &buf) == 0) && (toIgnore(path, i->path().string()) == 2))
				{
					i.disable_recursion_pending();
				}
				else if ((stat(i->path().string().c_str(), &buf) == 0) && (toIgnore(path, i->path().string()) == 0))
				{
					if ((stat(i->path().string().c_str(), &buf) == 0) && buf.st_mode & S_IFREG)
					{
						if (toIgnore(path, i->path().string()) == 0)
						{
							ofstream outFile;
							outFile.open(addLog.c_str(), ios_base::app);
							outFile << i->path().string() << endl;
							outFile.close();
							copyFile(path, i->path().string(), copyTo);
						}
					}
				}
			}
		}
	}
}

// path to ignore
int toIgnore(string path, string paramPath)
{
	string ignorePath = path + "/.imperiumignore";
	string addLog = path + "/.imperium/add.log";
	string line;
	int offset;

	string paramInIgnore = paramPath;
	string::size_type i = paramInIgnore.find(path);
	if (i != string::npos)
		paramInIgnore.erase(i, path.length());
	// If path is in .imperiumignore
	if (ignoreIt(ignorePath, paramInIgnore))
	{
		return 2;
	}
	// Check if already in add.log
	ifstream ifs(addLog.c_str());
	if (ifs.is_open())
	{
		while (!ifs.eof())
		{
			getline(ifs, line);
			if ((offset = line.find(paramPath, 0)) != string::npos)
			{
				return 1;
			}
		}
	}
	// If in neither of the two files
	return 0;
}

// Is it in .imperiumignore?
int ignoreIt(string ignorePath, string paramInIgnore)
{
	string line;
	int offset;
	ifstream ignore(ignorePath.c_str());
	if (ignore.is_open())
	{
		while (!ignore.eof())
		{
			getline(ignore, line);
			if ((offset = line.find(paramInIgnore, 0)) != string::npos)
			{
				return 1;
			}
		}
	}
	return 0;
}

void copyFile(string path, string filePath, string addFolder)
{
	string filePathCopy = filePath;
	string::size_type i = filePathCopy.find(path);
	if (i != string::npos)
		filePathCopy.erase(i, path.length());
	// addFolder += filePathCopy;
	// cout << addFolder << endl
	// 	 << filePath << endl
	// 	 << filePathCopy << endl;

	vector<string> tokenizedFolders;
	char *filePathCopyC = &filePathCopy[0];

	char *p = strtok(filePathCopyC, "/");
	while (p)
	{
		tokenizedFolders.push_back(p);
		p = strtok(NULL, "/");
	}

	string addFolderCopy = addFolder;

	for (int i = 0; i < tokenizedFolders.size() - 1; i++)
	{
		mkdir((addFolderCopy + "/" + tokenizedFolders[i]).c_str(), 0777);
		addFolderCopy += "/" + tokenizedFolders[i];
	}
	addFolderCopy += "/" + tokenizedFolders[tokenizedFolders.size() - 1];
	const auto copyOptions = fs::copy_options::overwrite_existing;
	fs::copy(filePath, addFolderCopy, copyOptions);
}

int main(int argc, char *argv[])
{
	const string dir = getenv("dir");
	root = dir;
	if (argc == 1)
	{
		cout << "Hey!\n";
	}
	else if (strcmp(argv[1], "init") == 0)
	{
		init(root);
	}
	else if (strcmp(argv[1], "add") == 0)
	{
		add(root, argv[2]);
	}
}