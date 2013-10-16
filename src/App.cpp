#include <string>
#include <iostream>
#include "Workspace.h"
#include "Strategy.h"

int main(int argc, char* argv[])
{
    try
    {
        // parse the command line args
        if ( argc != 3 )
        {
            throw std::invalid_argument(std::string("usage: ") + argv[0] + " CFG_FILE SYMBOL");
        }
        const std::string cfgFileName = argv[1];
        const std::string symbol = argv[2];

        // create our Strategy and connect it to our Workspace
        VwapStrategy vwapStrategy(symbol);
        Workspace workspace(vwapStrategy);
        workspace.init(cfgFileName);

        // quickfix is now running in the background, so we just wait here until the user hits ENTER to shut us down
        std::cout << std::endl << "Hit [ENTER] to quit..." << std::endl;
        std::string input;
        std::getline(std::cin, input);
    }
    catch(const std::exception & e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}

