#include "base.hpp"
#include "data.hpp"
#include "game.hpp"
#include "resources.hpp"

int main(int argc, const char** argv) {
    (void) argc;
    (void) argv;
    
    Game::init();
    Log::info("hello there ", 3, " ", 4);

    //string text = readFileString("assets/entities/cow.td");
    //cout << text << "\n\n";
    //DataEntry* de = DataEntry::readText(text);
    //de->prettyPrint(cout); cout << "\n";
    //return 0;

    Game::run();
    Game::destory();
    return 0;
}