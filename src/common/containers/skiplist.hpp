#pragma once
#include <vector>
#include <memory>
#include <random>
#include <limits>
#include <optional>

/**
 * 1. base layer is a fully sorted linked list
 * 2. each higher level is a sparser version that "skips" nodes
 * 3. each node has multiple forward pointers
 * 4. on average, O(log n) search/insert/delete
 */
template<typename K, typename V>
class SkipList {
private:
    struct Node {
        K key;
        V value;
        std::vector<std::shared_ptr<Node>> forward; // forward pointers for each level

        Node(const K& key, const V& value, int level)
            : key(key), value(value), forward(level, nullptr) {}
    };

    static constexpr int MAX_LEVEL = 16; // max levels in the skip list
    const float probability = 0.5f; // probability to promote level
    int level = 1; // current highest level in list

    std::shared_ptr<Node> head; // head node with MAX_LEVEL pointers

    // rng to decide height of new node
    std::mt19937 gen{std::random_device{}()};
    std::uniform_real_distribution<> dist{0.0, 1.0};

    // randomly determine the level of a new node.
    int randomLevel() {
        int lvl = 1;
        while (dist(gen) < probability && lvl < MAX_LEVEL)
            ++lvl;
        return lvl;
    }

public:
    SkipList() {
        head = std::make_shared<Node>(K{}, V{}, MAX_LEVEL);
    }

    void insert(const K& key, const V& value) {
        // overview:
        // 1. find where the key should be inserted
        //  - for each level from the top, traverse until exceed or end
        //  - after each level, add to update[level] = last node traversed
        // 2. randomly pick a level for key
        // 3. insert key at all levels below this level
        //  - eg random=2, insert into levels 0 and 1
        //  - use update[level]->forward[level] = newNode

        // track last node seen at each level before inserting the new node
        // update[i] = last node at level i whose forward pointer might need
        //             to be updated when inserting a new node
        std::vector<std::shared_ptr<Node>> update(MAX_LEVEL);
        auto curr = head;

        // traverse from top level to level 0 to find insert position
        for (int i = level - 1; i >= 0; --i) {
            while (curr->forward[i] && curr->forward[i]->key < key) {
                curr = curr->forward[i];
            }
            update[i] = curr; // remember nodes before insertion point
        }
        
        // point curr to the first candidate node at level 0 >= key
        curr = curr->forward[0];

        if (curr && curr->key == key) {
            // key exists: overwrite value
            curr->value = value;
        } else {
            // key not exist: insert new node
            int newLevel = randomLevel();

            // extend the current max level if needed
            if (newLevel > level) {
                for (int i = level; i < newLevel; ++i) {
                    // each new level starts from the head
                    update[i] = head;
                }
                level = newLevel;
            }

            auto newNode = std::make_shared<Node>(key, value, newLevel);

            // splice in the new node at each level
            // curr: update[i]->prev_forward[i]
            // need: update[i]->newNode->prev_forward[i]
            for (int i = 0; i < newLevel; ++i) {
                newNode->forward[i] = update[i]->forward[i];
                update[i]->forward[i] = newNode;
            }
        }
    }

    std::optional<V> get(const K& key) const {
        auto curr = head;

        // traverse from top level to level 0
        for (int i = level - 1; i >= 0; --i) {
            while (curr->forward[i] && curr->forward[i]->key < key) {
                curr = curr->forward[i];
            }
        }

        curr = curr->forward[0];
        if (curr && curr->key == key) {
            return curr->value;
        }
        return std::nullopt;
    }

    //no-op if the key does not exist.
    void remove(const K& key) {
        std::vector<std::shared_ptr<Node>> update(MAX_LEVEL);
        auto curr = head;

        // find update path for each level
        for (int i = level - 1; i >= 0; --i) {
            while (curr->forward[i] && curr->forward[i]->key < key) {
                curr = curr->forward[i];
            }
            update[i] = curr;
        }

        curr = curr->forward[0];
        if (!curr || curr->key != key) return;

        // unlink the node from all levels
        for (int i = 0; i < level; ++i) {
            if (update[i]->forward[i] != curr) break;
            update[i]->forward[i] = curr->forward[i];
        }

        // decrease current level if needed
        while (level > 1 && !head->forward[level - 1]) {
            --level;
        }
    }

    // returns all key-value pairs in sorted order.
    std::vector<std::pair<K, V>> entries() const {
        std::vector<std::pair<K, V>> result;
        auto curr = head->forward[0];
        while (curr) {
            result.emplace_back(curr->key, curr->value);
            curr = curr->forward[0];
        }
        return result;
    }

    // returns the number of key-value entries.
    size_t size() const {
        size_t count = 0;
        auto curr = head->forward[0];
        while (curr) {
            ++count;
            curr = curr->forward[0];
        }
        return count;
    }
    
    void clear() {
        head = std::make_shared<Node>(K{}, V{}, MAX_LEVEL);
        level = 1;
    }
};
