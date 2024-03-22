#pragma once
#include "renderer.hpp"
#include "world.hpp"
#include <fstream>

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
        u8 color;
        const char* prefix;
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
            return *this;
        }
    };
    extern logger fatal;
    extern logger error;
    extern logger warning;
    extern logger info;
};

namespace Game {
    extern bool inspectMode;
    extern bool printFPS;
    extern World testWorld;
    extern glm::vec2 cameraAngle;
    extern glm::vec3 cameraPos;
    extern glm::vec3 cameraVel;

    void init();
    void run();
    void destory();

    void processInput(f32 dt);
};