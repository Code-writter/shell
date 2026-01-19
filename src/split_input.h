#include<bits/stdc++.h>

using namespace std;

//Split string into tokens
vector<string> split_input(string input) {
    vector<string> args;
    string current_arg;
    char quote_char = 0; // 0 = None, ' = Single, " = Double
    
    for (size_t i = 0; i < input.length(); i++) {
        char c = input[i];
        
        if (c == '\'' || c == '"') {
            if (quote_char == 0) {
                // START of a quoted section
                quote_char = c;
            } 
            else if (quote_char == c) {
                // END of the quoted section (matching quote found)
                quote_char = 0;
            } 
            else {
                // We are inside quotes, but found a DIFFERENT quote type.
                // Example: echo "shell's"
                // We are in ", we found '. Treat it as literal text.
                current_arg += c;
            }
        } 
        else if (c == ' ') {
            if (quote_char != 0) {
                // Inside quotes: Space is literal text
                current_arg += c;
            } else {
                // Outside quotes: Space is a separator
                if (!current_arg.empty()) {
                    args.push_back(current_arg);
                    current_arg.clear();
                }
            }
        } 
        else {
            // Normal character
            current_arg += c;
        }
    }
    
    // Push the last argument if exists
    if (!current_arg.empty()) {
        args.push_back(current_arg);
    }
    
    return args;
}




// vector<string> split_input(string input){
//     vector<string> args;
//     string current_arg;

//     bool in_quotes = false;

//     for(size_t i = 0; i<input.length(); i++){
//         char c = input[i];

//         if(c == '\''){
//             in_quotes = !in_quotes;

//         }else if(c == ' ' && !in_quotes){
//             if(!current_arg.empty()){
//                 args.push_back(current_arg);
//                 current_arg.clear();
//             }
//         }else{
//             current_arg += c;
//         }

//     }
    
//     if(!current_arg.empty()){
//         args.push_back(current_arg);
//     }
//     return args;
// }





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