#include "base.hpp"
#include "data.hpp"
#include <fstream>

void help() {
    cout << "Usage: datatool (convert|bundle) [input file] [output file]\n";
    cout << "  convert - converts between text data and binary data representations\n";
    cout << "  bundle - bundle a folder into a binary data file\n";
}

i32 convert(string inputFilename, string outputFilename) {
    FileEntry fe = { true, inputFilename };
    if(fe.hasExtension("td")) {
        DataEntry* de = DataEntry::readText(readFileString(inputFilename.c_str()));
        if(de->type == DataEntry::ERROR)
            ERR_EXIT("PARSE ERROR: " << de->error.message);
        FILE* fout = fopen(outputFilename.c_str(), "wb");
        if(!fout) ERR_EXIT("Cannot write into file " << outputFilename);
        de->writeBinary(fout);
        fclose(fout);
    }
    else if(fe.hasExtension("bd")) {
        FILE* fin = fopen(inputFilename.c_str(), "rb");
        DataEntry* de = DataEntry::readBinary(fin);
        if(de->type == DataEntry::ERROR)
            ERR_EXIT("PARSE ERROR: " << de->error.message);
        fclose(fin);
        
        std::ofstream fout(outputFilename.c_str());
        if(!fout.is_open()) ERR_EXIT("Cannot write into file " << outputFilename);
        de->prettyPrint(fout);
        fout.close();
    }
    else ERR_EXIT("File " << inputFilename << " has no data extension");
    return 0;
}

i32 bundle(string input, string output) {
    ERR_EXIT("Not yet implemented");
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