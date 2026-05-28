#include <iostream>
#include <vector>
#include <list>
#include <chrono>
#include <fstream>
#include <cmath>
#include <random>
#include <string>

using namespace std;

// Simple key-value pair used by hash table buckets and probe slots
template <typename K, typename V>
struct KeyValuePair {
    K key;
    V value;

    KeyValuePair(K k, V v) : key(k), value(v) {}
};

// Primary hash for integers: simple modulo by table size
size_t hashInt(int key, size_t tableSize) {
    return key % tableSize;
}

// Simple string hash (multiplicative 31), reduced modulo table size
size_t hashString(const string& key, size_t tableSize) {
    size_t hash = 0;
    for (char c : key) {
        hash = (hash * 31 + c) % tableSize;
    }
    return hash;
}

// Secondary hash for double hashing: must be non-zero and relatively small
size_t secondHash(int key, size_t tableSize) {
    size_t prime = tableSize > 3 ? tableSize - 2 : 1;
    return 1 + (key % prime);
}

// Separate chaining hash table using std::list for buckets
class SeparateChaining {
private:
    vector<list<KeyValuePair<int, string>>> table;
    size_t size;
    size_t count;

public:
    SeparateChaining(size_t initialSize) : size(initialSize), count(0) {
        table.resize(size);
    }

    void insert(int key, const string& value) {
        // compute bucket index and append if key not present
        size_t index = hashInt(key, size);

        // walk the bucket at `index` to check for existing key
        // - if found, update the value (no change to `count`)
        // - otherwise append a new KeyValuePair and increment `count`
        for (auto& pair : table[index]) {
            if (pair.key == key) {
                pair.value = value; // update existing entry
                return;
            }
        }

        // not found: append to the end of the bucket list
        table[index].push_back(KeyValuePair<int, string>(key, value));
        count++;
    }

    bool search(int key, string& value) {
        // scan the bucket for the key
        size_t index = hashInt(key, size);

        // iterate over entries in the bucket; buckets are typically short
        for (const auto& pair : table[index]) {
            if (pair.key == key) {
                value = pair.value; // copy out the found value
                return true;
            }
        }

        // not found in bucket
        return false;
    }

    double getLoadFactor() const {
        // current load factor = items / table slots
        return static_cast<double>(count) / size;
    }
};

// Open addressing with linear probing and a DELETED sentinel
class LinearProbing {
private:
    vector<KeyValuePair<int, string>*> table;
    size_t size;
    size_t count;

    KeyValuePair<int, string>* DELETED;

public:
    LinearProbing(size_t initialSize) : size(initialSize), count(0) {
        table.resize(size, nullptr);
        DELETED = new KeyValuePair<int, string>(-1, "DELETED");
    }

    ~LinearProbing() {
        for (auto ptr : table) {
            if (ptr != nullptr && ptr != DELETED) {
                delete ptr;
            }
        }
        delete DELETED;
    }

    void insert(int key, const string& value) {
        if (getLoadFactor() > 0.7) {
            rehash();
        }

        // Probe linearly from the initial index. Linear probing sequence:
        // index, index+1, index+2, ... (mod table size). We stop when we
        // find a nullptr (never used) or a DELETED marker (previously removed).
        size_t index = hashInt(key, size);
        size_t i = 0;

        while (i < size) {
            size_t currentIndex = (index + i) % size;

            // empty or reclaimed slot -> insert new entry here
            if (table[currentIndex] == nullptr || table[currentIndex] == DELETED) {
                table[currentIndex] = new KeyValuePair<int, string>(key, value);
                count++;
                return;
            } else if (table[currentIndex]->key == key) {
                // key already present in-table; update the value
                table[currentIndex]->value = value;
                return;
            }

            i++;
        }

        cout << "Error: Hash table is full" << endl;
    }

