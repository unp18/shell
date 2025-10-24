#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <filesystem>
#include <fstream>

std::vector<std::string> getArgs(const std::string &input){
    std::vector<std::string> args;
    std::string tmp;
    bool in_double_quote = false;
    bool in_single_quote = false;

    for (size_t i = 0; i < input.length(); ++i) {
        char c = input[i];

        if (c == '\\' && (in_double_quote || in_single_quote)) {
            if (i + 1 < input.length()) {
                char next = input[i + 1];
                tmp += next;
                i++;
            }
        } else if (c == '"' && !in_single_quote) {
            in_double_quote = !in_double_quote;
            if (!in_double_quote && !tmp.empty()) {
                args.push_back(tmp);
                tmp.clear();
            }
        } else if (c == '\'' && !in_double_quote) {
            in_single_quote = !in_single_quote;
            if (!in_single_quote && !tmp.empty()) {
                args.push_back(tmp);
                tmp.clear();
            }
        } else if (std::isspace(c) && !in_double_quote && !in_single_quote) {
            if (!tmp.empty()) {
                args.push_back(tmp);
                tmp.clear();
            }
        } else {
            tmp += c;
        }
    }
    if (!tmp.empty()) args.push_back(tmp);
    return args;
}

bool isBuiltin(const std::string &cmd){
    std::vector<std::string> commands = {"type", "echo", "exit","pwd","cd"};
    for(auto &command : commands){
        if(command == cmd){
            return true;
        }
    }
    return false;
}

void typeBuiltin(const std::string& cmd){
    std::vector<std::string> builtins = {"type", "echo", "exit", "pwd", "cd"};
    for(auto &b : builtins){
        if(cmd == b){
            std::cout << cmd << " is a shell builtin" << std::endl;
            return;
        }
    }

    const char *pathEnv = std::getenv("PATH");
    if(!pathEnv){
        std::cout<< cmd<< ": not found"<<std::endl;
        return;
    }

    std::istringstream ss(pathEnv);
    std::string dir;
    while(std::getline(ss,dir,':')){
        if(dir.empty()) continue;
        std::string fullPath = dir + "/" + cmd;
        if(access(fullPath.c_str(),X_OK) == 0){
            std::cout<< cmd<< " is "<< fullPath <<std::endl;
            return;
        }
    }
    std::cout << cmd << ": not found" << std::endl;
}

void external(std::vector<std::string> args,
              const std::string &inFile,
              const std::string &outFile,
              bool append) {
    if (args.empty()) return;
    const std::string &cmd = args[0];

    const char *pathEnv = std::getenv("PATH");
    if (!pathEnv) {
        std::cerr << cmd << ": not found" << std::endl;
        return;
    }

    std::string pathVar(pathEnv);
    std::istringstream ss(pathVar);
    std::string dir, fullPath;
    bool found = false;
    while (std::getline(ss, dir, ':')) {
        if (dir.empty()) continue;
        std::string potentialPath = dir + "/" + cmd;
        if (access(potentialPath.c_str(), X_OK) == 0) {
            fullPath = potentialPath;
            found = true;
            break;
        }
    }

    if (!found) {
        std::cerr << cmd << ": command not found" << std::endl;
        return;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return;
    } else if (pid == 0) {
        // --- Child process ---
        if (!inFile.empty()) {
            int fd = open(inFile.c_str(), O_RDONLY);
            if (fd < 0) {
                perror(inFile.c_str());
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if (!outFile.empty()) {
            int flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
            int fd = open(outFile.c_str(), flags, 0644);
            if (fd < 0) {
                perror(outFile.c_str());
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        std::vector<char*> argv;
        for (auto &a : args)
            argv.push_back(const_cast<char*>(a.c_str()));
        argv.push_back(nullptr);
        execv(fullPath.c_str(), argv.data());
        perror("execv");
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
}

int main() {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    while(true){
        std::cout << "$ ";
        std::string input;
        std::getline(std::cin, input);
        if (input == "exit 0") return 0;

        std::vector<std::string> args = getArgs(input);
        if (args.empty()) continue;

        // --- Parse Redirections ---
        std::string outFile, inFile;
        bool append = false;

        for (size_t i = 0; i < args.size(); ++i) {
            if (args[i] == ">" || args[i] == "1>") {
                if (i + 1 < args.size()) outFile = args[i + 1];
                args.erase(args.begin() + i, args.begin() + i + 2);
                break;
            } else if (args[i] == ">>") {
                if (i + 1 < args.size()) { outFile = args[i + 1]; append = true; }
                args.erase(args.begin() + i, args.begin() + i + 2);
                break;
            } else if (args[i] == "<") {
                if (i + 1 < args.size()) inFile = args[i + 1];
                args.erase(args.begin() + i, args.begin() + i + 2);
                break;
            }
        }

        // --- Builtin commands ---
        std::streambuf* original_cout = std::cout.rdbuf();
        std::ofstream file;
        if (!outFile.empty()) {
            file.open(outFile, append ? std::ios::app : std::ios::trunc);
            std::cout.rdbuf(file.rdbuf());
        }

        if (args[0] == "echo") {
            for (size_t i = 1; i < args.size(); ++i)
                std::cout << args[i] << (i + 1 < args.size() ? " " : "");
            std::cout << std::endl;
        }
        else if (args[0] == "type") {
            for (size_t i = 1; i < args.size(); ++i)
                typeBuiltin(args[i]);
        }
        else if (args[0] == "pwd") {
            std::cout << std::filesystem::current_path().string() << std::endl;
        }
        else if (args[0] == "cd") {
            std::string dir = (args.size() > 1) ? args[1] : getenv("HOME");
            if (chdir(dir.c_str()) != 0)
                std::cout << "cd: " << dir << ": No such file or directory" << std::endl;
        }
        else {
            external(args, inFile, outFile, append);
        }

        if (!outFile.empty()) {
            std::cout.rdbuf(original_cout);
            file.close();
        }
    }
}