#include<bits/stdc++.h>

// Readline headers for tab completions
#include<stdio.h>
#include<readline/readline.h>

// File system
#include<filesystem>


using namespace std;
namespace fs = std::filesystem;

vector<string> builtin_candidates = {"echo", "exit", "type", "pwd", "cd"};
vector<string> matches; // Chache matches during autocompletion

// The Generator: Populates 'matches' on state 0, then feeds them back
char* command_generator(const char* text, int state) {
    // If state is 0, this is the first call for this tab press.
    // We must find ALL matches now and cache them.
    if (state == 0) {
        matches.clear();
        string text_str(text);
        set<string> found; // To track duplicates (e.g., 'ls' in multiple folders)

        // 1. Scan Builtins
        for (const auto& cmd : builtin_candidates) {
            if (cmd.find(text_str) == 0) { // Check prefix match
                matches.push_back(cmd);
                found.insert(cmd);
            }
        }

        // 2. Scan PATH
        string path_env = getenv("PATH");
        stringstream ss(path_env);
        string path_dir;

        while (getline(ss, path_dir, ':')) {
            // Check if directory exists before trying to open it
            if (!fs::exists(path_dir) || !fs::is_directory(path_dir)) {
                continue;
            }

            // Iterate over files in the directory
            try {
                for (const auto& entry : fs::directory_iterator(path_dir)) {
                    string filename = entry.path().filename().string();
                    
                    // Check 1: Does filename start with our text?
                    if (filename.find(text_str) == 0) {
                        
                        // Check 2: Have we already added this command?
                        if (found.find(filename) == found.end()) {
                            
                            // Check 3: Is it actually an executable file?
                            if (access(entry.path().c_str(), X_OK) == 0) {
                                matches.push_back(filename);
                                found.insert(filename);
                            }
                        }
                    }
                }
            } catch (...) {
                // Ignore permission errors (e.g., cannot read a directory)
            }
        }
    }

    // Return the next match from our cache
    if (state < matches.size()) {
        return strdup(matches[state].c_str());
    }
    return nullptr;
}

// char* builtin_generator(const char* text, int state){
//     static size_t list_index;
//     static size_t len;

//     if(!state){
//         list_index = 0;
//         len = strlen(text);
//     }

//     while(list_index < builtin_candidates.size()){
//         const string &name = builtin_candidates[list_index];
//         list_index++;

//         if(strncmp(name.c_str(), text, len) == 0){
//             // Readline requires a malloc'd string
//             return strdup(name.c_str());
//         }
//     }
//     return nullptr;
// }

char** shell_completion(const char* text, int start, int end){
    if(start == 0){
        // rl_completion_matches : It searches
        return rl_completion_matches(text, command_generator);
    }
    return nullptr; 
}