#include<bits/stdc++.h>

using namespace std;

vector<string> split_input(string input) {
    vector<string> args;
    string current_arg;
    char state = 0; // 0 = None, ' = Single Quote, " = Double Quote
    
    auto push_arg = [&](){
        if(!current_arg.empty()){
            args.push_back(current_arg);
            current_arg.clear();
        }
    };

    for (size_t i = 0; i < input.length(); i++) {
        char c = input[i];

        if(state == 0 && c == '|'){
            // Found a pipe outside of 
            push_arg(); 
            args.push_back("|");
            continue;
        }

        if (state == '\'') {
            // --- SINGLE QUOTE MODE ---
            // Everything is literal, except the closing quote.
            if (c == '\'') {
                state = 0; // End Quote
            } else {
                current_arg += c;
            }
        }
        else if (state == '"') {
            // --- DOUBLE QUOTE MODE ---
            // Only escape specific characters: \, ", $, newline
            if (c == '"') {
                state = 0; // End Quote
            }
            else if (c == '\\') {
                // Check the next character
                if (i + 1 < input.length()) {
                    char next = input[i + 1];
                    // The prompt lists these as special inside double quotes
                    if (next == '\\' || next == '"' || next == '$' || next == '\n') {
                        current_arg += next; // Add the escaped char
                        i++; // Skip the backslash
                    } else {
                        current_arg += c; // Keep backslash literal (e.g., \a -> \a)
                    }
                } else {
                    current_arg += c; // Trailing backslash
                }
            }
            else {
                current_arg += c;
            }
        }
        else {
            // --- NO QUOTE MODE ---
            if (c == '\\') {
                // Outside quotes: Backslash escapes EVERYTHING
                if (i + 1 < input.length()) {
                    current_arg += input[i + 1];
                    i++;
                }
            }
            else if (c == '\'') {
                state = '\''; // Enter Single Quote Mode
            }
            else if (c == '"') {
                state = '"'; // Enter Double Quote Mode
            }
            else if (c == ' ') {
                push_arg();
            }
            else {
                current_arg += c;
            }
        }
    }
    push_arg();
    return args;
}


// vector<string> split_input(string input) {
//     vector<string> args;
//     string current_arg;
//     char quote_char = 0; // 0 = None, ' = Single, " = Double
    
//     for (size_t i = 0; i < input.length(); i++) {
//         char c = input[i];
        
//         // --- NEW LOGIC: Backslash handling ---
//         // Only handle backslash if we are NOT inside quotes
//         if (c == '\\' && quote_char == 0) {
//             // Ensure we don't overflow if '\' is the very last char
//             if (i + 1 < input.length()) {
//                 i++; // Skip the backslash
//                 current_arg += input[i]; // Add the next char literally
//             }
//             continue; // Move to next iteration
//         }

//         // --- Standard Quote Logic ---
//         if (c == '\'' || c == '"') {
//             if (quote_char == 0) {
//                 quote_char = c; // Start quoting
//             } 
//             else if (quote_char == c) {
//                 quote_char = 0; // End quoting
//             } 
//             else {
//                 current_arg += c; // Quote inside a different quote (e.g. " ' ")
//             }
//         } 
//         // --- Space Logic ---
//         else if (c == ' ') {
//             if (quote_char != 0) {
//                 current_arg += c; // Space inside quotes
//             } else {
//                 // Space outside quotes -> New Argument
//                 if (!current_arg.empty()) {
//                     args.push_back(current_arg);
//                     current_arg.clear();
//                 }
//             }
//         } 
//         // --- Normal Character ---
//         else {
//             current_arg += c;
//         }
//     }
    
//     if (!current_arg.empty()) {
//         args.push_back(current_arg);
//     }
    
//     return args;
// }

// //Split string into tokens
// vector<string> split_input(string input) {
//     vector<string> args;
//     string current_arg;
//     char quote_char = 0; // 0 = None, ' = Single, " = Double
    
//     for (size_t i = 0; i < input.length(); i++) {
//         char c = input[i];
        
//         if (c == '\'' || c == '"') {
//             if (quote_char == 0) {
//                 // START of a quoted section
//                 quote_char = c;
//             } 
//             else if (quote_char == c) {
//                 // END of the quoted section (matching quote found)
//                 quote_char = 0;
//             } 
//             else {
//                 // We are inside quotes, but found a DIFFERENT quote type.
//                 // Example: echo "shell's"
//                 // We are in ", we found '. Treat it as literal text.
//                 current_arg += c;
//             }
//         } 
//         else if (c == ' ') {
//             if (quote_char != 0) {
//                 // Inside quotes: Space is literal text
//                 current_arg += c;
//             } else {
//                 // Outside quotes: Space is a separator
//                 if (!current_arg.empty()) {
//                     args.push_back(current_arg);
//                     current_arg.clear();
//                 }
//             }
//         } 
//         else {
//             // Normal character
//             current_arg += c;
//         }
//     }
    
//     // Push the last argument if exists
//     if (!current_arg.empty()) {
//         args.push_back(current_arg);
//     }
    
//     return args;
// }




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