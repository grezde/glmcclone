#pragma once
#include "base.hpp"
#include <fstream>
#include <glm/ext/vector_int2.hpp>

namespace Log {
    extern std::ofstream logfile;
    #ifdef WINDOWS
        constexpr const char* endline = "\r\n";
    #else
        constexpr const char* endline = "\n";
    #endif
    void init();
    void changeColor(u8 color);
    void resetColor();
    struct logger {
        const u8 color;
        const char* prefix;
        const bool shouldExit;
        template<typename... Ts>
        inline logger& operator()(Ts... args) {
            changeColor(color);
            cerr << prefix;
            resetColor();
            cerr << ": "; 
            (cerr << ... << args);
            cerr << endline;
            logfile << prefix << ": ";
            (logfile << ... << args);
            logfile << endline;
            if(shouldExit) exit(1);
            return *this;
        }
    };
    extern logger fatal;
    extern logger error;
    extern logger warning;
    extern logger info;
};

namespace Controller {
    extern glm::ivec2 mousePos;
    extern glm::ivec2 mouseDif;

    void processInput();
};