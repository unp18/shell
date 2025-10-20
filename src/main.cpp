#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <filesystem>


void splitString(const std::string& str, std::vector<std::string>& tokens) {
    std::istringstream iss(str);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
}

bool isBuiltin(const std::string &cmd){
  std::vector<std::string> commands = {"type", "echo", "exit","pwd","cd"};
  for(auto command : commands){
    if(command == cmd){
      std::cout<<cmd<<" is a shell builtin"<<std::endl;
      return true;
    }
  }
  return false;
}
void type(const std::string&cmd){
  if(isBuiltin(cmd)) return;
  if(cmd.empty()){
    std::cerr<< "type: missing arguments";
    return ;
  }
  const char *pathEnv = std::getenv("PATH");
  if(!pathEnv){
    std::cout<< cmd<< ": not found"<<std::endl;
    return ;
  }

  std::string pathVar(pathEnv);
  std::istringstream ss(pathVar);
  std::string dir;
  bool found=0;

  while(std::getline(ss,dir,':')){
    if(dir.empty()) continue;

    std::string fullPath = dir + "/" + cmd;

    if(access(fullPath.c_str(),X_OK) == 0){
      std::cout<< cmd<< " is "<< fullPath <<std::endl;
      found =true;
      break;
    }

  }

  if(!found){
    std::cout<< cmd << ": not found"<< std::endl;
  }

}

  void external(const std::string &input) {
    // 1. Parse the input string into command and arguments
    std::vector<std::string> args;
    splitString(input, args);
    if (args.empty()) {
        return; // No command entered
    }
    const std::string& cmd = args[0];

    // 2. Find the full path of the command
    const char *pathEnv = std::getenv("PATH");
    if (!pathEnv) {
        std::cerr << cmd << ": not found" << std::endl;
        return;
    }

    std::string pathVar(pathEnv);
    std::istringstream ss(pathVar);
    std::string dir;
    std::string fullPath;
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

    // 3. Fork and execute the command
    pid_t pid = fork();

    if (pid == -1) {
        // Fork failed
        perror("fork");
        return;
    } else if (pid == 0) {
        // --- Child Process ---
        // Prepare arguments for execv (needs a char* array ending in NULL)
        std::vector<char*> argv;
        for (const auto& arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr); // Null-terminate the array

        // Execute the command. execv replaces the child process.
        execv(fullPath.c_str(), argv.data());

        // If execv returns, it's always an error.
        perror("execv");
        exit(EXIT_FAILURE);
    } else {
        // --- Parent Process ---
        int status;
        // Wait for the child process to terminate
        waitpid(pid, &status, 0);
    }
}


int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // Uncomment this block to pass the first stage
  while(true){
  std::cout << "$ ";
  std::string input;
  std::getline(std::cin, input);
  if(input == "exit 0"){
    return 0;
  }
  if(input.substr(0,4) == "echo"){
    std::cout<<input.substr(5)<<std::endl;
  }
  else if(input.substr(0,4) == "type"){
    std::string cmd = input.substr(5);
    type(cmd);
  }
  else if(input == "pwd"){
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::cout<<currentPath.string()<<std::endl;
  }
  else if(input.substr(0,2) == "cd"){
    const std::string newDirectory = input.substr(3);
    if(newDirectory == "~"){
      chdir(getenv("HOME"));
    }
    else(chdir(newDirectory.c_str()) != 0){
      std::cout<<"cd: "<<input.substr(3)<<": No such file or directory"<<std::endl;
    }
  }
  else{
    external(input);
  }
  }
}
