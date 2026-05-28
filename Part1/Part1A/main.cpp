#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_set>
#include <set>
#include <chrono>
#include <algorithm>
#include <cctype>

using namespace std;

// This program solves Assignment 2 Part 1A: "Me spell rite".
// It compares four dictionary structures for spell checking:
// 1. naive linear list
// 2. balanced binary search tree using std::set
// 3. trie / prefix tree
// 4. hash table using std::unordered_set

// Each TrieNode represents one character in a word path.
// children[26] stores links for lowercase letters a-z.
// isEndOfWord is true when the path forms a complete dictionary word.
struct TrieNode {
    bool isEndOfWord;
    TrieNode* children[26];

    TrieNode() {
        isEndOfWord = false;
        for (int i = 0; i < 26; i++) {
            children[i] = nullptr;
        }
    }

    // Destructor deletes all child nodes to avoid memory leaks.
    ~TrieNode() {
        for (int i = 0; i < 26; i++) {
            if (children[i]) {
                delete children[i];
            }
        }
    }
};

// These are the four dictionary structures being compared.
vector<string> linearList;              // Naive linear list
set<string> bst;                        // Balanced binary search tree
TrieNode* trieRoot = new TrieNode();    // Trie root node
unordered_set<string> hashTable;        // Hash table

// This function cleans words before storing or searching.
// It removes punctuation and converts letters to lowercase.
// Example: "Computer!" becomes "computer".
string cleanWord(string word) {
    string result = "";

    for (char c : word) {
        if (isalpha(c)) {
            result += tolower(c);
        }
    }

    return result;
}

// This inserts a word into the trie letter by letter.
// If a needed child node does not exist, it creates it.
// At the end, the final node is marked as a complete word.
void insertTrie(string word) {
    TrieNode* current = trieRoot;

    for (char c : word) {
        int index = c - 'a';

        // Ignore characters outside a-z.
        if (index < 0 || index >= 26) continue;

        if (!current->children[index]) {
            current->children[index] = new TrieNode();
        }

        current = current->children[index];
    }

    current->isEndOfWord = true;
}

// This searches for a word inside the trie.
// If any required character path is missing, the word is not found.
// The word is correct only if the final node is marked as end of word.
bool searchTrie(string word) {
    TrieNode* current = trieRoot;

    for (char c : word) {
        int index = c - 'a';

        if (index < 0 || index >= 26) return false;

        if (!current->children[index]) {
            return false;
        }

        current = current->children[index];
    }

    return current && current->isEndOfWord;
}

// This loads the dictionary file.
// Every cleaned word is inserted into all four structures.
// This makes the comparison fair because all methods use the same words.
bool loadDictionary(string filename) {
    ifstream file(filename);

    if (!file.is_open()) {
        cout << "Error: Could not open dictionary file" << endl;
        return false;
    }

    string word;

    while (getline(file, word)) {
        word = cleanWord(word);

        if (!word.empty()) {
            linearList.push_back(word);
            bst.insert(word);
            insertTrie(word);
            hashTable.insert(word);
        }
    }

    // Sorting is only used for organized storage.
    // The actual linear search still scans one by one.
    sort(linearList.begin(), linearList.end());

    return true;
}

// Naive linear-list approach.
// It checks every dictionary word one by one until it finds a match.
// Worst-case time complexity: O(n), where n is dictionary size.
bool checkLinear(string word) {
    word = cleanWord(word);

    if (word.empty()) return true;

    for (string w : linearList) {
        if (w == word) {
            return true;
        }
    }

    return false;
}

// BBST approach using std::set.
// std::set is usually implemented as a balanced binary search tree.
// Lookup time complexity: O(log n).
bool checkBST(string word) {
    word = cleanWord(word);

    if (word.empty()) return true;

    return bst.find(word) != bst.end();
}

// Trie approach.
// Lookup depends on the number of characters in the word.
// Time complexity: O(k), where k is the word length.
bool checkTrie(string word) {
    word = cleanWord(word);

    if (word.empty()) return true;

    return searchTrie(word);
}

