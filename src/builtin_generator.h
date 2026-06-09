#include<bits/stdc++.h>

// Readline headers for tab completions
#include<stdio.h>
#include<readline/readline.h>
#include<readline/history.h>

// File system
#include<filesystem>
#include<memory> // For popen
#include<array>  // For popen buffer

using namespace std;
namespace fs = std::filesystem;

// External reference to the history stored in main.cpp
extern vector<string> command_history;

vector<string> builtin_candidates = {"echo", "exit", "type", "pwd", "cd", "history"};
vector<string> matches; // Chache matches during autocompletion

// The Generator: Populates 'matches' on state 0, then feeds them back
char* command_generator(const char* text, int state) {
    if (state == 0) {
        matches.clear();
        string text_str(text);
        set<string> found; 

        // 1. Scan Builtins
        for (const auto& cmd : builtin_candidates) {
            if (cmd.find(text_str) == 0) { 
                matches.push_back(cmd);
                found.insert(cmd);
            }
        }

        // 2. Scan PATH
        string path_env = getenv("PATH");
        if(path_env.empty()) return nullptr;

        stringstream ss(path_env);
        string path_dir;

        while (getline(ss, path_dir, ':')) {
            if (!fs::exists(path_dir) || !fs::is_directory(path_dir)) {
                continue;
            }

            try {
                for (const auto& entry : fs::directory_iterator(path_dir)) {
                    string filename = entry.path().filename().string();
                    
                    if (filename.find(text_str) == 0) {
                        if (found.find(filename) == found.end()) {
                            if (access(entry.path().c_str(), X_OK) == 0) {
                                matches.push_back(filename);
                                found.insert(filename);
                            }
                        }
                    }
                }
            } catch (...) {
            }
        }
    }

    if (state < matches.size()) {
        return strdup(matches[state].c_str());
    }
    return nullptr;
}


// --- NEW: AI Autocomplete Helper ---
string get_ai_autocomplete(const string& current_line) {
    // 1. Gather recent history
    string recent_history = "";
    int start_idx = max(0, (int)command_history.size() - 5);
    for (int i = start_idx; i < command_history.size(); i++) {
        recent_history += command_history[i] + "\n";
    }

    // 2. Escape quotes for shell safety
    auto escape_quotes = [](const string& str) {
        string escaped;
        for (char c : str) {
            if (c == '"' || c == '\\' || c == '`' || c == '$') escaped += '\\';
            escaped += c;
        }
        return escaped;
    };

    // 3. Construct the popen command (Ensure the python script path is correct!)
    string script_path = std::filesystem::current_path().string() + "/ai_agent.py"; 
    string system_cmd = "python3 " + script_path + " --autocomplete --history \"" + 
                        escape_quotes(recent_history) + "\" \"" + escape_quotes(current_line) + "\"";

    array<char, 128> buffer;
    string ai_suggestion;
    
    // 4. Call Python and read output
    unique_ptr<FILE, int(*)(FILE*)> pipe(popen(system_cmd.c_str(), "r"), pclose);
    if (pipe) {
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            ai_suggestion += buffer.data();
        }
    }
    
    // 5. Clean up any trailing newlines from the AI output
    ai_suggestion.erase(ai_suggestion.find_last_not_of(" \n\r\t") + 1);
    
    // Ignore if AI returned its error string
    if (ai_suggestion.find("AI Error:") != string::npos) {
        return "";
    }
    
    return ai_suggestion;
}


// --- UPDATED: Shell Completion Hook ---
// char** shell_completion(const char* text, int start, int end){
    
//     // 1. Try standard command completion first (if typing the first word)
//     if(start == 0){
//         char** standard_matches = rl_completion_matches(text, command_generator);
//         if (standard_matches != nullptr) {
//             return standard_matches; 
//         }
//     }

//     // 2. If standard completion yields nothing OR we are typing arguments, try AI!
//     string current_buffer = rl_line_buffer; 

//     if (!current_buffer.empty()) {
//         string ai_suggestion = get_ai_autocomplete(current_buffer);

//         if (!ai_suggestion.empty()) {
//             // Delete what the user currently typed
//             rl_delete_text(0, rl_end);
            
//             // Insert the AI's predicted command
//             rl_insert_text(ai_suggestion.c_str());
            
//             // Force the terminal to update visibly
//             rl_redisplay();
            
//             // Tell readline we handled everything, do not print files
//             rl_attempted_completion_over = 1; 
//             return nullptr;
//         }
//     }

//     // 3. Absolute fallback: if AI fails, default to standard file completion
//     return nullptr; 
// }


// --- UPDATED: Shell Completion Hook (No visual garbage) ---
char** shell_completion(const char* text, int start, int end){
    
    // 1. Try standard command completion first (if typing the first word)
    if(start == 0){
        char** standard_matches = rl_completion_matches(text, command_generator);
        if (standard_matches != nullptr) {
            return standard_matches; 
        }
    }

    // 2. Try AI Autocomplete
    string current_buffer = rl_line_buffer; 

    if (!current_buffer.empty()) {
        string ai_suggestion = get_ai_autocomplete(current_buffer);

        if (!ai_suggestion.empty()) {
            
            // 1. Safely wipe the current text in Readline's buffer
            rl_delete_text(0, rl_end);
            rl_point = 0; // Reset the internal cursor position
            
            // 2. Insert the AI's predicted command
            rl_insert_text(ai_suggestion.c_str());
            
            // 3. Force Readline to redraw the prompt and the new text cleanly
            rl_redisplay();
            
            // 4. Tell readline we handled everything
            rl_attempted_completion_over = 1; 
            return nullptr;
        }
    }

    // 3. Absolute fallback
    return nullptr; 
}