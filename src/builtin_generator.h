#include<bits/stdc++.h>

// Readline headers for tab completions
#include<stdio.h>
#include<readline/readline.h>


using namespace std;

vector<string> builtin_candidates = {"echo", "exit", "type", "pwd", "cd"};

char* builtin_generator(const char* text, int state){
    static size_t list_index;
    static size_t len;

    if(!state){
        list_index = 0;
        len = strlen(text);
    }

    while(list_index < builtin_candidates.size()){
        const string &name = builtin_candidates[list_index];
        list_index++;

        if(strncmp(name.c_str(), text, len) == 0){
            // Readline requires a malloc'd string
            return strdup(name.c_str());
        }
    }
    return nullptr;
}

char** shell_completion(const char* text, int start, int end){
    if(start == 0){
        // rl_completion_matches : It searches
        return rl_completion_matches(text, builtin_generator);
    }
    return nullptr; 
}