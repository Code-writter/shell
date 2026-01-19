#include<bits/stdc++.h>

// Custom Headers
#include "get_file_path.h"
#include "trim.h"

using namespace std;

int main(){
    // Flush after every std::cout / std::cerr

    cout<<unitbuf;
    cerr<<unitbuf;

    while(true){
        cout<<"$ ";

        // Capture user commands
        string input_line;
        getline(cin, input_line);

        if(input_line.empty()) continue;

        stringstream ss(input_line);

        string command, args;

        ss >> command;

        size_t command_length = command.length();
        if(input_line.length() > command_length){
            size_t arg_start = input_line.find_first_not_of(" ", command_length);

            if(arg_start != string::npos){
                args = input_line.substr(arg_start);
            }
        }

        // ###### COMMAND HANDLING #########

        if(command == "exit")
        {
            break;
        }

        else if(command == "echo")
        {
            cout<<args<<endl;
        }

        else if(command == "type")
        {
            if(args == "exit" || args == "echo" || args == "type" || args == "ls")
            {
                cout<<args<<" is a shell builtin"<<endl;
            }
            else
            {
                // Get path

                string path = get_path(args);

                if(!path.empty()){
                    cout<<args<<" is "<<path<<endl;
                }
                else{
                    cout<<args<<": not found"<<endl;
                }
            }

        }else
        {
            cout<<input_line<<": command not found"<<endl;
        }

    }

}