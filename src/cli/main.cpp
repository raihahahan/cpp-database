#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <sstream>

#define SOCKET_PATH "/.kvdb/db.sock"
const std::string sockPath = std::string(std::getenv("HOME")) + SOCKET_PATH;
constexpr const char* HELP_TEXT = R"(Available commands:

put <key> <value>       - Insert or update a key-value pair
get <key>               - Retrieve the value for a given key
del <key>               - Delete the specified key
getall                  - Retrieves all key-value pairs
help                    - Show this help message
exit                    - Quit the CLI
)";

void printHelp() {
    std::cout << HELP_TEXT << std::endl;
}

std::string sendCommandToServer(const std::string& command) {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return "ERROR: Failed to create socket.\n";
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, sockPath.c_str(), sizeof(addr.sun_path) - 1);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sock);
        return "ERROR: Failed to connect to server.\n";
    }

    // Send command
    if (write(sock, command.c_str(), command.size()) < 0) {
        perror("write");
        close(sock);
        return "ERROR: Failed to send command.\n";
    }

    // Read response
    char buffer[1024];
    std::ostringstream response;
    ssize_t bytesRead;
    while ((bytesRead = read(sock, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        response << buffer;
        if (bytesRead < static_cast<ssize_t>(sizeof(buffer) - 1)) break;
    }

    close(sock);
    return response.str();
}

int main() {
    printHelp();

    std::string cmd;
    while (std::cout << "> ", std::cin >> cmd) {
        std::ostringstream oss;
        if (cmd == "put") {
            std::string key, value;
            std::cin >> key >> value;
            oss << "put " << key << " " << value << "\n";
        } else if (cmd == "get") {
            std::string key;
            std::cin >> key;
            oss << "get " << key << "\n";
        } else if (cmd == "del") {
            std::string key;
            std::cin >> key;
            oss << "del " << key << "\n";
        } else if (cmd == "getall") {
            oss << "getall\n";
        } else if (cmd == "help") {
            printHelp();
            continue;
        } else if (cmd == "exit") {
            break;
        } else {
            std::cout << "Unknown command\n";
            continue;
        }

        // send and print response
        std::string response = sendCommandToServer(oss.str());
        std::cout << response;
    }

    return 0;
}
