#include <iostream>
#include <string>

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // Uncomment this block to pass the first stage
  while(true){
  std::cout << "$ ";
    std::string input;
    std::getline(std::cin, input);

    if (input == "exit 0") return 0;

    if (input.rfind("echo", 0) == 0) { // starts with "echo"
      if (input.size() > 5)
        std::cout << input.substr(5) << std::endl;
      else
        std::cout << std::endl;
    } else {
      std::cout << input << ": command not found" << std::endl;
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
