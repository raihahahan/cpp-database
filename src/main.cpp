#include "db/database.hpp"
#include "storage/lsm/engine/lsm_engine.hpp"
#include <iostream>

int main() {
    auto engine = std::make_unique<LSMEngine>();
    Database db(std::move(engine));

    db.put("hello", "world");
    auto result = db.get("hello");
    std::cout << "Get result: " << result.value_or("not found") << "\n";

    return 0;
}
