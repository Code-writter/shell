#include<bits/stdc++.h>
#include<cstdlib> // For getenv
#include<unistd.h> // For access(), X_OK
#include<string>

using namespace std;

// Get the file path within given command
string get_path(string target_file) {
    string path_env = getenv("PATH");
    if (path_env.empty()) return "";

    stringstream ss(path_env);
    string path_dir;

    while (getline(ss, path_dir, ':')) {
        string abs_path = path_dir + "/" + target_file;
        
        // Check if file exists and is executable
        if (access(abs_path.c_str(), X_OK) == 0) {
            return abs_path; // Found it! Return immediately.
        }
    }
    // If the loop finishes without returning, the file wasn't found anywhere.
    return "";
}
