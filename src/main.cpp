#include <iostream>
#include <string>
using namespace std;

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // REPL
  /*
    Read :  Display a prompt and wait for user input
    Eval : Parse and execute the command
    Print : Display the output or error message
    Loop : Return to step 1 and wait for the next command
  */
  while(true){
    cout << "$ ";
    // Capture user's commands
    string command;
    getline(cin, command);

    if(command == "exit"){
      break;
    }
    // Here we are taking the string after four then printing all the command
    else if(command.substr(0, 4) == "echo"){
      cout<<command.substr(5)<<endl;

    }
    else if(command.substr(0,4) == "type"){
      if(command.substr(5) == "exit" || command.substr(5) == "echo" || command.substr(5) == "type"){
        cout<<command.substr(5)<<" is a shell builtin"<<endl;
      }else{
        cout<<command<<": not found"<<endl;
      }
    }
    else{
      // Command not found message
      cout<<command<<": command not found "<<endl;
    }




  }

}
