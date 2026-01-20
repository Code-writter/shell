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

vector<string> command_history;

bool run_command(vector<string> input, bool should_fork = true){

    // ________________DETECT REDIRECTION__________
    string stdout_file = "";
    string stderr_file = "";
    bool stdout_append = false; // For >> appending the stdout
    bool stderr_append = false; // For 2>> appending the stderr

    // Scan loop must handle erasing elements dynamically
    for(size_t i = 0; i<input.size(); ){
        // Check for append first
        if(input[i] == ">>" || input[i] == "1>>"){
            if(i + 1 < input.size()){
                stdout_file = input[i+ 1];
                stdout_append = true; // Enable append Mode

                // Erase other part
                input.erase(input.begin() + i, input.begin() + i + 2);
                continue;
            }
        }
        // Check for OVERWRITE
        else if(input[i] == ">" || input[i] == "1>"){
            if(i + 1 < input.size()){

                stdout_file = input[i + 1];
                stdout_append = false; // Keep the append mode disabled
                // Remove Operator and filename
                input.erase(input.begin() + i, input.begin() + i + 2);
                // Do not increment because the next element shifted into current
                continue;
            }
        }
        // Appending stderror
        else if(input[i] == "2>>"){
            if(i + 1 < input.size()){
                stderr_file = input[i + 1];
                stderr_append = true; // use O_APPEND

                // erase the part
                input.erase(input.begin() + i, input.begin() + i + 2);
                continue;
            }
        }
        else if(input[i] == "2>"){
            if(i + 1 < input.size()){
                stderr_file = input[i+1];
                // make the stderror append false
                stderr_append = false; // use O_TRUNC
                input.erase(input.begin() + i, input.begin() + i + 2);
                continue;
            }
        }
        i++; // Move next part only if we didn't remove anything
    }

    if(input.empty()) return true;
    string command = input[0];
    // string redirect_file = "";
    // bool append_mode = false;

    // // Scan for ">" or "1>"
    // for(size_t i = 0; i < input.size(); i++){
    //     if(input[i] == ">" || input[i] == "1>"){
    //         if(i + 1 < input.size()){
    //             redirect_file = input[i + 1];

    //             input.erase(input.begin() + i, input.begin() + i + 2);
    //             break;
    //         }
    //     }
    // }

    // string command = input[0];

    // Save & RESTORE STDOUT for Builtins
    int saved_stdout = -1;
    int saved_stderr = -1;

    bool is_child = !should_fork;

    auto setup_redirection = [&](bool is_child_process){
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
        return false;
    }

    else if(command ==  "history"){
        if(setup_redirection(is_child)){
            for(size_t i = 0; i<command_history.size(); i++){
                cout<<"    "<<i+1<<"  "<<command_history[i]<<endl;
            }
            restore_redirection();
        }
    }

    else if(command == "echo")
    {   
        if(setup_redirection(is_child)){

            // cout<<args<<endl;
            // Print every thing after the first token
            for(size_t i = 1; i<input.size(); i++){
                cout<<input[i]<<(i == input.size() - 1 ? "" : " ");
            }
            cout<<endl;
            restore_redirection();
        }
    }

    else if(command == "type")
    {
        if(setup_redirection(is_child)){

            if(input.size() >= 2){

                
                string args = input[1];

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
        if(setup_redirection(is_child)){
            cout<<filesystem::current_path().string()<<endl;
        }
    }
    // Change dir 
    else if(command == "cd")
    {
        if(setup_redirection(is_child)){
            if(input.size() > 1){
                string path = input[1];
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
            if(!should_fork) exit(0);
        }
        else
        {
            if(should_fork){ 
                // Normal case : Shell --> Fork --> Exec
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
                    for(auto &s : input){
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
                // else if(pid > 0)
                // {
                //     // PARENT PROCESS
                //     int status;
                //     waitpid(pid, &status, 0);
                // }
                else
                {
                    wait(NULL);
                }
            }else{
                setup_redirection(true);

                vector<char*> args;
                for(auto &s : input) args.push_back(&s[0]);
                args.push_back(NULL);
                execvp(command.c_str(), args.data());
                perror("execvp");
                exit(1);
            }
        }
        // cout<<input_line<<": command not found"<<endl;
    }
    return true; //Continue shell loop
}