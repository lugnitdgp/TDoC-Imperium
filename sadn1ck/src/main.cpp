#include <bits/stdc++.h>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

#define INIT "init"

/// Displaying help if no commands are given
void displayHelp() {
  std::cout << "usage: imperium <command> [<args>]\n\n";
  std::cout << "Start a working area\n";
  std::cout
      << "\tinit\t\tInitialise an empty repository inside the current folder\n";
  std::cout << "\n";
}

// https://stackoverflow.com/q/18100097
/// check if directory exists
bool dirExists(const char *path) {
  // @TODO: Add error checks
  struct stat statcheck;
  return stat(path, &statcheck) == 0 && S_ISDIR(statcheck.st_mode);
}

/// create directory at path
void createDir(const char *path) {
  // @TODO: Add error checks
  // std::cout << "Inside createDir: " << path << "\n";
  mkdir(path, 0777);
}

/// imperium init function
void init(const char *path) {
  // @TODO: Add error checks
  // std::cout << "Inside init: " << path << "\n";
  if (dirExists(path)) {
    std::cout << "Existing imperium repository\n";
  } else {
    createDir(path);
    std::ofstream customFileWriter;
    customFileWriter.open((path) + std::string("/conflict"));
    customFileWriter << "false\n";
    customFileWriter.close();
    customFileWriter.open((path) + std::string("/commit.log"));
    customFileWriter << "\n";
    customFileWriter.close();
    customFileWriter.open((path) + std::string("/add.log"));
    customFileWriter << "\n";
    customFileWriter.close();
    std::cout << "Initialised empty imperium repository at " << path << "\n";
  }
}

int main(int argc, char **argv) {
  if (argc <= 1) {
    displayHelp();
  } else {
    // @TODO: check if there are 2 or more arguments
    // $ imperium <command> [<args>]
    // branch from <command>
    // std::cout << argv[1] << "\n";
    if (strcmp(argv[1], INIT) == 0) {
      if (argc == 2) {
        const char *path = strcat(std::getenv("PWD"), "/.imperium");
        init(path);
      }
      if (argc == 3) {
        // @TODO: Add functionality for creating folder along with this
      }
    }
  }
  return 0;
}