#include "base.hpp"
#include "data.hpp"
#include "game.hpp"

int main(int argc, const char** argv) {
    (void) argc;
    (void) argv;
    
    string text = readFileString("assets/blocks/log.bd");
    cout << text << "\n\n";
    DataEntry* de = DataEntry::readText(text);
    de->prettyPrint(cout); cout << "\n";
    return 0;

    Game g;
    g.init();
    g.run();
    g.destory();
    return 0;
}