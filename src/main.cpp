#include<bits/stdc++.h>
#include<sys/wait.h> // For wait()


// Custom Headers
#include "get_file_path.h"
#include "split_input.h"

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

        // stringstream ss(input_line);

        // string command, args;

        // ss >> command;

        // size_t command_length = command.length();
        // if(input_line.length() > command_length){
        //     size_t arg_start = input_line.find_first_not_of(" ", command_length);

        //     if(arg_start != string::npos){
        //         args = input_line.substr(arg_start);
        //     }
        // }

        // Parsing Input into list of strings
        vector<string> parts_of_input = split_input(input_line);
        string command = parts_of_input[0];

        // ###### COMMAND HANDLING #########

        if(command == "exit")
        {
            break;
        }

        else if(command == "echo")
        {
            // cout<<args<<endl;
            // Print every thing after the first token
            for(size_t i = 1; i<parts_of_input.size(); i++){
                cout<<parts_of_input[i]<<(i == parts_of_input.size() - 1 ? "" : " ");
            }
            cout<<endl;
        }

        else if(command == "type")
        {
            if(parts_of_input.size() > 2) continue;
            string args = parts_of_input[1];

            if(args == "exit" || args == "echo" || args == "type")
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
            // --- EXECUTE EXTERNAL PROGRAM ----

            // 1. Search for command in PATH
            string path = get_path(command);

            if(path.empty())
            {
                cout<<command<<": command not found"<<endl;
            }
            else
            {
                pid_t pid = fork();

                if(pid == 0)
                {
                    // CHILD PROCESS
                    // we are now inside the child process we must replace this
                    // process with target program using execvp.

                    // convert vector<string> to char* array for C API
                    vector<char*> args;
                    for(auto &s : parts_of_input){
                        args.push_back(&s[0]);

                    }
                    args.push_back(NULL); // Must be NULL teminated

                    // EXECUTE
                    // execvp searches PATH program automatically and runs the command.
                    execvp(command.c_str(), args.data());
                    
                    // IF execvp returns, it failed (ex : Permission denied)
                    perror("execvp failed");
                    exit(1);
                }
                else if(pid > 0)
                {
                    // PARENT PROCESS
                    int status;
                    waitpid(pid, &status, 0);
                }
                else
                {
                    perror("fork failed");
                }
            }

            // cout<<input_line<<": command not found"<<endl;
        }

    }

}