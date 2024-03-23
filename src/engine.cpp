#include "engine.hpp"
#include "renderer.hpp"
#include "resources.hpp"


std::ofstream Log::logfile;
const Log::logger Log::fatal = logger {
    .color = 1,
    .prefix = "FATAL",
    .shouldExit = true
};
const Log::logger Log::error = logger {
    .color = 1,
    .prefix = "ERROR",
    .shouldExit = false
};
const Log::logger Log::warning = logger {
    .color = 1,
    .prefix = "WARN",
    .shouldExit = false
};
const Log::logger Log::info = logger {
    .color = 1,
    .prefix = "INFO",
    .shouldExit = false
};

#ifdef WINDOWS

#include <windows.h>

HANDLE hConsole;
CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
WORD saved_attributes;

void initConsole() {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
    saved_attributes = consoleInfo.wAttributes;
}

void Log::changeColor(u8 color) {
    SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
}

void Log::resetColor() {
    SetConsoleTextAttribute(hConsole, saved_attributes);
}

#else

void initConsole() {}

void Log::changeColor(u8 color) {
    (void) color;
    // TODO: implement colors in logging
    cerr << "\x1b[0;30m";
}

void Log::resetColor() {
    cerr << "\x1b[0m";
}

#endif

void Log::init() {
    initConsole();
    logfile.open("output/log.txt");
    if(!logfile.is_open()) ERR_EXIT("Cound not open log file");
}


bool Input::disabledCursor = false;
vec2 Input::mousePos = { 0, 0 };
vec2 Input::mouseDiff = { 0, 0 }; 
registry<Input::EventDescription> Input::events;
vec2 lastMousePos = {0, 0};

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    (void) window;
    Input::mousePos.x = xpos;
    Input::mousePos.y = ypos;
}

void Input::init() {
    glfwSetCursorPosCallback(window::window, cursor_position_callback);
    events.add("forward", EventDescription(EventDescription::KEY_IS_PRESSED, GLFW_KEY_W));
    events.add("backward", EventDescription(EventDescription::KEY_IS_PRESSED, GLFW_KEY_S));
    events.add("strafe_left", EventDescription(EventDescription::KEY_IS_PRESSED, GLFW_KEY_A));
    events.add("strafe_right", EventDescription(EventDescription::KEY_IS_PRESSED, GLFW_KEY_D));
    events.add("escape", EventDescription(EventDescription::KEY_DOWN, GLFW_KEY_ESCAPE));
    events.add("spectator_toggle", EventDescription(EventDescription::KEY_DOWN, GLFW_KEY_I));
}

bool Input::EventDescription::happened() {
    switch(type) {
        case KEY_DOWN: return currentState == 0b01;
        case KEY_UP:   return currentState == 0b10;
        case KEY_PRESS: return currentState & 0b01;
        case KEY_IS_PRESSED: return currentState & 0b01;
    }
    return false;
}

void Input::EventDescription::update() {
    currentState = ((currentState << 1) | isPressed(glfwKey)) & 0b11;
}

void Input::processInput() {
    mouseDiff = mousePos - lastMousePos;
    lastMousePos = mousePos;
    for(EventDescription& ev : events.items)
        ev.update();
}

void Input::toggleCursor() {
    disabledCursor = !disabledCursor;
    glfwSetInputMode(window::window, GLFW_CURSOR, disabledCursor ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

bool Input::isPressed(i32 key) {
    return glfwGetKey(window::window, key) == GLFW_PRESS;
}