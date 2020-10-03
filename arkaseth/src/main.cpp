#include <iostream>
#include <utility>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <experimental/filesystem>

using namespace std;
namespace fs = experimental::filesystem;

string root = "";

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
		ignorePath << ".imperiumignore\n.gitignore\nnode_modules\n.git\n.env";
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

			cout << "Initialised the repo!\n";
		}
		else
		{
			cout << "Error!\n";
		}
	}
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
}