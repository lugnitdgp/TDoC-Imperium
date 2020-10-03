#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <fstream>
using namespace std;

string root = "";

void init(const char* p)
{
	char path[PATH_MAX];
	strcpy(path, p);
	strcat(path,"/.imperium/");
	struct stat buf;
	if(stat(path, &buf)==0 && S_ISDIR(buf.st_mode))
	{
		cout<<"Reinitialized existing Imperium repository in "<<path<<"\n";
		return;
	}
	else
	{
		mkdir(path,0777);
		ofstream fout;
		fout.open(root+"/.imperiumignore");
		fout<<"/node_modules\n";
		fout<<"/.env\n";
		fout.close();
		chdir(path);
		fout.open("conflict");
		fout<<"false\n";
		fout.close();
		fout.open("add.log");
		fout.close();
		fout.open("commit.log");
		fout.close();
		cout<<"Initialized empty Imperium repository in "<<path<<"\n";
		return;
	}
}

int main(int argc, char* argv[]) {
	const char* path = getenv("dir");
	root = path;
	if(strcmp(argv[1],"init")==0)
	init(path);
	return 0;
}