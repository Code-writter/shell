#include<bits/stdc++.h>
#include<sys/wait.h> // For wait()
#include<filesystem> // for pwd
#include<fcntl.h> // for open(), O_CREAT, 

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

    //____READLINE____
    rl_attempted_completion_function = shell_completion;

    while(true){
        // cout<<"$ ";

        // Capture user commands
        // string input_line;
        // getline(cin, input_line);

        // if(input_line.empty()) continue;
        
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
            // Normal executin (No pipes)
            if(!run_command(commands[0], true)) break;
        }
        else{
            // PIPE EXECUTION
            int prev_pipe_read = -1;
            vector<pid_t> children;

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
                    // CHILD PROCESS

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
                    // PARENT PROCESS
                    children.push_back(pid);

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

            for(pid_t pid : children){
                waitpid(pid, NULL, 0);
            }
        }

        // 1. Finding the pipe in the command
        // auto pipe_it = find(parts_of_input.begin(), parts_of_input.end(), "|");

        // // 2. We got the pipe in the command
        // if(pipe_it != parts_of_input.end()){
        //     // split parts of cammand into left command and right command
        //     vector<string> left_part(parts_of_input.begin(), pipe_it);
        //     vector<string> right_part(pipe_it + 1, parts_of_input.end());

        //     //1. create the Pipe
        //     int pipefd[2];
        //     if(pipe(pipefd) == -1){
        //         perror("pipe"); 
        //         continue;
        //     }
                

        //     //2. Fork the Left Child (Writer)
        //     pid_t pid1 = fork();
        //     if(pid1 == 0){
        //         // Redirect stdout to pipe write end
        //         dup2(pipefd[1], STDOUT_FILENO);

        //         // Close both the ends we only need the dup'd copy
        //         close(pipefd[0]);
        //         close(pipefd[1]);

        //         // Run the command 
        //         run_command(left_part);
        //         exit(0); 
        //     }

        //     // 3. Fork the right child (Reader)
        //     pid_t pid2 = fork();
        //     if(pid2 == 0){
        //         // Redirect stdin from pipe read-end

        //         dup2(pipefd[0], STDIN_FILENO);
        //         close(pipefd[0]);
        //         close(pipefd[1]);

        //         run_command(right_part);
        //         exit(0);
        //     }

        //     // 4. Parent : Close Pipes and wait
        //     close(pipefd[0]);
        //     close(pipefd[1]);
        //     // Wait for a child matching PID to die.
        //     // If PID is greater than 0, match any process whose process ID is PID.
        //     waitpid(pid1, NULL, 0);
        //     waitpid(pid2, NULL, 0);

        // }else{
        //     // Notmal Execution (No pipes are given)
        //     if(!run_command(parts_of_input)) break; // Break if exit command found 
        //     // we are checking boolean because the run command function will return 
        //     // false when get the exit command
        // }

    }
}