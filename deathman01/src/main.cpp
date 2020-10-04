#include <bits/stdc++.h>
#include <utility>
#include <fstream>
#include <string>
#include <string.h>
#include <sys/stat.h>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

std::string root = "";

void init(std::string path){
	
	struct stat buffer;
	if(stat((path+"/.imperium").c_str() , &buffer)==0){
		std::cout << "Reprodotery has already been Initiliazed \n";
	}else{
		std::string ignorepath = path + "./imperiumignore";
		std::ofstream ignore( ignorepath.c_str() );
		
		ignore << ".gitignore\n.imperium\n.git\n.imperiumignore\n.node_modules\n";
		ignore.close();
		
		path += "/.ipmerium";
		int created = mkdir( path.c_str() , 0777);
		if(created == 0){
			std::string commitlog = path+"/comitlog";
			std::ofstream commit( commitlog.c_str());
			commit.close();

			std::string addlog = path+"/addlog";
			std::ofstream add( commitlog.c_str());
			add.close();

			std::string conflictlog = path+"/conflict";
			std::ofstream conflict( conflictlog.c_str());
			conflict << "False";
			conflict.close();

			std::cout << " Initilized an empty reprosotory\n";

		}else{
			std::cout<<"ERROR\n";
		}
	}
}

int main(int argc , char **argv) {
		// your code goes here
		const std::string dir = getenv( "dir" );
		root = dir;
		if( strcmp(argv[1] , "init") == 0){
			init( root );
		}
	return 0;
}

