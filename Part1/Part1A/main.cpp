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

    // Trie node used for the trie-based spell checking approach.
    struct TrieNode {
        bool isEndOfWord;
        TrieNode* children[26];

        TrieNode() {
            isEndOfWord = false;
            for (int i = 0; i < 26; i++) {
                children[i] = nullptr;
            }
        }

        ~TrieNode() {
            for (int i = 0; i < 26; i++) {
                if (children[i]) {
                    delete children[i];
                }
            }
        }
    };

    vector<string> linearList;
    set<string> bst;
    TrieNode* trieRoot = new TrieNode();
    unordered_set<string> hashTable;

    // Normalizes a word by keeping only letters and converting them to lowercase.
    string cleanWord(string word) {
        string result = "";
        for (char c : word) {
            if (isalpha(c)) {
                result += tolower(c);
            }
        }
        return result;
    }

    // Inserts one dictionary word into the trie structure.
    void insertTrie(string word) {
        TrieNode* current = trieRoot;

        for (char c : word) {
            int index = c - 'a';
            if (index < 0 || index >= 26) continue;

            if (!current->children[index]) {
                current->children[index] = new TrieNode();
            }
            current = current->children[index];
        }
        current->isEndOfWord = true;
    }

    // Searches a word in the trie structure.
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

    // Loads the dictionary and fills the linear list, BBST, trie, and hash map.
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

        sort(linearList.begin(), linearList.end());

        return true;
    }

    // Naive linear-list approach: checks the word using the sorted vector.
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

    // String BBST approach: checks the word using std::set.
    bool checkBST(string word) {
        word = cleanWord(word);
        if (word.empty()) return true;

        return bst.find(word) != bst.end();
    }

    // Trie approach: checks the word using the prefix tree.
    bool checkTrie(string word) {
        word = cleanWord(word);
        if (word.empty()) return true;

        return searchTrie(word);
    }

    // Hash map approach: checks the word using unordered_set.
    bool checkHash(string word) {
        word = cleanWord(word);
        if (word.empty()) return true;

        return hashTable.find(word) != hashTable.end();
    }

    // Measures how long each data structure takes to build from the dictionary.
    void measureBuildTime(string filename) {
        // Clear existing dictionaries
        linearList.clear();
        bst.clear();

        delete trieRoot; // Free old trie
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

    // Spell-checks a text file and measures lookup time for each approach.
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
    cout << "Hash: " << hashMiss << " misspelled words, Time: " << hashTime << " microseconds" << endl;}

    // Runs lookup benchmarks on increasing text sizes and reports the average time.
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
            cout << "Linear: " << linearTimes.back() << " μs" << endl;
            cout << "BST: " << bstTimes.back() << " μs" << endl;
            cout << "Trie: " << trieTimes.back() << " μs" << endl;
            cout << "Hash: " << hashTimes.back() << " μs" << endl;
            cout << "-----------------------------------" << endl;
        }

        ofstream output("benchmark_results.csv");;
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

        delete trieRoot;

        return 0;
    }