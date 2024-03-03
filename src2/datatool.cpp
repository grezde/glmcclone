#include "data.hpp"

void help() {
    cout << "Usage: datatool (convert|bundle) [input file] [output file]\n";
    cout << "  convert - converts between text data and binary data representations\n";
    cout << "  bundle - bundle a folder into a binary data file\n";
}

i32 convert(string input, string output) {
    return 0; 
}

i32 bundle(string input, string output) {
    return 0;
}

int main(int argc, const char** argv) {
    vector<string> args;
    for(i32 i=1; i<argc; i++)
        args.push_back(argv[i]);
    if(args.size() != 3) {
        cout << "ERROR: Wrong arguments\n";
        help();
        return 1;
    }
    if(args[0] == "convert")
        return convert(args[1], args[2]);
    if(args[0] == "bundle")
        return bundle(args[1], args[2]);
    cout << "ERROR: Wrong command\n";
    return 1;
}