#include<bits/stdc++.h>
#include<sys/wait.h> // For wait()
#include<filesystem> // for pwd
#include<fcntl.h> // for open(), O_CREAT, 
#include <signal.h> // For signal handling

// Readline headers for tab completions
#include<stdio.h>
#include<readline/readline.h>
#include<readline/history.h>

// Custom Headers
// #include "get_file_path.h"
// #include "split_input.h"
// #include "builtin_generator.h"
#include "run_command.h"

using namespace std;

// vector<string> builtin_candidates = {"echo", "exit", "type", "pwd", "cd"};

int main(){
    // Flush after every std::cout / std::cerr
    cout<<unitbuf;
    cerr<<unitbuf;

    // --- PARENT SHELL TERMINAL CONTROL SETUP ---
    // Ignore terminal stop signals to prevent the shell from freezing 
    // when assigning foreground process groups to children (like tmux).
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

    //____READLINE____
    rl_attempted_completion_function = shell_completion;

    // _____LOAD HISTORY FORM HASHFILE______
    const char* histfile = getenv("HISTFILE");
    if(histfile){
        ifstream file(histfile);
        if(file.is_open()){
            string line;
            while(getline(file, line)){
                if(!line.empty()){
                    command_history.push_back(line);
                    add_history(line.c_str());
                }
            }
            file.close();
            history_write_index = command_history.size();
        }
    }

    while(true){
        
        char* input_c = readline("$ ");

        // readline returns NULL or EOF (Ctrl+D)
        if(!input_c){
            cout<<endl;
            break;
        }

        string input_line = input_c;
        // Add valid commands to history (allows up arrow usage)
        if(!input_line.empty()){
            add_history(input_c);
            command_history.push_back(input_line);
        }

        // Free the Memory readline allocated 
        free(input_c);
        
        if(input_line.empty()) continue;

        // Parsing Input into list of strings
        vector<string> parts_of_input = split_input(input_line);
        if(parts_of_input.empty()) continue;

        // _______PIPELINE SUPPORT (|)__________

        vector<vector<string>> commands;
        vector<string> current_command;

        for(const auto& token : parts_of_input){
            if(token == "|"){
                if(!current_command.empty()){
                    commands.push_back(current_command);
                    current_command.clear();
                }
            }else{
                current_command.push_back(token);
            }
        }
        // If current command is not empty and we do not get the pipe
        if(!current_command.empty()){
            commands.push_back(current_command);
        }

        // Loop execution
        if(commands.size() == 1){
            // Normal execution (No pipes)
            
            pid_t pid = fork();
            
            if (pid < 0) {
                perror("Fork failed");
                continue;
            }
            
            if (pid == 0) {
                // --- CHILD PROCESS ---
                // 1. Put the child in its own process group
                setpgid(0, 0);

                // 2. Hand over terminal foreground control to this child
                tcsetpgrp(STDIN_FILENO, getpid());
                
                // RUN command (Assumes run_command handles execvp and exits)
                if(!run_command(commands[0], false)) exit(0); 
                exit(0);
                
            } else {
                 // --- PARENT PROCESS ---
                
                // 1. Also set the child's process group here to avoid race conditions
                setpgid(pid, pid);

                // 2. Hand over terminal control to the child process group
                tcsetpgrp(STDIN_FILENO, pid);

                // 3. Wait for child (e.g., tmux) to finish or be suspended
                int status;
                waitpid(pid, &status, WUNTRACED);

                // 4. RECLAIM THE TERMINAL! 
                // Give control back to your shell's process ID so your prompt works again.
                tcsetpgrp(STDIN_FILENO, getpid());
            }
        }
        else{
            // PIPE EXECUTION
            int prev_pipe_read = -1;
            vector<pid_t> children;
            pid_t last_child_pid = -1;

            for(size_t i = 0; i<commands.size(); i++){
                int pipefd[2];

                // Create a pipe if this is NOT the last command
                bool is_last = (i == commands.size() - 1);

                if(!is_last){
                    if(pipe(pipefd) == -1){
                        perror("pipe");
                        break;
                    }
                }

                pid_t pid = fork();

                if(pid == 0){
                    // --- CHILD PROCESS ---
                    
                    // For terminal control, only the LAST process in a pipeline 
                    // should get foreground TTY control.
                    if (is_last) {
                        setpgid(0, 0);
                        tcsetpgrp(STDIN_FILENO, getpid());
                    }

                    // 1. Setup Input
                    if(prev_pipe_read != -1){
                        dup2(prev_pipe_read, STDIN_FILENO);
                        close(prev_pipe_read);
                    }

                    // 2. Setup Output (to next command)
                    if(!is_last){
                        dup2(pipefd[1], STDOUT_FILENO);
                        close(pipefd[1]);
                        close(pipefd[0]);
                    }

                    // RUN
                    run_command(commands[i], false); // Without forking
                    exit(0);
                }
                else{
                    // --- PARENT PROCESS ---
                    children.push_back(pid);
                    
                    if (is_last) {
                        last_child_pid = pid;
                        setpgid(pid, pid);
                        tcsetpgrp(STDIN_FILENO, pid);
                    }

                    // 1. Close the previous read end
                    if(prev_pipe_read != -1){
                        close(prev_pipe_read);
                    }

                    // 2. setup next read end
                    if(!is_last){
                        prev_pipe_read = pipefd[0];
                        close(pipefd[1]);
                    }
                }
            }

            // Wait for all children in pipeline
            for(pid_t pid : children){
                int status;
                waitpid(pid, &status, WUNTRACED);
            }
            
            // Reclaim terminal control after pipeline finishes
            tcsetpgrp(STDIN_FILENO, getpid());
        }
    }

    // --- RESTORE DEFAULT SIGNALS BEFORE EXITING ---
    signal(SIGTTOU, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);

    if(histfile){
        ofstream file(histfile, std::ios::app);
        if(file.is_open()){
            for(int i = history_write_index; i<command_history.size(); i++){
                file<<command_history[i]<<endl;
            }
            file.close();
        }
    }
    return 0;
}