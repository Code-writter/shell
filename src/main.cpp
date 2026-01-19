#include<bits/stdc++.h>
#include<sys/wait.h> // For wait()
#include<filesystem> // for pwd
#include<fcntl.h> // for open(), O_CREAT, 
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
        if(parts_of_input.empty()) continue;

        // ________________DETECT REDIRECTION__________
        string redirect_file = "";
        bool append_mode = false;

        // Scan for ">" or "1>"
        for(size_t i = 0; i < parts_of_input.size(); i++){
            if(parts_of_input[i] == ">" || parts_of_input[i] == "1>"){
                if(i + 1 < parts_of_input.size()){
                    redirect_file = parts_of_input[i + 1];

                    parts_of_input.erase(parts_of_input.begin() + i, parts_of_input.begin() + i + 2);
                    break;
                }
            }
        }

        string command = parts_of_input[0];

        // Save & RESTORE STDOUT for Builtins
        int save_stdout = -1;
        int redirect_fd = -1;

        auto setup_builtin_redirection = [&](){
            if(!redirect_file.empty()){
                // Save current stdout (terminal) to a new fd
                save_stdout = dup(STDOUT_FILENO);
                /* Open the targe file
                    O_TRUNC = Overwrite file (Required for '>')
                    O_CREAT = Create file if missing
                    O_WRONLY = Write only
                */
                redirect_fd = open(redirect_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);

                if(redirect_fd < 0){
                    perror("open");
                    return false;
                }

                // Replace stdout (1) with our file
                dup2(redirect_fd, STDOUT_FILENO);
                close(redirect_fd);
            }
            return true;
        };

        auto restore_builtin_redirection = [&]() {
            if(save_stdout != -1){
                // Restore terminal to stdout
                dup2(save_stdout, STDOUT_FILENO);
                close(save_stdout);
            }
        };

        // ###### COMMAND HANDLING #########

        if(command == "exit")
        {
            return 0;
        }

        else if(command == "echo")
        {   
            if(setup_builtin_redirection()){
    
                // cout<<args<<endl;
                // Print every thing after the first token
                for(size_t i = 1; i<parts_of_input.size(); i++){
                    cout<<parts_of_input[i]<<(i == parts_of_input.size() - 1 ? "" : " ");
                }
                cout<<endl;
                restore_builtin_redirection();
            }
        }

        else if(command == "type")
        {
            if(setup_builtin_redirection()){

                if(parts_of_input.size() >= 2) continue;
                string args = parts_of_input[1];

                if(args == "exit" || args == "echo" || args == "type" || args == "pwd" || args == "cd")
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
                restore_builtin_redirection();
            }
        }

        // Current Path 
        else if(command == "pwd")
        {
            if(setup_builtin_redirection()){
                cout<<filesystem::current_path().string()<<endl;
            }
        }
        // Change dir 
        else if(command == "cd")
        {
            if(setup_builtin_redirection()){
                if(parts_of_input.size() > 1){
                    string path = parts_of_input[1];
                    // Handle ~  for Home path
                    if(path == "~"){
                        const char* home = getenv("HOME");
                        if(home){
                            path = home;
                        }else{
                            cout<<"cd: HOME environment variable not set";
                            continue;
                        }
                    }
                    // chdir returns 0 on sucess and -1 on failure
                    if(chdir(path.c_str()) != 0){
                        cout<<"cd: "<<path<<": No such file or directory"<<endl;
                    }
                }
            }
            restore_builtin_redirection();
        }
        else
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

                    // Handle Redirection inside the child
                    if(!redirect_file.empty()){
                        int fd = open(redirect_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if(fd < 0){
                            perror("open");
                            exit(1);
                        }

                        // Replace stdout with file
                        dup2(fd, STDOUT_FILENO);
                        close(fd);
                    }

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
                    wait(NULL);
                }
            }

            // cout<<input_line<<": command not found"<<endl;
        }

    }

}