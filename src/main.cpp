#include <iostream>
#include <string>
using namespace std;

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // TODO: Uncomment the code below to pass the first stage
  std::cout << "$ ";

  // Capture user's commands
  string command;
  getline(cin, command);

  // Command not found message
  cout<<command<<": command not found "<<endl;

}
