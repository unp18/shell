#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>

bool isBuiltin(const std::string &cmd){
  std::vector<std::string> commands = {"type", "echo", "exit"};
  for(auto command : commands){
    if(command == cmd){
      std::cout<<cmd<<": is a shell builtin"<<std::endl;
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
    // if(input.substr(5) == "echo" || input.substr(5) == "type" || input.substr(5) == "exit"){
    //   std::cout<<input.substr(5)<<" is a shell builtin"<<std::endl;
    // }
    // else{
    //   std::string cmd = input.substr(5);
    //   isAvailable(cmd);

    // }
  }
  else{
    std::cout<< input <<": command not found"<< std::endl;
  }
  // std::stringstream parsed(input);
  // std::string word;
  // bool f=0;
  // while(parsed >> word){
  //   if(f){
  //     std::cout<<word<<" ";
  //     continue;
  //   }
  //   if(word == "echo"){
  //     f=1;
  //     continue;
  //   }
  //   else{
  //     break;
  //   }
  // }
  // if(f){
  //   std::cout<<std::endl;
  // }
  // else{
  //   std::cout<< input <<": command not found"<< std::endl;
  // }
  }
}
