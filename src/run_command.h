#include<bits/stdc++.h>
#include<sys/wait.h> // For wait()
#include<filesystem> // for pwd
#include<fcntl.h> // for open(), O_CREAT, 
#include<algorithm>
#include<string>
#include<fstream>
#include<memory> // For unique_ptr
#include<array>  // For array

// Readline headers for tab completions
#include<stdio.h>
#include<readline/readline.h>
#include<readline/history.h>

// Custom Headers
#include "get_file_path.h"
#include "split_input.h"
#include "builtin_generator.h"

int history_write_index = 0;

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

    // Save & RESTORE STDOUT for Builtins
    int saved_stdout = -1;
    int saved_stderr = -1;

    bool is_child = !should_fork;

    auto setup_redirection = [&](bool is_child_process){
        // 1. Handle STDOUT (>)
        if(!stdout_file.empty()){
            if(!is_child_process) saved_stdout = dup(STDOUT_FILENO);

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
            // Determine the starting address
            if(input.size() > 1 && input[1] == "-r"){
                if(input.size() > 2){
                    string filename = input[2];
                    ifstream file(filename);
                    if((file.is_open())){
                        string line;
                        while(getline(file, line)){
                            if(!line.empty()){
                                command_history.push_back(line);
                            }
                        }
                        file.close();
                        history_write_index = command_history.size();
                    }else{
                        cerr <<"history:"<<filename<<": No such file or directory"<<endl;
                    }
                }
                else{
                    cerr<<"history: option requires an argument -- 'r'"<<endl;
                }
            }
            else if(input.size() > 2 && input[1] == "-w"){
                if(input.size()> 2){
                    string filename = input[2];

                    ofstream file(filename);
                    if(file.is_open()){
                        for(const auto& line : command_history){
                            file<<line <<endl;
                        }
                        file.close();
                        history_write_index = command_history.size();
                    }else{
                        cerr<<"history: "<< filename <<": cannot create file"<<endl;
                    }
                }else{
                    cerr<<"history: option requires an argument -- 'w'"<<endl;
                }
            }
            else if(input.size() > 2 && input[1] == "-a"){
                if(input.size()> 2){
                    string filename = input[2];

                    ofstream file(filename, std::ios::app); //append index
                    if(file.is_open()){
                        for(int i = history_write_index; i<command_history.size(); i++){
                            file<<command_history[i]<<endl;
                        }
                        file.close();
                        history_write_index = command_history.size();
                    }else{
                        cerr<<"history: "<< filename <<": cannot create file"<<endl;
                    }
                }else{
                    cerr<<"history: option requires an argument -- 'a'"<<endl;
                }
            }
            else{
                int start_index = 0;
                if(input.size() > 1){
                    try{
                        int n = stoi(input[1]);
                        start_index = max(0, (int)command_history.size() - n);
                    }catch(...){}
                }
                for(int i = start_index; i<command_history.size(); i++){
                    cout<<"    "<<i+1<<"  "<<command_history[i]<<endl;
                }
            }
            restore_redirection();
        }
    }

    // --- NEW: AI INTEGRATION ---
    else if (command == "ai") {
        if (setup_redirection(is_child)) {
            // 1. Gather the prompt
            string prompt = "";
            for (size_t i = 1; i < input.size(); i++) { 
                prompt += input[i] + " ";               
            }

            // 2. Gather the last 5 commands from history as context
            string recent_history = "";
            int start_idx = max(0, (int)command_history.size() - 5);
            for (int i = start_idx; i < command_history.size(); i++) {
                recent_history += command_history[i] + "\n";
            }

            // 3. Helper to escape quotes so the shell doesn't break
            auto escape_quotes = [](const string& str) {
                string escaped;
                for (char c : str) {
                    if (c == '"' || c == '\\' || c == '`' || c == '$') escaped += '\\';
                    escaped += c;
                }
                return escaped;
            };

            // 4. Construct the popen command
            string script_path = std::filesystem::current_path().string() + "/ai_agent.py"; 
            string system_cmd = "python3 " + script_path + " --history \"" + 
                                escape_quotes(recent_history) + "\" \"" + escape_quotes(prompt) + "\"";

            // 5. Execute using popen to CAPTURE the output
            array<char, 128> buffer;
            string full_output;
            unique_ptr<FILE, decltype(&pclose)> pipe(popen(system_cmd.c_str(), "r"), pclose);
            
            if (!pipe) {
                cerr << "❌ Failed to start AI assistant." << endl;
            } else {
                while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                    full_output += buffer.data();
                }

                // 6. Parse the <EXEC> tags
                string start_tag = "<EXEC>";
                string end_tag = "</EXEC>";
                size_t start_pos = full_output.find(start_tag);
                size_t end_pos = full_output.find(end_tag);

                if (start_pos != string::npos && end_pos != string::npos && end_pos > start_pos) {
                    // Extract explanation
                    string explanation = full_output.substr(0, start_pos);
                    explanation.erase(explanation.find_last_not_of(" \n\r\t") + 1);
                    
                    // Extract command
                    size_t cmd_start = start_pos + start_tag.length();
                    string exec_command = full_output.substr(cmd_start, end_pos - cmd_start);
                    exec_command.erase(0, exec_command.find_first_not_of(" \n\r\t"));
                    exec_command.erase(exec_command.find_last_not_of(" \n\r\t") + 1);

                    // 7. Print and ask for confirmation
                    cout << "\n🤖 " << explanation << "\n\n";
                    cout << "🚀 Run command: \033[1;32m" << exec_command << "\033[0m ? [Y/n/Enter]: ";
                    
                    string user_choice;
                    getline(cin, user_choice);

                    if (user_choice.empty() || user_choice == "y" || user_choice == "Y") {
                        cout << endl;
                        
                        vector<string> ai_parsed_input = split_input(exec_command); 
                        
                        // 9. Recursively run the command!
                        if (!ai_parsed_input.empty()) {
                            run_command(ai_parsed_input, should_fork);
                        }
                    } else {
                        cout << "Skipped.\n";
                    }
                } else {
                    // Fallback if AI didn't use tags correctly
                    cout << "\n🤖 " << full_output << "\n";
                }
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

                if(args == "exit" || args == "echo" || args == "type" || args == "pwd" || args == "cd" || args == "history")
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
                    setup_redirection(true);

                    vector<char*> args;
                    for(auto &s : input){
                        args.push_back(&s[0]);

                    }
                    args.push_back(NULL); // Must be NULL teminated

                    // EXECUTE
                    execvp(command.c_str(), args.data());
                    
                    // IF execvp returns, it failed (ex : Permission denied)
                    perror("execvp");
                    exit(1);
                }
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
    }
    return true; //Continue shell loop
}