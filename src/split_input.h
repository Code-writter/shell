#include<bits/stdc++.h>

using namespace std;

//Split string into tokens
vector<string> split_input(string input){
    vector<string> tokens;
    stringstream ss(input);

    string token;

    while(ss >> token){
        tokens.push_back(token);
    }
    return tokens;
}