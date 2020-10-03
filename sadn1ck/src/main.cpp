#include <bits/stdc++.h>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <type_traits>

#define INIT "init"
#define ADD "add"

/// Displaying help if no commands are given
void displayHelp() {
  std::cout << "usage: imperium <command> [<args>]\n\n";
  std::cout << "Start a working area\n";
  std::cout
      << "\tinit\t\tInitialise an empty repository inside the current folder\n";
  std::cout << "\tadd\t\tAdd files to staging area\n";
  std::cout << "\n";
}

// https://stackoverflow.com/q/18100097
/// check if directory exists
bool dirExists(const char *path) {
  // TODO: Add error checks
  struct stat statcheck;
  return stat(path, &statcheck) == 0 && S_ISDIR(statcheck.st_mode);
}

/// create directory at path
void createDir(const char *path) {
  // TODO: Add error checks
  // std::cout << "Inside createDir: " << path << "\n";
  mkdir(path, 0777);
}

/// custom function to write .imperiumIgnore file
void writeIgnoreFile(const char *ignorePath) {
  std::ofstream customFileWriter;
  customFileWriter.open((ignorePath) + std::string("/.imperiumIgnore"));
  customFileWriter << "node_modules\n.env\n";
  customFileWriter.close();
}

/// imperium init function
void init(const char *path) {
  // TODO: Add error checks
  if (dirExists(path)) {
    std::cout << "Existing imperium repository\n";
  } else {
    createDir(path);
    std::ofstream customFileWriter;
    // TODO: create a custom function to handle file writes
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
    // $ imperium <command> [<args>]
    /// if init is called
    if (strcmp(argv[1], INIT) == 0) {
      if (argc == 2) {
        const char *ignorePath = std::getenv("PWD");
        writeIgnoreFile(ignorePath);
        const char *path = strcat(std::getenv("PWD"), "/.imperium");
        init(path);
      }
      if (argc == 3) {
        // TODO: Add functionality for creating folder along with this
        createDir(argv[2]);
        char *temp = strcat(std::getenv("PWD"), "/");
        temp = std::strcat(temp, argv[2]);
        // write .imperiumIgnore here
        const char *ignorePath = temp;
        writeIgnoreFile(ignorePath);
        // written .imperiumIgnore here
        temp = std::strcat(temp, "/.imperium");
        const char *path = temp;
        init(path);
      }
    }
    if (strcmp(argv[1], ADD) == 0) {
      std::cout << "Add called\n";
    }
  }
  return 0;
}