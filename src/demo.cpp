#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <readline/history.h>
#include <readline/readline.h>

std::vector<std::string> vocabulory{"zero", "one", "two",   "three", "four",
                                    "five", "six", "seven", "eight", "nine"};

char *command_generator(const char *text, int state) {
  static std::vector<std::string> matches;
  static size_t match_index = 0;

  if (state == 0) {
    matches.clear();
    match_index = 0;

    std::string textstr(text);
    for (auto word : vocabulory) {
      if (word.size() >= textstr.size() &&
          word.compare(0, textstr.size(), textstr) == 0) {
        matches.push_back(word);
      }
    }
  }

  if (match_index >= matches.size()) {
    return nullptr;
  } else {
    return strdup(matches[match_index++].c_str());
  }
}

char **command_completion(const char *text, int start, int end) {
  rl_attempted_completion_over = 1;
  return rl_completion_matches(text, command_generator);
}

int main(int argc, char **argv) {
  if (argc > 1 && std::string(argv[1]) == "-d") {
    rl_bind_key('\t', rl_insert);
  }
  rl_attempted_completion_function = command_completion;

  char *buf;
  std::string cmd;

  while ((buf = readline("> ")) != nullptr) {
    cmd = std::string(buf);
    if (cmd.size() > 0) {
      add_history(buf);
    }
    free(buf);
    std::stringstream scmd(cmd);
    scmd >> cmd;

    std::vector<std::string>::iterator itr =
        std::find(vocabulory.begin(), vocabulory.end(), cmd);
    if (itr != vocabulory.end()) {
      std::cout << cmd << ": " << std::distance(vocabulory.begin(), itr)
                << std::endl;
    } else {
      std::cout << "Invalid number name" << std::endl;
    }
  }

  std::cout << std::endl;
  return 0;
}