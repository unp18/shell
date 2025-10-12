#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unistd.h>     // for access(), X_OK
#include <sys/stat.h>   // for stat()
#include <cstdlib>      // for getenv()

int main() {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    while (true) {
        std::cout << "$ ";
        std::string input;
        std::getline(std::cin, input);

        if (input == "exit 0") return 0;

        // Handle "type <cmd>"
        if (input.rfind("type ", 0) == 0) {
            std::string cmd = input.substr(5);
            const char* pathEnv = getenv("PATH");
            if (!pathEnv) {
                std::cerr << "PATH not set\n";
                continue;
            }

            std::string pathStr(pathEnv);
            std::stringstream ss(pathStr);
            std::string dir;
            bool found = false;

            while (std::getline(ss, dir, ':')) {
                std::string fullPath = dir + "/" + cmd;
                struct stat sb;
                if (stat(fullPath.c_str(), &sb) == 0) { // file exists
                    if (access(fullPath.c_str(), X_OK) == 0) { // executable
                        std::cout << fullPath << std::endl;
                        found = true;
                        break;
                    }
                }
            }

            if (!found) {
                std::cout << cmd << ": not found" << std::endl;
            }
        } else {
            std::cout << input << ": command not found" << std::endl;
        }
    }
}