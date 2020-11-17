#include <bits/stdc++.h>
#include <cstdlib>
#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <type_traits>

#define INIT "init"
#define ADD "add"

namespace fs = std::experimental::filesystem;

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

/// custom function to write to files
void writeFile(std::string writePath, std::string fileContent) {
  std::ofstream customFileWriter;
  customFileWriter.open(writePath);
  customFileWriter << fileContent;
  customFileWriter.close();
}

/// imperium init function
void init(const char *path) {
  // TODO: Add error checks
  if (dirExists(path)) {
    std::cout << "Existing imperium repository\n";
  } else {
    createDir(path);
    writeFile((path) + std::string("/conflict"), "false\n");
    writeFile((path) + std::string("/commit.log"), "\n");
    writeFile((path) + std::string("/add.log"), "\n");

    std::cout << "Initialised empty imperium repository at " << path << "\n";
  }
}

/// imperium add function
void add(const char *path) {
  if (fs::exists(std::string(path)) == 1) {
    std::cout << "Path exists\n";
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
        std::string ignorePath = std::string(std::getenv("PWD"));
        ignorePath = ignorePath + std::string("/.imperiumIgnore");
        std::string fileContent =
            "node_modules\n.env\n.imperium\n.imperiumIgnore";
        writeFile(ignorePath, fileContent);
        const char *path = strcat(std::getenv("PWD"), "/.imperium");
        init(path);
      }
      if (argc == 3) {
        createDir(argv[2]);
        char *temp = strcat(std::getenv("PWD"), "/");
        temp = std::strcat(temp, argv[2]);
        // write .imperiumIgnore here
        std::string ignorePath = std::string(temp);
        ignorePath = ignorePath + std::string("/.imperiumIgnore");
        std::string fileContent = "node_modules\n.env\n";
        writeFile(ignorePath, fileContent);
        // written .imperiumIgnore here
        temp = std::strcat(temp, "/.imperium");
        const char *path = temp;
        init(path);
      }
    }
    if (strcmp(argv[1], ADD) == 0) {
      if (argc != 3) {
        std::cout << "What do you want me to add? Give me some arguments!\n";
        displayHelp();
      }
      if (argc >= 3) {
        if (strcmp(argv[2], ".") == 0) {
          const char *addPath = std::getenv("PWD");
          std::cout << addPath << "\n";
        }
      }
    }
  }
  return 0;
}