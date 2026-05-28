#include <iostream>
#include <vector>
#include <list>
#include <chrono>
#include <fstream>
#include <random>
#include <string>
#include <algorithm>

using namespace std;

template <typename K, typename V>
struct KeyValuePair {
    K key;
    V value;

    KeyValuePair(K k, V v) : key(k), value(v) {}
};

size_t hashInt(int key, size_t tableSize) {
    return key % tableSize;
}

size_t secondHash(int key, size_t tableSize) {
    size_t prime = tableSize > 3 ? tableSize - 2 : 1;
    return 1 + (key % prime);
}

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
        size_t index = hashInt(key, size);

        for (auto& pair : table[index]) {
            if (pair.key == key) {
                pair.value = value;
                return;
            }
        }

        table[index].push_back(KeyValuePair<int, string>(key, value));
        count++;
    }

    bool search(int key, string& value) {
        size_t index = hashInt(key, size);

        for (const auto& pair : table[index]) {
            if (pair.key == key) {
                value = pair.value;
                return true;
            }
        }

        return false;
    }
};

class LinearProbing {
private:
    vector<KeyValuePair<int, string>*> table;
    size_t size;
    size_t count;

public:
    LinearProbing(size_t initialSize) : size(initialSize), count(0) {
        table.resize(size, nullptr);
    }

    ~LinearProbing() {
        for (auto ptr : table) {
            if (ptr != nullptr) {
                delete ptr;
            }
        }
    }

    void insert(int key, const string& value) {
        size_t index = hashInt(key, size);

        for (size_t i = 0; i < size; i++) {
            size_t currentIndex = (index + i) % size;

            if (table[currentIndex] == nullptr) {
                table[currentIndex] = new KeyValuePair<int, string>(key, value);
                count++;
                return;
            }

            if (table[currentIndex]->key == key) {
                table[currentIndex]->value = value;
                return;
            }
        }

        cout << "Error: Linear probing table is full" << endl;
    }

    bool search(int key, string& value) {
        size_t index = hashInt(key, size);

        for (size_t i = 0; i < size; i++) {
            size_t currentIndex = (index + i) % size;

            if (table[currentIndex] == nullptr) {
                return false;
            }

            if (table[currentIndex]->key == key) {
                value = table[currentIndex]->value;
                return true;
            }
        }

        return false;
    }
};

class DoubleHashing {
private:
    vector<KeyValuePair<int, string>*> table;
    size_t size;
    size_t count;

public:
    DoubleHashing(size_t initialSize) : size(initialSize), count(0) {
        table.resize(size, nullptr);
    }

    ~DoubleHashing() {
        for (auto ptr : table) {
            if (ptr != nullptr) {
                delete ptr;
            }
        }
    }

    void insert(int key, const string& value) {
        size_t h1 = hashInt(key, size);
        size_t h2 = secondHash(key, size);

        for (size_t i = 0; i < size; i++) {
            size_t currentIndex = (h1 + i * h2) % size;

            if (table[currentIndex] == nullptr) {
                table[currentIndex] = new KeyValuePair<int, string>(key, value);
                count++;
                return;
            }

            if (table[currentIndex]->key == key) {
                table[currentIndex]->value = value;
                return;
            }
        }

        cout << "Error: Double hashing table is full" << endl;
    }

    bool search(int key, string& value) {
        size_t h1 = hashInt(key, size);
        size_t h2 = secondHash(key, size);

        for (size_t i = 0; i < size; i++) {
            size_t currentIndex = (h1 + i * h2) % size;

            if (table[currentIndex] == nullptr) {
                return false;
            }

            if (table[currentIndex]->key == key) {
                value = table[currentIndex]->value;
                return true;
            }
        }

        return false;
    }
};

vector<int> generateUniqueKeys(int count) {
    vector<int> keys;

    for (int i = 1; i <= count; i++) {
        keys.push_back(i);
    }

    random_device rd;
    mt19937 gen(rd());

    shuffle(keys.begin(), keys.end(), gen);

    return keys;
}

