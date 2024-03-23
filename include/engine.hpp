#pragma once
#include "base.hpp"
#include "resources.hpp"
#include <fstream>
#include <glm/vec2.hpp>

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
        inline const logger& operator()(Ts... args) const {
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
    extern const logger fatal;
    extern const logger error;
    extern const logger warning;
    extern const logger info;
};

namespace Input {
    extern bool disabledCursor;
    extern vec2 mousePos;
    extern vec2 mouseDiff;

    void toggleCursor();
    bool isPressed(i32 key);

    struct EventDescription {
        enum Type : u8 {
            KEY_DOWN,
            KEY_UP,
            KEY_PRESS, // with repeats
            KEY_IS_PRESSED // continous
        };
        Type type;
        u32 glfwKey;
        u8 currentState;

        EventDescription(Type type, u32 glfwKey) : type(type), glfwKey(glfwKey), currentState(0) {}
        void update();
        bool happened();
    };
    extern registry<EventDescription> events;

    void init();
    void processInput();
};