// Hash table approach using unordered_set.
// Average lookup time is O(1).
// Worst case can be slower if many collisions happen.
bool checkHash(string word) {
    word = cleanWord(word);

    if (word.empty()) return true;

    return hashTable.find(word) != hashTable.end();
}

// This measures dictionary build time for each structure.
// I first read all words into a vector, then build each structure separately.
// This makes the build-time comparison clearer.
void measureBuildTime(string filename) {
    linearList.clear();
    bst.clear();

    delete trieRoot;
    trieRoot = new TrieNode();

    hashTable.clear();

    ifstream file(filename);
    vector<string> words;
    string word;

    while (getline(file, word)) {
        word = cleanWord(word);

        if (!word.empty()) {
            words.push_back(word);
        }
    }

    auto startLinear = chrono::high_resolution_clock::now();

    for (string w : words) {
        linearList.push_back(w);
    }

    sort(linearList.begin(), linearList.end());

    auto endLinear = chrono::high_resolution_clock::now();

    auto startBST = chrono::high_resolution_clock::now();

    for (string w : words) {
        bst.insert(w);
    }

    auto endBST = chrono::high_resolution_clock::now();

    auto startTrie = chrono::high_resolution_clock::now();

    for (string w : words) {
        insertTrie(w);
    }

    auto endTrie = chrono::high_resolution_clock::now();

    auto startHash = chrono::high_resolution_clock::now();

    for (string w : words) {
        hashTable.insert(w);
    }

    auto endHash = chrono::high_resolution_clock::now();

    auto linearTime = chrono::duration_cast<chrono::milliseconds>(endLinear - startLinear).count();
    auto bstTime = chrono::duration_cast<chrono::milliseconds>(endBST - startBST).count();
    auto trieTime = chrono::duration_cast<chrono::milliseconds>(endTrie - startTrie).count();
    auto hashTime = chrono::duration_cast<chrono::milliseconds>(endHash - startHash).count();

    cout << "Dictionary build times for " << words.size() << " words:" << endl;
    cout << "Linear List: " << linearTime << " ms" << endl;
    cout << "BST: " << bstTime << " ms" << endl;
    cout << "Trie: " << trieTime << " ms" << endl;
    cout << "Hash: " << hashTime << " ms" << endl;
}

// This checks every word in the text file.
// Each word is tested using all four methods.
// I count misspelled words and measure total lookup time.
void checkFile(string filename) {
    ifstream file(filename);

    if (!file.is_open()) {
        cout << "Error: Could not open text file" << endl;
        return;
    }

    int linearMiss = 0, bstMiss = 0, trieMiss = 0, hashMiss = 0;
    int totalWords = 0;

    long long linearTime = 0, bstTime = 0, trieTime = 0, hashTime = 0;

    string word;

    while (file >> word) {
        totalWords++;

        auto start = chrono::high_resolution_clock::now();
        if (!checkLinear(word)) linearMiss++;
        auto end = chrono::high_resolution_clock::now();
        linearTime += chrono::duration_cast<chrono::microseconds>(end - start).count();

        start = chrono::high_resolution_clock::now();
        if (!checkBST(word)) bstMiss++;
        end = chrono::high_resolution_clock::now();
        bstTime += chrono::duration_cast<chrono::microseconds>(end - start).count();

        start = chrono::high_resolution_clock::now();
        if (!checkTrie(word)) trieMiss++;
        end = chrono::high_resolution_clock::now();
        trieTime += chrono::duration_cast<chrono::microseconds>(end - start).count();

        start = chrono::high_resolution_clock::now();
        if (!checkHash(word)) hashMiss++;
        end = chrono::high_resolution_clock::now();
        hashTime += chrono::duration_cast<chrono::microseconds>(end - start).count();
    }

    cout << "File: " << filename << endl;
    cout << "Total words: " << totalWords << endl;
    cout << "Linear List: " << linearMiss << " misspelled words, Time: " << linearTime << " microseconds" << endl;
    cout << "BST: " << bstMiss << " misspelled words, Time: " << bstTime << " microseconds" << endl;
    cout << "Trie: " << trieMiss << " misspelled words, Time: " << trieTime << " microseconds" << endl;
    cout << "Hash: " << hashMiss << " misspelled words, Time: " << hashTime << " microseconds" << endl;
}

