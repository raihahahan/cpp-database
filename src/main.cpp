#include "db/database.hpp"
#include "storage/lsm/engine/lsm_engine.hpp"
#include "cli/command.hpp"
#include "config.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <csignal>
#include <iostream>
#include <memory>
#include <sstream>
#include <sys/stat.h>
#include <filesystem> 
#include <cstring>
#include <cerrno> 

#define LOCK_FILE "/tmp/kvdb.lock" // lock file for singleton implementation
#define LOG_FILE "daemon.log"
#define SOCKET_FILE "db.sock"

// global to hold the lock file descriptor, so it's not closed prematurely
int global_lock_fd = -1;

void cleanupAndExit(int signal_number) {
    std::cout << "\n[Daemon] Received signal " << signal_number << ". Shutting down.\n";

    // clean up socket file
    if (unlink(SOCKET_FILE) != 0) {
        std::cerr << "Warning: Could not remove socket file '" << SOCKET_FILE << "': " << strerror(errno) << std::endl;
    }

    // explicitly close the lock file descriptor to release the lock
    if (global_lock_fd != -1) {
        if (close(global_lock_fd) != 0) {
            std::cerr << "Warning: Could not close lock file descriptor: " << strerror(errno) << std::endl;
        }
    }
    exit(EXIT_SUCCESS);
}

// ensure the lock file is created and locked by the daemon process.
int ensureSingleInstance() {
    int fd = open(LOCK_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR); // permissions 0600 (rw- --- ---)
    if (fd < 0) {
        std::cerr << "Error: Failed to open lock file '" << LOCK_FILE << "': " << strerror(errno) << std::endl;
        return -1;
    }

    // try to acquire exclusive lock on the entire file
    if (lockf(fd, F_TLOCK, 0) != 0) {
        if (errno == EACCES || errno == EAGAIN) {
            std::cerr << "Another instance of db_main is already running. Exiting." << std::endl;
        } else {
            std::cerr << "Error: Could not lock file '" << LOCK_FILE << "': " << strerror(errno) << std::endl;
        }
        close(fd);
        return -1;
    }

    // if we reach here, we successfully acquired the lock!!!

    // write PID into the lock file for informational purposes
    if (ftruncate(fd, 0) != 0) {
        std::cerr << "Warning: Could not truncate lock file '" << LOCK_FILE << "': " << strerror(errno) << std::endl;
    }

    if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
        std::cerr << "Warning: Could not seek to beginning of lock file '" << LOCK_FILE << "': " << strerror(errno) << std::endl;
    }

    std::string pidStr = std::to_string(getpid()) + "\n";
    ssize_t bytesWritten = write(fd, pidStr.c_str(), pidStr.size());
    if (bytesWritten != (ssize_t)pidStr.size()) {
        std::cerr << "Warning: Could not write PID to lock file '" << LOCK_FILE << "': " << strerror(errno) << std::endl;
    }

    // keep the file descriptor open.
    return fd;
}

void daemonize(const std::string& current_base_path) {
    std::cout << "Starting daemon\n";
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS); // parent exits

    // this (daemon) is the child process
    setsid(); // create a new session and become the session leader
    umask(0); // clear file mode creation mask

    // change current working directory to the base path
    if (chdir(current_base_path.c_str()) != 0) {
        std::cerr << "Error: Could not change directory to '" << current_base_path << "': " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    
    std::cout << "METADATA:\n";
    std::cout << "Base path: " << current_base_path << std::endl;
    std::cout << "Log file: " << current_base_path << "/" << LOG_FILE << std::endl;
    std::cout << "Socket file: " << current_base_path << SOCKET_FILE << "\n\n";
    
    std::cout << "COMMANDS:\n";
    std::cout << "CLI: ./db_cli\n";
    std::cout << "End process: pkill db_main\n";
    std::cout << "Check process: ps aux | grep db_main\n\n";
    
    std::cout << "NOTE: If you have already called ./db_main before this, no new process will be created from this.\n";

    // close standard descriptors
    close(STDIN_FILENO);

    // redirect stdout and stderr to logs
    int out = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (out < 0) {
        // fallback: print to original stderr if log file can't be opened
        perror("Error opening log file");
        exit(EXIT_FAILURE);
    }
    dup2(out, STDOUT_FILENO);
    dup2(out, STDERR_FILENO);
    close(out);
}

void handleClient(int clientSock, Database& db) {
    char buffer[1024];
    ssize_t bytesRead = read(clientSock, buffer, sizeof(buffer) - 1);
    if (bytesRead <= 0) return;

    buffer[bytesRead] = '\0';
    std::istringstream iss(buffer);
    std::ostringstream response;
    std::string cmd;
    iss >> cmd;

    if (cmd == "put") {
        std::string key, value;
        iss >> key >> value;
        response << PutCommand(key, value).execute(db);
    } else if (cmd == "get") {
        std::string key;
        iss >> key;
        response << GetCommand(key).execute(db);
    } else if (cmd == "del") {
        std::string key;
        iss >> key;
        response << RemoveCommand(key).execute(db);
    } else if (cmd == "getall") {
        for (const auto& [k, v] : db.getRange()) {
            response << k << ": " << v << "\n";
        }
    } else {
        response << "Unknown command\n";
    }

    std::string respStr = response.str();
    write(clientSock, respStr.c_str(), respStr.size());
}


int main() {
    const std::string basePath = std::string(std::getenv("HOME")) + "/.kvdb";
    std::filesystem::create_directories(basePath);
    daemonize(basePath);

    // now we are in the child process (daemon)
    // acquire the single instance lock
    global_lock_fd = ensureSingleInstance();
    if (global_lock_fd == -1) {
        // if -1, then another instance is running
        // or there was an error
        return EXIT_FAILURE;
    }

    signal(SIGINT, cleanupAndExit);
    signal(SIGTERM, cleanupAndExit);
    signal(SIGHUP, SIG_IGN);

    if (unlink(SOCKET_FILE) != 0 && errno != ENOENT) {
        std::cerr << "Warning: Could not remove old socket file '" << SOCKET_FILE << "': " << strerror(errno) << std::endl;
    }

    int serverSock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (serverSock < 0) {
        std::cerr << "Error: socket creation failed: " << strerror(errno) << std::endl;
        cleanupAndExit(EXIT_FAILURE);
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_FILE, sizeof(addr.sun_path) - 1);

    if (bind(serverSock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Error: socket bind failed: " << strerror(errno) << std::endl;
        close(serverSock);
        cleanupAndExit(EXIT_FAILURE);
    }

    if (listen(serverSock, 5) < 0) {
        std::cerr << "Error: socket listen failed: " << strerror(errno) << std::endl;
        close(serverSock);
        cleanupAndExit(EXIT_FAILURE);
    }

    // initialize DB
    Database db(std::make_unique<LSMEngine>());
    std::cout << "DB INITIALISED\n";

    while (true) {
        int clientSock = accept(serverSock, nullptr, nullptr);
        if (clientSock >= 0) {
            handleClient(clientSock, db);
            close(clientSock);
        } else {
            // handle accept errors, especially if a signal interrupts it
            if (errno == EINTR) {
                continue; // interrupted by a signal, try again
            }
            std::cerr << "Error: accept failed: " << strerror(errno) << std::endl;
        }
    }

    close(serverSock);
    return 0;
}