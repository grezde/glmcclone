#include "base.hpp"
#include "data.hpp"
#include "game.hpp"
#include "resources.hpp"

int main(int argc, const char** argv) {
    (void) argc;
    (void) argv;
    
    // string text = readFileString("assets/blocks/log.td");
    // cout << text << "\n\n";
    // DataEntry* de = DataEntry::readText(text);
    // de->prettyPrint(cout); cout << "\n";
    // return 0;

    Game::init();
    Game::run();
    Game::destory();
    return 0;
}