vector<int> generateSearchKeys(const vector<int>& insertedKeys, int searchCount) {
    vector<int> searchKeys;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(0, insertedKeys.size() - 1);

    for (int i = 0; i < searchCount; i++) {
        searchKeys.push_back(insertedKeys[distrib(gen)]);
    }

    return searchKeys;
}

void benchmarkHashTables() {
    cout << "Benchmarking hash tables..." << endl;

    const int tableSize = 1009;
    const vector<double> loadFactors = {
        0.1, 0.2, 0.3, 0.4, 0.5,
        0.6, 0.7, 0.8, 0.9
    };

    const int maxKeys = 908;
    const int searchIterations = 10000;

    vector<int> insertKeys = generateUniqueKeys(maxKeys);

    ofstream outputFile("hash_benchmark.csv");

    outputFile << "LoadFactor,SeparateChaining_Insert,SeparateChaining_Search,"
               << "LinearProbing_Insert,LinearProbing_Search,"
               << "DoubleHashing_Insert,DoubleHashing_Search\n";

    for (double targetLoadFactor : loadFactors) {
        int itemsToInsert = static_cast<int>(targetLoadFactor * tableSize);

        vector<int> currentKeys(
            insertKeys.begin(),
            insertKeys.begin() + itemsToInsert
        );

        vector<int> searchKeys =
            generateSearchKeys(currentKeys, searchIterations);

        SeparateChaining scTable(tableSize);
        LinearProbing lpTable(tableSize);
        DoubleHashing dhTable(tableSize);

        auto startSC = chrono::high_resolution_clock::now();

        for (int key : currentKeys) {
            scTable.insert(key, "value" + to_string(key));
        }

        auto endSC = chrono::high_resolution_clock::now();

        auto startLP = chrono::high_resolution_clock::now();

        for (int key : currentKeys) {
            lpTable.insert(key, "value" + to_string(key));
        }

        auto endLP = chrono::high_resolution_clock::now();

        auto startDH = chrono::high_resolution_clock::now();

        for (int key : currentKeys) {
            dhTable.insert(key, "value" + to_string(key));
        }

        auto endDH = chrono::high_resolution_clock::now();

        long long scInsertTime =
            chrono::duration_cast<chrono::microseconds>(endSC - startSC).count();

        long long lpInsertTime =
            chrono::duration_cast<chrono::microseconds>(endLP - startLP).count();

        long long dhInsertTime =
            chrono::duration_cast<chrono::microseconds>(endDH - startDH).count();

        string result;

        startSC = chrono::high_resolution_clock::now();

        for (int key : searchKeys) {
            scTable.search(key, result);
        }

        endSC = chrono::high_resolution_clock::now();

        startLP = chrono::high_resolution_clock::now();

        for (int key : searchKeys) {
            lpTable.search(key, result);
        }

        endLP = chrono::high_resolution_clock::now();

        startDH = chrono::high_resolution_clock::now();

        for (int key : searchKeys) {
            dhTable.search(key, result);
        }

        endDH = chrono::high_resolution_clock::now();

        long long scSearchTime =
            chrono::duration_cast<chrono::microseconds>(endSC - startSC).count();

        long long lpSearchTime =
            chrono::duration_cast<chrono::microseconds>(endLP - startLP).count();

        long long dhSearchTime =
            chrono::duration_cast<chrono::microseconds>(endDH - startDH).count();

        cout << "Load Factor: " << targetLoadFactor << endl;
        cout << "  Separate Chaining - Insert: "
             << scInsertTime << " us, Search: "
             << scSearchTime << " us" << endl;

        cout << "  Linear Probing - Insert: "
             << lpInsertTime << " us, Search: "
             << lpSearchTime << " us" << endl;

        cout << "  Double Hashing - Insert: "
             << dhInsertTime << " us, Search: "
             << dhSearchTime << " us" << endl;

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