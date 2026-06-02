#include <iostream>
#include <vector>
#include <list>
#include <chrono>
#include <fstream>
#include <random>
#include <string>
#include <algorithm>
#include <unordered_set>

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
    const size_t R = 9973; // prime smaller than 10007
    return R - (key % R);
}

class SeparateChaining {
private:
    vector<list<KeyValuePair<int, string>>> table;
    size_t size;
    size_t count;

public:
    SeparateChaining(size_t initialSize)
        : size(initialSize), count(0) {
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
    LinearProbing(size_t initialSize)
        : size(initialSize), count(0) {
        table.resize(size, nullptr);
    }

    ~LinearProbing() {
        for (auto ptr : table) {
            delete ptr;
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
    DoubleHashing(size_t initialSize)
        : size(initialSize), count(0) {
        table.resize(size, nullptr);
    }

    ~DoubleHashing() {
        for (auto ptr : table) {
            delete ptr;
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
    unordered_set<int> used;

    mt19937 gen(42);
    uniform_int_distribution<int> dist(1, 100000000);

    while ((int)keys.size() < count) {
        int key = dist(gen);

        if (used.find(key) == used.end()) {
            used.insert(key);
            keys.push_back(key);
        }
    }

    return keys;
}

vector<int> generateSuccessfulSearchKeys(const vector<int>& insertedKeys, int searchCount) {
    vector<int> searchKeys;

    mt19937 gen(100);
    uniform_int_distribution<int> dist(0, insertedKeys.size() - 1);

    for (int i = 0; i < searchCount; i++) {
        searchKeys.push_back(insertedKeys[dist(gen)]);
    }

    return searchKeys;
}

vector<int> generateUnsuccessfulSearchKeys(int searchCount) {
    vector<int> searchKeys;

    for (int i = 0; i < searchCount; i++) {
        searchKeys.push_back(200000000 + i);
    }

    return searchKeys;
}

void benchmarkHashTables() {
    cout << "Benchmarking hash tables..." << endl;

    const int tableSize = 10007;

    const vector<double> loadFactors = {
        0.5, 0.6, 0.7, 0.8, 0.9, 0.91, 0.92, 0.93, 0.94,
        0.95, 0.96, 0.97, 0.98, 0.99, 0.995, 0.997, 0.998, 0.999
    };

    const int maxKeys = static_cast<int>(0.999 * tableSize);
    const int searchIterations = 100000;

    vector<int> allInsertKeys = generateUniqueKeys(maxKeys);

    ofstream outputFile("hash_benchmark.csv");

    outputFile
        << "LoadFactor,"
        << "SeparateChaining_Insert,"
        << "SeparateChaining_SuccessfulSearch,"
        << "SeparateChaining_UnsuccessfulSearch,"
        << "LinearProbing_Insert,"
        << "LinearProbing_SuccessfulSearch,"
        << "LinearProbing_UnsuccessfulSearch,"
        << "DoubleHashing_Insert,"
        << "DoubleHashing_SuccessfulSearch,"
        << "DoubleHashing_UnsuccessfulSearch\n";

    for (double loadFactor : loadFactors) {
        int itemsToInsert = static_cast<int>(loadFactor * tableSize);

        vector<int> currentKeys(
            allInsertKeys.begin(),
            allInsertKeys.begin() + itemsToInsert
        );

        vector<int> successfulSearchKeys =
            generateSuccessfulSearchKeys(currentKeys, searchIterations);

        vector<int> unsuccessfulSearchKeys =
            generateUnsuccessfulSearchKeys(searchIterations);

        SeparateChaining scTable(tableSize);
        LinearProbing lpTable(tableSize);
        DoubleHashing dhTable(tableSize);

        auto start = chrono::high_resolution_clock::now();

        for (int key : currentKeys) {
            scTable.insert(key, "x");
        }

        auto end = chrono::high_resolution_clock::now();

        long long scInsertTime =
            chrono::duration_cast<chrono::microseconds>(end - start).count();

        start = chrono::high_resolution_clock::now();

        for (int key : currentKeys) {
            lpTable.insert(key, "x");
        }

        end = chrono::high_resolution_clock::now();

        long long lpInsertTime =
            chrono::duration_cast<chrono::microseconds>(end - start).count();

        start = chrono::high_resolution_clock::now();

        for (int key : currentKeys) {
            dhTable.insert(key, "x");
        }

        end = chrono::high_resolution_clock::now();

        long long dhInsertTime =
            chrono::duration_cast<chrono::microseconds>(end - start).count();

        string result;

        start = chrono::high_resolution_clock::now();

        for (int key : successfulSearchKeys) {
            scTable.search(key, result);
        }

        end = chrono::high_resolution_clock::now();

        long long scSuccessfulSearchTime =
            chrono::duration_cast<chrono::microseconds>(end - start).count();

        start = chrono::high_resolution_clock::now();

        for (int key : unsuccessfulSearchKeys) {
            scTable.search(key, result);
        }

        end = chrono::high_resolution_clock::now();

        long long scUnsuccessfulSearchTime =
            chrono::duration_cast<chrono::microseconds>(end - start).count();

        start = chrono::high_resolution_clock::now();

        for (int key : successfulSearchKeys) {
            lpTable.search(key, result);
        }

        end = chrono::high_resolution_clock::now();

        long long lpSuccessfulSearchTime =
            chrono::duration_cast<chrono::microseconds>(end - start).count();

        start = chrono::high_resolution_clock::now();

        for (int key : unsuccessfulSearchKeys) {
            lpTable.search(key, result);
        }

        end = chrono::high_resolution_clock::now();

        long long lpUnsuccessfulSearchTime =
            chrono::duration_cast<chrono::microseconds>(end - start).count();

        start = chrono::high_resolution_clock::now();

        for (int key : successfulSearchKeys) {
            dhTable.search(key, result);
        }

        end = chrono::high_resolution_clock::now();

        long long dhSuccessfulSearchTime =
            chrono::duration_cast<chrono::microseconds>(end - start).count();

        start = chrono::high_resolution_clock::now();

        for (int key : unsuccessfulSearchKeys) {
            dhTable.search(key, result);
        }

        end = chrono::high_resolution_clock::now();

        long long dhUnsuccessfulSearchTime =
            chrono::duration_cast<chrono::microseconds>(end - start).count();

        cout << "Load Factor: " << loadFactor << endl;

        cout << "  Separate Chaining - Insert: "
             << scInsertTime
             << " us, Successful Search: "
             << scSuccessfulSearchTime
             << " us, Unsuccessful Search: "
             << scUnsuccessfulSearchTime
             << " us"
             << endl;

        cout << "  Linear Probing - Insert: "
             << lpInsertTime
             << " us, Successful Search: "
             << lpSuccessfulSearchTime
             << " us, Unsuccessful Search: "
             << lpUnsuccessfulSearchTime
             << " us"
             << endl;

        cout << "  Double Hashing - Insert: "
             << dhInsertTime
             << " us, Successful Search: "
             << dhSuccessfulSearchTime
             << " us, Unsuccessful Search: "
             << dhUnsuccessfulSearchTime
             << " us"
             << endl;

        outputFile
            << loadFactor << ","
            << scInsertTime << ","
            << scSuccessfulSearchTime << ","
            << scUnsuccessfulSearchTime << ","
            << lpInsertTime << ","
            << lpSuccessfulSearchTime << ","
            << lpUnsuccessfulSearchTime << ","
            << dhInsertTime << ","
            << dhSuccessfulSearchTime << ","
            << dhUnsuccessfulSearchTime << "\n";
    }

    outputFile.close();

    cout << "Benchmark results written to hash_benchmark.csv" << endl;
}

int main() {
    benchmarkHashTables();
    return 0;
}