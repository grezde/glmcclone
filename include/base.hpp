#pragma once
#include <iostream>
#include <ostream>
#include <system_error>
#include <vector>
#include <map>

#ifdef DEBUG
    #include <signal.h>
    #define DEBUGGER raise(SIGTRAP)
#endif

typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef float f32;
typedef double f64;
using std::string;
using std::vector;
using std::cout;
using std::cerr;
using std::ostream;
using std::map;
using std::pair;

#ifdef DEBUG
    inline string removeSourceBeggining(string filename) { return filename.substr(0); }
    #define ERR_EXIT(x) do { \
            cerr << "FATAL ERROR: " << x << "\nAt " << removeSourceBeggining(__FILE__) << ":" << __LINE__ << " in " << __PRETTY_FUNCTION__ << "\n";\
            exit(1);\
    } while(0)
#else
    #define ERR_EXIT(x) do{ cerr << "FATAL ERROR: " << x << "\n"; exit(1); } while(0)
#endif

// FILE SYSTEM

vector<u8> readFileBytes(const char* filename);
string readFileString(const char* filename);
bool fileExists(const char* filename);

struct FileEntry {
    bool isDir;
    string name;
    bool hasExtension(const string& extension) const;
    void removeExtension(u32 extensionSize);
};
vector<FileEntry> readFolder(const char* name);
vector<FileEntry> readFolderRecursively(const char* name);