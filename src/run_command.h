#include<bits/stdc++.h>
#include<sys/wait.h> // For wait()
#include<filesystem> // for pwd
#include<fcntl.h> // for open(), O_CREAT, 

// Readline headers for tab completions
#include<stdio.h>
#include<readline/readline.h>
#include<readline/history.h>

// Custom Headers
#include "get_file_path.h"
#include "split_input.h"
#include "builtin_generator.h"

using namespace std;


bool run_command(vector<string> parts_of_input){

    // ________________DETECT REDIRECTION__________
    string stdout_file = "";
    string stderr_file = "";
    bool stdout_append = false; // For >> appending the stdout
    bool stderr_append = false; // For 2>> appending the stderr

    // Scan loop must handle erasing elements dynamically
    for(size_t i = 0; i<parts_of_input.size(); ){
        // Check for append first
        if(parts_of_input[i] == ">>" || parts_of_input[i] == "1>>"){
            if(i + 1 < parts_of_input.size()){
                stdout_file = parts_of_input[i+ 1];
                stdout_append = true; // Enable append Mode

                // Erase other part
                parts_of_input.erase(parts_of_input.begin() + i, parts_of_input.begin() + i + 2);
                continue;
            }
        }
        // Check for OVERWRITE
        else if(parts_of_input[i] == ">" || parts_of_input[i] == "1>"){
            if(i + 1 < parts_of_input.size()){

                stdout_file = parts_of_input[i + 1];
                stdout_append = false; // Keep the append mode disabled
                // Remove Operator and filename
                parts_of_input.erase(parts_of_input.begin() + i, parts_of_input.begin() + i + 2);
                // Do not increment because the next element shifted into current
                continue;
            }
        }
        // Appending stderror
        else if(parts_of_input[i] == "2>>"){
            if(i + 1 < parts_of_input.size()){
                stderr_file = parts_of_input[i + 1];
                stderr_append = true; // use O_APPEND

                // erase the part
                parts_of_input.erase(parts_of_input.begin() + i, parts_of_input.begin() + i + 2);
                continue;
            }
        }
        else if(parts_of_input[i] == "2>"){
            if(i + 1 < parts_of_input.size()){
                stderr_file = parts_of_input[i+1];
                // make the stderror append false
                stderr_append = false; // use O_TRUNC
                parts_of_input.erase(parts_of_input.begin() + i, parts_of_input.begin() + i + 2);
                continue;
            }
        }
        i++; // Move next part only if we didn't remove anything
    }

    if(parts_of_input.empty()) return true;
    string command = parts_of_input[0];
    // string redirect_file = "";
    // bool append_mode = false;

    // // Scan for ">" or "1>"
    // for(size_t i = 0; i < parts_of_input.size(); i++){
    //     if(parts_of_input[i] == ">" || parts_of_input[i] == "1>"){
    //         if(i + 1 < parts_of_input.size()){
    //             redirect_file = parts_of_input[i + 1];

    //             parts_of_input.erase(parts_of_input.begin() + i, parts_of_input.begin() + i + 2);
    //             break;
    //         }
    //     }
    // }

    // string command = parts_of_input[0];

    // Save & RESTORE STDOUT for Builtins
    int saved_stdout = -1;
    int saved_stderr = -1;

    auto setup_redirection = [&](bool is_child_process = false){
        // 1. Handle STDOUT (>)
        if(!stdout_file.empty()){
            if(!is_child_process) saved_stdout = dup(STDOUT_FILENO);

            // int fd = open(stdout_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);

            // Determine flags based on the mode
            int flags = O_WRONLY | O_CREAT;
            if(stdout_append){
                flags |= O_APPEND; // Add to end
            }else{
                flags |= O_TRUNC; // Wipe clean the previous data
            }

            int fd = open(stdout_file.c_str(), flags, 0644);

            if(fd < 0){
                perror("open stdout");
                return false;
            }

            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        //2. HANDLE STDERR
        if(!stderr_file.empty()){
            if(!is_child_process) saved_stderr = dup(STDERR_FILENO);
            // int fd = open(stderr_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            int flags = O_WRONLY | O_CREAT;
            if(stderr_append){
                flags |= O_APPEND; // Add to end
            }else{
                flags |= O_TRUNC; // Wipe clean the previous data
            }

            int fd = open(stderr_file.c_str(), flags, 0644);

            if(fd < 0){
                perror("open stderr");
                return false;
            }

            dup2(fd, STDERR_FILENO);
            close(fd);
        }
        return true;
    };

    auto restore_redirection = [&]() {
        if(saved_stdout != -1){
            // Restore terminal to stdout
            dup2(saved_stdout, STDOUT_FILENO);
            close(saved_stdout);
        }
        if(saved_stderr != -1){
            dup2(saved_stderr, STDERR_FILENO);
            close(saved_stderr);
        }
    };

    // ###### COMMAND HANDLING #########

    if(command == "exit")
    {
        return 0;
    }

    else if(command == "echo")
    {   
        if(setup_redirection()){

            // cout<<args<<endl;
            // Print every thing after the first token
            for(size_t i = 1; i<parts_of_input.size(); i++){
                cout<<parts_of_input[i]<<(i == parts_of_input.size() - 1 ? "" : " ");
            }
            cout<<endl;
            restore_redirection();
        }
    }

    else if(command == "type")
    {
        if(setup_redirection()){

            if(parts_of_input.size() >= 2){

                
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
                restore_redirection();
            }
        }
    }

    // Current Path 
    else if(command == "pwd")
    {
        if(setup_redirection()){
            cout<<filesystem::current_path().string()<<endl;
        }
    }
    // Change dir 
    else if(command == "cd")
    {
        if(setup_redirection()){
            if(parts_of_input.size() > 1){
                string path = parts_of_input[1];
                // Handle ~  for Home path
                if(path == "~"){
                    const char* home = getenv("HOME");
                    if(home){
                        path = home;
                    }
                    // else{
                    //     cout<<"cd: HOME environment variable not set";
                    //     continue;
                    // }
                }
                // chdir returns 0 on sucess and -1 on failure
                if(chdir(path.c_str()) != 0){
                    cout<<"cd: "<<path<<": No such file or directory"<<endl;
                }
            }
        }
        restore_redirection();
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
                // if(!redirect_file.empty()){
                //     int fd = open(redirect_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                //     if(fd < 0){
                //         perror("open");
                //         exit(1);
                //     }

                //     // Replace stdout with file
                //     dup2(fd, STDOUT_FILENO);
                //     close(fd);
                // }
                setup_redirection(true);

                vector<char*> args;
                for(auto &s : parts_of_input){
                    args.push_back(&s[0]);

                }
                args.push_back(NULL); // Must be NULL teminated

                // EXECUTE
                // execvp searches PATH program automatically and runs the command.
                execvp(command.c_str(), args.data());
                
                // IF execvp returns, it failed (ex : Permission denied)
                perror("execvp");
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
    return true; //Continue shell loop
}