#include <random>
#include <sstream>

std::string generateUUID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    const char* hex = "0123456789abcdef";
    std::stringstream ss;

    for (int i = 0; i < 32; ++i) {
        ss << hex[dis(gen)];
        if (i == 7 || i == 11 || i == 15 || i == 19) ss << "-";
    }
    return ss.str();
}