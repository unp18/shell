#include <iostream>
#include <string>
#include <sstream>

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
    break;
  }
  std::stringstream parsed(input);
  std::string word;
  bool f=0;
  while(parsed >> word){
    if(f){
      std::cout<<word<<" ";
      continue;
    }
    if(word == "echo"){
      f=1;
      continue;
    }
    else{
      break;
    }
  }
  if(f){
    std::cout<<std::endl;
  }
  else{
    std::cout<< input <<": command not found"<< std::endl;
  }
  }
}
