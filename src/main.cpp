#include "game.hpp"

int main(int argc, const char** argv) {
    (void) argc;
    (void) argv;
    
    Game g;
    g.init();
    g.run();
    g.destory();
    return 0;
}