    bool search(int key, string& value) {
        // probe linearly until an empty slot is reached
        size_t index = hashInt(key, size);
        size_t i = 0;

        while (i < size) {
            size_t currentIndex = (index + i) % size;

            // If we hit a nullptr the key was never inserted (stop probe)
            if (table[currentIndex] == nullptr) {
                return false;
            } else if (table[currentIndex] != DELETED && table[currentIndex]->key == key) {
                // found a live entry matching the key
                value = table[currentIndex]->value;
                return true;
            }

            i++;
        }

        return false;
    }

    void rehash() {
        // double table size and reinsert all active items
        size_t newSize = size * 2;
        vector<KeyValuePair<int, string>*> oldTable = table;

        table.clear();
        table.resize(newSize, nullptr);
        size = newSize;
        count = 0;

        for (auto ptr : oldTable) {
            if (ptr != nullptr && ptr != DELETED) {
                insert(ptr->key, ptr->value);
                delete ptr;
            }
        }
    }

    // Get load factor
    double getLoadFactor() const {
        return static_cast<double>(count) / size;
    }
};

// Open addressing with double hashing (secondary step size)
class DoubleHashing {
private:
    vector<KeyValuePair<int, string>*> table;
    size_t size;
    size_t count;

    KeyValuePair<int, string>* DELETED;

public:
    DoubleHashing(size_t initialSize) : size(initialSize), count(0) {
        table.resize(size, nullptr);
        DELETED = new KeyValuePair<int, string>(-1, "DELETED");
    }

    ~DoubleHashing() {
        for (auto ptr : table) {
            if (ptr != nullptr && ptr != DELETED) {
                delete ptr;
            }
        }
        delete DELETED;
    }

    void insert(int key, const string& value) {
        if (getLoadFactor() > 0.7) {
            rehash();
        }

        // Use two hashes: probe sequence is (h1 + i * h2) mod size.
        // Double hashing reduces clustering compared to linear probing.
        size_t h1 = hashInt(key, size);
        size_t h2 = secondHash(key, size);
        size_t i = 0;

        while (i < size) {
            size_t currentIndex = (h1 + i * h2) % size;

            // empty or deleted -> place new entry
            if (table[currentIndex] == nullptr || table[currentIndex] == DELETED) {
                table[currentIndex] = new KeyValuePair<int, string>(key, value);
                count++;
                return;
            } else if (table[currentIndex]->key == key) {
                // existing key found -> update
                table[currentIndex]->value = value;
                return;
            }

            i++;
        }

        cout << "Error: Hash table is full" << endl;
    }

    bool search(int key, string& value) {
        // probe using the double-hash sequence until empty slot
        size_t h1 = hashInt(key, size);
        size_t h2 = secondHash(key, size);
        size_t i = 0;

        while (i < size) {
            size_t currentIndex = (h1 + i * h2) % size;

            // If we reach an unused slot, probing can stop — key not present
            if (table[currentIndex] == nullptr) {
                return false;
            } else if (table[currentIndex] != DELETED && table[currentIndex]->key == key) {
                // found active entry
                value = table[currentIndex]->value;
                return true;
            }

            i++;
        }

        return false;
    }

    void rehash() {
        // grow table and reinsert active entries
        size_t newSize = size * 2;
        vector<KeyValuePair<int, string>*> oldTable = table;

        table.clear();
        table.resize(newSize, nullptr);
        size = newSize;
        count = 0;

        for (auto ptr : oldTable) {
            if (ptr != nullptr && ptr != DELETED) {
                insert(ptr->key, ptr->value);
                delete ptr;
            }
        }
    }

    double getLoadFactor() const {
        return static_cast<double>(count) / size;
    }
};

vector<int> generateRandomKeys(int count, int min, int max) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(min, max);

    vector<int> keys(count);
    for (int i = 0; i < count; i++) {
        keys[i] = distrib(gen);
    }

    return keys;
}

