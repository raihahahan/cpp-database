#include "../db/database.hpp"
#include "../storage/lsm/engine/lsm_engine.hpp"
#include "command.hpp"
#include <memory>
#include <iostream>
#include <fstream>

constexpr const char* HELP_TEXT = R"(Available commands:

put <key> <value>       - Insert or update a key-value pair
get <key>               - Retrieve the value for a given key
del <key>               - Delete the specified key
get all                 - Retrieves all key-value pairs
help                    - Show this help message
exit                    - Quit the CLI
)";

void printHelp(const std::string& helpFilePath = "help.txt") {
    std::cout << HELP_TEXT;
};

int main() {
    printHelp();
    Database db(std::make_unique<LSMEngine>());
    std::string cmd;
    while (std::cout << "> ", std::cin >> cmd) {
        if (cmd == "put") {
            std::string key, value;
            std::cin >> key >> value;
            PutCommand(key, value).execute(db);
        } else if (cmd == "get") {
            std::string key;
            std::cin >> key;
            GetCommand(key).execute(db);
        } else if (cmd == "del") {
            std::string key;
            std::cin >> key;
            RemoveCommand(key).execute(db);
        } else if (cmd == "exit") {
            break;
        } else if (cmd == "help") {
            printHelp();
            break;
        } else {
            std::cout << "Unknown command\n";
        }
    }
}
