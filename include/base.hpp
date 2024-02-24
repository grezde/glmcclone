#pragma once
#include <iostream>
#include <ostream>
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

#ifdef DEBUG
    inline string removeSourceBeggining(string filename) { return filename.substr(42); }
    #define ERR_EXIT(x) do { \
            cerr << "FATAL ERROR: " << x << "\nAt " << removeSourceBeggining(__FILE__) << ":" << __LINE__ << "in " << __PRETTY_FUNCTION__ << "\n";\
            exit(1);\
    } while(0)
#else
    #define ERR_EXIT(x) do{ cerr << "FATAL ERROR: " << x << "\n"; exit(1); } while(0)
#endif

vector<u8> readWholeFile(const char* filename);