void benchmarkHashTables() {
    cout << "Benchmarking hash tables..." << endl;

    const int initialSize = 1000;
    const vector<double> loadFactors = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9};
    const int keysToInsert = 900; // Maximum will be 90% of initialSize
    const int searchIterations = 10000;

    ofstream outputFile("C:/Users/dudec/CLionProjects/Task3/hash_benchmark.csv");
    outputFile << "LoadFactor,SeparateChaining_Insert,SeparateChaining_Search,"
               << "LinearProbing_Insert,LinearProbing_Search,"
               << "DoubleHashing_Insert,DoubleHashing_Search\n";

    vector<int> insertKeys = generateRandomKeys(keysToInsert, 1, 100000);
    vector<int> searchKeys = generateRandomKeys(searchIterations, 1, 100000);

    for (double targetLoadFactor : loadFactors) {
        int itemsToInsert = static_cast<int>(targetLoadFactor * initialSize);

        SeparateChaining scTable(initialSize);
        LinearProbing lpTable(initialSize);
        DoubleHashing dhTable(initialSize);

        long long scInsertTime = 0, scSearchTime = 0;
        long long lpInsertTime = 0, lpSearchTime = 0;
        long long dhInsertTime = 0, dhSearchTime = 0;

        auto startSC = chrono::high_resolution_clock::now();
        for (int i = 0; i < itemsToInsert; i++) {
            scTable.insert(insertKeys[i], "value" + to_string(insertKeys[i]));
        }
        auto endSC = chrono::high_resolution_clock::now();
        scInsertTime = chrono::duration_cast<chrono::microseconds>(endSC - startSC).count();

        auto startLP = chrono::high_resolution_clock::now();
        for (int i = 0; i < itemsToInsert; i++) {
            lpTable.insert(insertKeys[i], "value" + to_string(insertKeys[i]));
        }
        auto endLP = chrono::high_resolution_clock::now();
        lpInsertTime = chrono::duration_cast<chrono::microseconds>(endLP - startLP).count();

        auto startDH = chrono::high_resolution_clock::now();
        for (int i = 0; i < itemsToInsert; i++) {
            dhTable.insert(insertKeys[i], "value" + to_string(insertKeys[i]));
        }
        auto endDH = chrono::high_resolution_clock::now();
        dhInsertTime = chrono::duration_cast<chrono::microseconds>(endDH - startDH).count();

        string result;
        startSC = chrono::high_resolution_clock::now();
        for (int i = 0; i < searchIterations; i++) {
            scTable.search(searchKeys[i], result);
        }
        endSC = chrono::high_resolution_clock::now();
        scSearchTime = chrono::duration_cast<chrono::microseconds>(endSC - startSC).count();

        startLP = chrono::high_resolution_clock::now();
        for (int i = 0; i < searchIterations; i++) {
            lpTable.search(searchKeys[i], result);
        }
        endLP = chrono::high_resolution_clock::now();
        lpSearchTime = chrono::duration_cast<chrono::microseconds>(endLP - startLP).count();

        startDH = chrono::high_resolution_clock::now();
        for (int i = 0; i < searchIterations; i++) {
            dhTable.search(searchKeys[i], result);
        }
        endDH = chrono::high_resolution_clock::now();
        dhSearchTime = chrono::duration_cast<chrono::microseconds>(endDH - startDH).count();

        cout << "Load Factor: " << targetLoadFactor << endl;
        cout << "  Separate Chaining - Insert: " << scInsertTime << " μs, Search: " << scSearchTime << " μs" << endl;
        cout << "  Linear Probing - Insert: " << lpInsertTime << " μs, Search: " << lpSearchTime << " μs" << endl;
        cout << "  Double Hashing - Insert: " << dhInsertTime << " μs, Search: " << dhSearchTime << " μs" << endl;

        outputFile << targetLoadFactor << ","
                  << scInsertTime << "," << scSearchTime << ","
                  << lpInsertTime << "," << lpSearchTime << ","
                  << dhInsertTime << "," << dhSearchTime << "\n";
    }

    outputFile.close();
    cout << "Benchmark results written to hash_benchmark.csv" << endl;
}

int main() {
    benchmarkHashTables();

    return 0;
}