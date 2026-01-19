#include<bits/stdc++.h>

using namespace std;

//Split string into tokens
vector<string> split_input(string input){
    vector<string> args;
    string current_arg;

    bool in_quotes = false;

    for(size_t i = 0; i<input.length(); i++){
        char c = input[i];

        if(c == '\''){
            in_quotes = !in_quotes;

        }else if(c == ' ' && !in_quotes){
            if(!current_arg.empty()){
                args.push_back(current_arg);
                current_arg.clear();
            }
        }else{
            current_arg += c;
        }

    }
    
    if(!current_arg.empty()){
        args.push_back(current_arg);
    }
    return args;
}





// //Split string into tokens
// vector<string> split_input(string input){
//     vector<string> tokens;
//     stringstream ss(input);

//     string token;

//     while(ss >> token){
//         tokens.push_back(token);
//     }
//     return tokens;
// }