// This runs benchmarks using different text sizes.
// It allows me to create the required runtime vs text length graph.
// The results are saved to benchmark_results.csv.
void runBenchmark(string filename, vector<int> sizes) {
    ifstream file(filename);

    if (!file.is_open()) {
        cout << "Error opening file for benchmark" << endl;
        return;
    }

    vector<string> allWords;
    string word;

    while (file >> word) {
        allWords.push_back(word);
    }

    vector<long long> linearTimes, bstTimes, trieTimes, hashTimes;

    for (int size : sizes) {
        if (size > allWords.size()) {
            size = allWords.size();
        }

        auto startTime = chrono::high_resolution_clock::now();

        for (int i = 0; i < size; i++) {
            checkLinear(allWords[i]);
        }

        auto endTime = chrono::high_resolution_clock::now();
        linearTimes.push_back(chrono::duration_cast<chrono::microseconds>(endTime - startTime).count());

        startTime = chrono::high_resolution_clock::now();

        for (int i = 0; i < size; i++) {
            checkBST(allWords[i]);
        }

        endTime = chrono::high_resolution_clock::now();
        bstTimes.push_back(chrono::duration_cast<chrono::microseconds>(endTime - startTime).count());

        startTime = chrono::high_resolution_clock::now();

        for (int i = 0; i < size; i++) {
            checkTrie(allWords[i]);
        }

        endTime = chrono::high_resolution_clock::now();
        trieTimes.push_back(chrono::duration_cast<chrono::microseconds>(endTime - startTime).count());

        startTime = chrono::high_resolution_clock::now();

        for (int i = 0; i < size; i++) {
            checkHash(allWords[i]);
        }

        endTime = chrono::high_resolution_clock::now();
        hashTimes.push_back(chrono::duration_cast<chrono::microseconds>(endTime - startTime).count());

        cout << "Benchmark for " << size << " words:" << endl;
        cout << "Linear: " << linearTimes.back() << " microseconds" << endl;
        cout << "BST: " << bstTimes.back() << " microseconds" << endl;
        cout << "Trie: " << trieTimes.back() << " microseconds" << endl;
        cout << "Hash: " << hashTimes.back() << " microseconds" << endl;
        cout << "-----------------------------------" << endl;
    }

    // Save benchmark results so they can be used for report graphs.
    ofstream output("benchmark_results.csv");

    if (output.is_open()) {
        output << "TextSize,Linear,BST,Trie,Hash\n";

        for (int i = 0; i < sizes.size(); i++) {
            if (i < linearTimes.size()) {
                output << sizes[i] << ","
                       << linearTimes[i] << ","
                       << bstTimes[i] << ","
                       << trieTimes[i] << ","
                       << hashTimes[i] << "\n";
            }
        }

        output.close();

        cout << "Benchmark results saved to benchmark_results.csv" << endl;
    }
}

int main() {
    string dictFile = "english_words.txt";
    string textFile = "spell_text.txt";

    cout << "Loading dictionary from " << dictFile << "..." << endl;

    auto startLoad = chrono::high_resolution_clock::now();
    bool success = loadDictionary(dictFile);
    auto endLoad = chrono::high_resolution_clock::now();

    if (!success) {
        return 1;
    }

    auto loadTime = chrono::duration_cast<chrono::milliseconds>(endLoad - startLoad).count();

    cout << "Dictionary loaded in " << loadTime << " ms" << endl;

    cout << "\nMeasuring dictionary build times..." << endl;
    measureBuildTime(dictFile);

    cout << "\nChecking spelling in " << textFile << "..." << endl;
    checkFile(textFile);

    cout << "\nRunning benchmark with different text sizes..." << endl;

    vector<int> textSizes = {100, 500, 1000, 5000, 10000};

    runBenchmark(textFile, textSizes);

    // Delete trie root at the end to release memory.
    delete trieRoot;

    return 0;
}