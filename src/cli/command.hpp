#include "../db/database.hpp"
#include <iostream>
#include <string>

struct Command {
    virtual std::string execute(Database& db) = 0;
    virtual ~Command() = default;
};

class PutCommand : public Command {
public:
    PutCommand(std::string key, std::string value)
        : key_(std::move(key)), value_(std::move(value)) {}

    std::string execute(Database& db) override {
        db.put(key_, value_);
        return "Inserted " + key_ + ": " + value_ + "\n";
    }

private:
    std::string key_;
    std::string value_;
};

class GetCommand : public Command {
public:
    GetCommand(std::string key) : key_(std::move(key)) {}

    std::string execute(Database& db) override {
        std::string res;
        if (key_ == "all") {
            auto result = db.getRange(-1);
            for (const auto& [key, val] : result) {
                res += key + ": " + val + "\n";
                std::cout << key << ": " << val << "\n";
            }
            return res;
        }
        auto result = db.get(key_);
        if (result) {
            return key_ + ": " + *result + "\n";
        } else {
            return key_ + " not found\n";
        }
    }

private:
    std::string key_;
};

class RemoveCommand : public Command {
public:
    RemoveCommand(std::string key) : key_(std::move(key)) {}

    std::string execute(Database& db) override {
        db.remove(key_);
        return "OK\n";
    }

private:
    std::string key_;
};