#include <bits/stdc++.h>
#include <cstdlib>
#include <experimental/bits/fs_fwd.h>
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
std::string root = "";

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
bool dirExists(std::string path) {
  // TODO: Add error checks
  struct stat statcheck;
  return stat(path.c_str(), &statcheck) == 0 && S_ISDIR(statcheck.st_mode);
}

/// create directory at path
void createDir(std::string path) {
  // TODO: Add error checks
  mkdir(path.c_str(), 0777);
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
    std::cout << path << "\n";
    createDir(path);
    createDir((path) + std::string("/.add"));
    createDir((path) + std::string("/.commit"));
    writeFile((path) + std::string("/conflict"), "false\n");
    writeFile((path) + std::string("/commit.log"), "\n");
    writeFile((path) + std::string("/add.log"), "\n");

    std::cout << "Initialised empty imperium repository at " << path << "\n";
  }
}

void addToCache(std::string path, char type) {
  struct stat buffer;
  if (stat((root + "/.imperium/.add").c_str(), &buffer) != 0) {
    mkdir((root + "/.imperium/.add").c_str(), 0777);
  }
  if (type == 'f') {
    std::string filename = path.substr(root.length());
    std::string filerel = root + "/.imperium/.add" +
                          filename.substr(0, filename.find_last_of("/"));

    struct stat buffer2;
    if (stat(filerel.c_str(), &buffer2) != 0) {
      fs::create_directories(filerel);
    }
    fs::copy_file(path, root + "/.imperium/.add" + filename,
                  fs::copy_options::overwrite_existing);
  } else if (type == 'd') {
    std::string filename = path.substr(root.length());
    std::string filerel = root + "/.imperium/.add" + filename;

    struct stat buffer3;
    if (stat(filerel.c_str(), &buffer3) != 0) {
      fs::create_directories(filerel);
    }
  }
}

bool ignoreFolder(std::string path, std::vector<std::string> dirname) {
  for (auto dir : dirname) {
    dir.pop_back();

    if (path.find(dir) != std::string::npos) {
      return true;
    }
  }
  return false;
}

bool toBeIgnored(std::string path, int onlyImperiumIgnore = 0) {
  std::ifstream addFile, ignoreFile;
  std::string file_or_dir;
  std::vector<std::string> filenames;
  std::vector<std::pair<std::string, char>> addedFileNames;
  std::vector<std::string> ignoreDir;

  ignoreFile.open(root + "/.imperiumIgnore");
  addFile.open(root + "/.imperium/add.log");

  if (ignoreFile.is_open()) {

    while (!ignoreFile.eof()) {
      std::getline(ignoreFile, file_or_dir);
      auto i = file_or_dir.end();

      i--;
      if (*i == '/') {
        ignoreDir.push_back(file_or_dir);
      } else {
        filenames.push_back(root + "/" + file_or_dir);
      }
    }
  }
  ignoreFile.close();

  if (onlyImperiumIgnore == 0) {
    if (addFile.is_open()) {
      while (!addFile.eof()) {
        std::getline(addFile, file_or_dir);
        if (file_or_dir.length() > 4) {
          addedFileNames.push_back(
              std::make_pair(file_or_dir.substr(1, file_or_dir.length() - 4),
                             file_or_dir.at(file_or_dir.length() - 1)));
        }
      }
    }
    addFile.close();

    for (auto fileEntry : addedFileNames) {
      if (path.compare(fileEntry.first) == 0) {
        addToCache(path, fileEntry.second);
        std::cout << "Updated: " << path << std::endl;
        return true;
      }
    }
  }
  if ((std::find(filenames.begin(), filenames.end(), path)) !=
          filenames.end() ||
      ignoreFolder(path, ignoreDir)) {
    return true;
  }
  return false;
}

void add(char *argv) {
  struct stat buffer;
  if (stat((root + "/.imperium").c_str(), &buffer) == 0) {
    std::ofstream addFile;
    addFile.open(root + "/.imperium/add.log", std::ios_base::app);
    std::string path = root;
    if (strcmp(argv, ".") != 0) {
      path = path + "/" + argv;
    }
    struct stat buffer2;
    if (stat(path.c_str(), &buffer2) == 0) {
      if (buffer2.st_mode & S_IFDIR) {
        if (!toBeIgnored(path)) {
          addFile << "\"" << path << "\""
                  << "-d\n";
          addToCache(path, 'd');
          std::cout << "Added directory: "
                    << "\"" << path << "\""
                    << "\n";
        }
        for (auto &p : fs::recursive_directory_iterator(path)) {
          if (toBeIgnored(p.path()))
            continue;
          struct stat buffer3;
          if (stat(p.path().c_str(), &buffer3) == 0) {

            if (buffer3.st_mode & S_IFDIR) {
              addToCache(p.path(), 'd');
              addFile << p.path() << "-d\n";
              std::cout << "Added directory: " << p.path() << "\n";
            } else if (buffer3.st_mode & S_IFREG) {
              addToCache(p.path(), 'f');
              addFile << p.path() << "-f\n";
              std::cout << "Added file: " << p.path() << "\n";
            } else {
              continue;
            }
          }
        }
      } else if (buffer2.st_mode & S_IFREG) {
        if (!toBeIgnored(path)) {
          addToCache(path, 'f');
          addFile << "\"" << path << "\""
                  << "-f\n";
          std::cout << "Added file: "
                    << "\"" << path << "\""
                    << "\n";
        }
      } else {
        std::cout << "Invalid path!\n";
      }
    }
  } else {
    std::cout << "Repository has not been initialised yet.\n";
  }
}

int main(int argc, char **argv) {
  root = std::getenv("PWD");
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
            "node_modules\n.env\n.imperium/\n.imperiumIgnore";
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
        std::string fileContent =
            "node_modules\n.env\n.imperium/\n.imperiumIgnore";
        writeFile(ignorePath, fileContent);
        // written .imperiumIgnore here
        temp = std::strcat(temp, "/.imperium");
        const char *path = temp;
        init(path);
      }
    }
    if (strcmp(argv[1], ADD) == 0) {
      if (strcmp(argv[2], ".") != 0) {
        for (int i = 2; i < argc; i++) {
          add(argv[i]);
        }
      } else {
        add(argv[2]);
      }
    }
  }
  return 0;
}