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
#include <fstream>
#include <fcntl.h>

bool reOut = false;
bool reError = false;
bool appOut = false;
bool appError = false;
std::string loc,locE;
std::vector<std::string> getArgs(const std::string &input){
  std::vector<std::string> args;
  std::string tmp;
  bool in_double_quote = false;
  bool in_single_quote = false;
  bool is_space = false;

  for (size_t i = 0; i < input.length(); ++i) {
        char c = input[i];

      if (c == '\\' && (in_double_quote || in_single_quote)) { // Handle escape sequences
        
        if (i + 1 < input.length()) {
            char next = input[i + 1];
        // Only unescape quotes or another backslash
            if ((in_double_quote && (next == '"' || next == '\\')) ||
            (in_single_quote && (next == '\'' || next == '\\'))) {
            tmp += next;
            i++; // skip the next char
        } else {
            tmp += '\\'; // keep the backslash
        }
          //      tmp += input[++i]; // Add escaped char
        } else tmp+='\\';
      } else if (c == '"' && !in_single_quote) {
            if(i!=input.length()-1 && input[i+1] == c) {i++;continue;}
            in_double_quote = !in_double_quote;
            if (!in_double_quote && !tmp.empty()) { // End of a quoted arg
                args.push_back(tmp);
                tmp.clear();
            }
      } else if (c == '\'' && !in_double_quote) {
            if(i!=input.length()-1 && input[i+1] == c) {i++;continue;}
            in_single_quote = !in_single_quote;
            if (!in_single_quote && !tmp.empty()) { // End of a quoted arg
                args.push_back(tmp);
                tmp.clear();
            }
      } else if (std::isspace(c) && !in_double_quote && !in_single_quote) {
            if (!tmp.empty()) {
                args.push_back(tmp);
                tmp.clear();
            }
            int temp = args.size()-1;
            if(temp >=1){
              int temp2 = args[temp].size()-1;
              if(args[temp] != " ") args.push_back(" ");
            }
      } else if (c == '\\'){
         if(i!=input.length()-1){
          tmp+=input[i+1];
          i++;
         }
      }
        else {
            tmp += c;
        }
    }

    if (!tmp.empty()) { // Add last argument if it exists
        args.push_back(tmp);
    }

    return args;
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

  void external(const std::vector<std::string> &args) {
    // 1. Parse the input string into command and arguments
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
        if (!loc.empty()) {
            int flags = O_WRONLY | O_CREAT | (appOut ? O_APPEND : O_TRUNC);
            int fd = open(loc.c_str(), flags, 0644);
            if (fd < 0) {
                perror(loc.c_str());
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        if (!locE.empty()) {
            int flags = O_WRONLY | O_CREAT | (appError ? O_APPEND :O_TRUNC);
            int fd = open(locE.c_str(), flags, 0644);
            if (fd < 0) {
                perror(locE.c_str());
                exit(1);
            }
            dup2(fd, STDERR_FILENO);
            close(fd);
        }
        std::vector<char*> argv;
        for (const auto& arg : args) {
          if(arg == " ") continue;
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
  std::vector<std::string> args = getArgs(input);
  loc="";
  reOut = false;
  reError = false;
  appError = false;
  appOut = false;
  locE = "";
  for(int i=0; i<args.size(); i++){
    if(args[i] == ">" || args[i] == "1>"){
      if(i!= args.size()-2){
        loc = args[i+2];
        args.erase(args.begin()+i, args.begin()+i+3);
        reOut = true;
      }
    }
    if(args[i] == ">>" || args[i] == "1>>"){
      if(i!= args.size()-2){
        loc = args[i+2];
        args.erase(args.begin()+i, args.begin()+i+3);
        reOut = true;
        appOut = true;
      }
    }
    if(args[i] == "2>"){
      if(i!= args.size()-2){
        locE = args[i+2];
        args.erase(args.begin()+i, args.begin()+i+3);
        reError = true;
      }
    }
    if(args[i] == "2>>"){
      if(i!= args.size()-2){
        locE = args[i+2];
        args.erase(args.begin()+i, args.begin()+i+3);
        reError = true;
        appError = true;
      }
    }
  }
  std::streambuf* original_cout = std::cout.rdbuf();
  std::streambuf* original_cerr = std::cerr.rdbuf();
        std::ofstream file,fileError;
        if (!loc.empty()) {
            file.open(loc, appOut ? std::ios::app : std::ios::out);
            std::cout.rdbuf(file.rdbuf());
        }
        if(!locE.empty()){
          fileError.open(locE,appError ? std::ios::app : std::ios::out);
          std::cerr.rdbuf(fileError.rdbuf());
        }

  if(args[0] == "echo"){
    for(int i=1; i<args.size();i++)
    std::cout<<args[i]<<"";
    std::cout<<std::endl;
  }
  else if(args[0] == "type"){
    for(int i=1; i<args.size();i++){
    type(args[i]);
    }
  }
  else if(args[0] == "pwd"){
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::cout<<currentPath.string()<<std::endl;
  }
  else if(args[0] == "cd"){
    const std::string newDirectory = input.substr(3);
    if(newDirectory == "~"){
      chdir(getenv("HOME"));
    }
    else if(chdir(newDirectory.c_str()) != 0){
      std::cout<<"cd: "<<input.substr(3)<<": No such file or directory"<<std::endl;
    }
  }
  else{
    external(args);
  }
  if (!loc.empty()) {
            std::cout.rdbuf(original_cout);
            file.close();
        }
  if(!locE.empty()){
    std::cerr.rdbuf(original_cerr);
    fileError.close();
  }
  //loc = "";
  }

}
