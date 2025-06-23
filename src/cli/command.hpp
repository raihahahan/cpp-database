#include "../db/database.hpp"
#include <iostream>

struct Command {
    virtual void execute(Database& db) = 0;
    virtual ~Command() = default;
};

class PutCommand : public Command {
public:
    PutCommand(std::string key, std::string value)
        : key_(std::move(key)), value_(std::move(value)) {}

    void execute(Database& db) override {
        db.put(key_, value_);
    }

private:
    std::string key_;
    std::string value_;
};

class GetCommand : public Command {
public:
    GetCommand(std::string key) : key_(std::move(key)) {}

    void execute(Database& db) override {
        if (key_ == "all") {
            auto result = db.getRange(-1);
            for (const auto& [key, val] : result) {
                std::cout << key << ": " << val << "\n";
            }
            return;
        }
        auto result = db.get(key_);
        if (result) {
            std::cout << key_ << ": " << *result << "\n";
        } else {
            std::cout << key_ << " not found\n";
        }
    }

private:
    std::string key_;
};

class RemoveCommand : public Command {
public:
    RemoveCommand(std::string key) : key_(std::move(key)) {}

    void execute(Database& db) override {
        db.remove(key_);
    }

private:
    std::string key_;
};