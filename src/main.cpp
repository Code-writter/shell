#include<bits/stdc++.h>
#include<sys/wait.h>
#include<filesystem>
#include<fcntl.h>
#include <signal.h>

#include<stdio.h>
#include<readline/readline.h>
#include<readline/history.h>

#include "run_command.h"

using namespace std;


bool is_builtin(const string& cmd) {
    return (cmd == "cd" || cmd == "pwd" || cmd == "echo" || 
            cmd == "type" || cmd == "history" || cmd == "ai" || cmd == "exit");
}


void apply_devops_wrapper(vector<string>& cmd) {
    if (cmd.empty()) return;
    string name = cmd[0];
    

    if (name == "servers" || name == "add-server" || 
        name == "update-key" || name == "convert-ppk" || 
        name == "devops-help" || name == "zen") {
        
        vector<string> new_cmd = {"bash", "-c", "source ~/.devops_functions.sh 2>/dev/null; \"$@\"", "_"};
        new_cmd.insert(new_cmd.end(), cmd.begin(), cmd.end());
        cmd = new_cmd;
    }
}

int main(int argc, char* argv[]) {
    
    cout<<unitbuf;
    cerr<<unitbuf;


    bool is_interactive = true;
    string non_interactive_cmd = "";

    if (argc >= 3 && string(argv[1]) == "-c") {
        is_interactive = false;
        non_interactive_cmd = argv[2];
    }

    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

    if (is_interactive) {
        rl_attempted_completion_function = shell_completion;
    }

    const char* histfile = getenv("HISTFILE");
    if(histfile && is_interactive){
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
        string input_line;

        if (is_interactive) {
            char* input_c = readline("$ ");
            if(!input_c){
                cout<<endl;
                break;
            }
            input_line = input_c;
            
            if(!input_line.empty()){
                add_history(input_c);
                command_history.push_back(input_line);
            }
            free(input_c);
        } else {
            input_line = non_interactive_cmd;
        }
        
        if(input_line.empty()) {
            if (!is_interactive) break;
            continue;
        }

        vector<string> parts_of_input = split_input(input_line);
        if(parts_of_input.empty()) {
            if (!is_interactive) break;
            continue;
        }

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
        if(!current_command.empty()){
            commands.push_back(current_command);
        }

        for (auto& cmd : commands) {
            apply_devops_wrapper(cmd);
        }

        if(commands.size() == 1){
            if (is_builtin(commands[0][0])) {
                if(!run_command(commands[0], true)) break; 
            } 
            else {
                pid_t pid = fork();
                
                if (pid < 0) {
                    perror("Fork failed");
                    if (!is_interactive) break;
                    continue;
                }
                
                if (pid == 0) {
                    setpgid(0, 0);
                    tcsetpgrp(STDIN_FILENO, getpid());
                    
                    if(!run_command(commands[0], false)) exit(0); 
                    exit(0);
                } else {
                    setpgid(pid, pid);
                    tcsetpgrp(STDIN_FILENO, pid);

                    int status;
                    waitpid(pid, &status, WUNTRACED);

                    tcsetpgrp(STDIN_FILENO, getpid());
                }
            }
        }
        else{
            int prev_pipe_read = -1;
            vector<pid_t> children;

            for(size_t i = 0; i<commands.size(); i++){
                int pipefd[2];
                bool is_last = (i == commands.size() - 1);

                if(!is_last){
                    if(pipe(pipefd) == -1){
                        perror("pipe");
                        break;
                    }
                }

                pid_t pid = fork();

                if(pid == 0){
                    if (is_last) {
                        setpgid(0, 0);
                        tcsetpgrp(STDIN_FILENO, getpid());
                    }

                    if(prev_pipe_read != -1){
                        dup2(prev_pipe_read, STDIN_FILENO);
                        close(prev_pipe_read);
                    }

                    if(!is_last){
                        dup2(pipefd[1], STDOUT_FILENO);
                        close(pipefd[1]);
                        close(pipefd[0]);
                    }

                    run_command(commands[i], false); 
                    exit(0);
                }
                else{
                    children.push_back(pid);
                    
                    if (is_last) {
                        setpgid(pid, pid);
                        tcsetpgrp(STDIN_FILENO, pid);
                    }

                    if(prev_pipe_read != -1){
                        close(prev_pipe_read);
                    }

                    if(!is_last){
                        prev_pipe_read = pipefd[0];
                        close(pipefd[1]);
                    }
                }
            }

            for(pid_t pid : children){
                int status;
                waitpid(pid, &status, WUNTRACED);
            }
            
            tcsetpgrp(STDIN_FILENO, getpid());
        }

        if (!is_interactive) {
            break;
        }
    }

    signal(SIGTTOU, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);

    if(histfile && is_interactive){
        ofstream file(histfile, std::ios::app);
        if(file.is_open()){
            for(size_t i = history_write_index; i<command_history.size(); i++){
                file<<command_history[i]<<endl;
            }
            file.close();
        }
    }
    return 0;
}