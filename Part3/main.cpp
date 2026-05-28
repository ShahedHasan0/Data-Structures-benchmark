#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <queue>
#include <map>
#include <random>
#include <iomanip>
#include <cctype>
using namespace std;
using Clock = chrono::high_resolution_clock;

// ============================================================
// Part Three - Fast & Furious: Dictionary Races
// Structures compared:
// 1) Ordinary Trie
// 2) Balanced Ternary Prefix Tree using treap-style priorities
// 3) Double Array Trie
// 4) Red-Black Tree
//
// Output files:
//   part3_results.csv  -> full benchmark table for graphs
//   part3_summary.csv  -> largest-size summary
//
// Compile:
//   g++ -O2 -std=c++17 part3_dictionary_races.cpp -o part3
// Run:
//   ./part3 words.txt
// If words.txt is missing or empty, a built-in sample dictionary is used.
// ============================================================

static double ms(Clock::time_point a, Clock::time_point b) {
    return chrono::duration<double, milli>(b - a).count();
}

static string cleanWord(const string& s) {
    string out;
    for (unsigned char c : s) {
        if (isalpha(c)) out.push_back((char)tolower(c));
    }
    return out;
}

static vector<string> loadWords(const string& path) {
    ifstream in(path);
    vector<string> words;
    string w;
    while (in >> w) {
        w = cleanWord(w);
        if (!w.empty()) words.push_back(w);
    }
    sort(words.begin(), words.end());
    words.erase(unique(words.begin(), words.end()), words.end());
    return words;
}

static vector<string> sampleWords() {
    return {
        "apple", "apply", "application", "banana", "band", "bandage", "cat", "cater",
        "dog", "door", "dorm", "dragon", "exit", "exam", "fast", "furious", "hash",
        "hello", "house", "labyrinth", "magic", "name", "spell", "tree", "trie", "wizard",
        "world", "yellow", "zebra"
    };
}

static vector<string> makeQueries(const vector<string>& words, int count) {
    vector<string> q;
    for (int i = 0; i < count && i < (int)words.size(); ++i) q.push_back(words[i]);
    // Add wrong words too, because spell checking needs both successful and failed lookup.
    for (int i = 0; i < count; ++i) q.push_back("notaword" + to_string(i));
    return q;
}

// ============================================================
// 1) ORDINARY TRIE
// ============================================================
struct TrieNode {
    TrieNode* child[26];
    bool endWord;
    TrieNode() : endWord(false) { fill(begin(child), end(child), nullptr); }
};

class OrdinaryTrie {
    TrieNode* root;
    size_t nodes;

    void destroy(TrieNode* n) {
        if (!n) return;
        for (TrieNode* c : n->child) destroy(c);
        delete n;
    }

public:
    OrdinaryTrie() : root(new TrieNode()), nodes(1) {}
    ~OrdinaryTrie() { destroy(root); }

    void insert(const string& word) {
        TrieNode* cur = root;
        for (char ch : word) {
            if (ch < 'a' || ch > 'z') return;
            int id = ch - 'a';
            if (!cur->child[id]) {
                cur->child[id] = new TrieNode();
                nodes++;
            }
            cur = cur->child[id];
        }
        cur->endWord = true;
    }

    bool search(const string& word) const {
        const TrieNode* cur = root;
        for (char ch : word) {
            if (ch < 'a' || ch > 'z') return false;
            int id = ch - 'a';
            if (!cur->child[id]) return false;
            cur = cur->child[id];
        }
        return cur->endWord;
    }

    size_t memoryBytes() const {
        return nodes * sizeof(TrieNode);
    }
};

// ============================================================
// 2) BALANCED TERNARY PREFIX TREE
// Treap-style balancing on left/right character branches.
// Middle branch continues the word and is another balanced TST level.
// ============================================================
struct TSTNode {
    char c;
    int priority;
    bool endWord;
    TSTNode *left, *mid, *right;
    TSTNode(char ch, int pr)
        : c(ch), priority(pr), endWord(false), left(nullptr), mid(nullptr), right(nullptr) {}
};

class BalancedTST {
    TSTNode* root;
    size_t nodes;
    mt19937 rng;

    TSTNode* rotateRight(TSTNode* y) {
        TSTNode* x = y->left;
        y->left = x->right;
        x->right = y;
        return x;
    }

    TSTNode* rotateLeft(TSTNode* x) {
        TSTNode* y = x->right;
        x->right = y->left;
        y->left = x;
        return y;
    }

    TSTNode* insertRec(TSTNode* n, const string& w, int i) {
        if (!n) {
            n = new TSTNode(w[i], (int)rng());
            nodes++;
        }

        if (w[i] < n->c) {
            n->left = insertRec(n->left, w, i);
            if (n->left && n->left->priority < n->priority) n = rotateRight(n);
        } else if (w[i] > n->c) {
            n->right = insertRec(n->right, w, i);
            if (n->right && n->right->priority < n->priority) n = rotateLeft(n);
        } else {
            if (i + 1 == (int)w.size()) n->endWord = true;
            else n->mid = insertRec(n->mid, w, i + 1);
        }
        return n;
    }

    bool searchRec(const TSTNode* n, const string& w, int i) const {
        if (!n) return false;
        if (w[i] < n->c) return searchRec(n->left, w, i);
        if (w[i] > n->c) return searchRec(n->right, w, i);
        if (i + 1 == (int)w.size()) return n->endWord;
        return searchRec(n->mid, w, i + 1);
    }

    void destroy(TSTNode* n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->mid);
        destroy(n->right);
        delete n;
    }

public:
    BalancedTST() : root(nullptr), nodes(0), rng(12345) {}
    ~BalancedTST() { destroy(root); }

    void insert(const string& w) {
        if (!w.empty()) root = insertRec(root, w, 0);
    }

    bool search(const string& w) const {
        if (w.empty()) return false;
        for (char ch : w) if (ch < 'a' || ch > 'z') return false;
        return searchRec(root, w, 0);
    }

    size_t memoryBytes() const {
        return nodes * sizeof(TSTNode);
    }
};

// ============================================================
// 3) DOUBLE ARRAY TRIE
// Simple offline builder: first builds temporary trie, then freezes it
// into base/check arrays.
// ============================================================
class DoubleArrayTrie {
    struct TempNode {
        vector<pair<int,int>> child;
        bool endWord = false;
    };

    vector<TempNode> temp;
    vector<int> baseArr, checkArr;
    vector<char> endArr;

    void ensureSize(int idx) {
        while ((int)baseArr.size() <= idx) {
            baseArr.push_back(0);
            checkArr.push_back(-1);
            endArr.push_back(0);
        }
    }

    int findBase(const vector<int>& chars) {
        int b = 1;
        while (true) {
            bool ok = true;
            for (int c : chars) {
                ensureSize(b + c);
                if (checkArr[b + c] != -1) {
                    ok = false;
                    break;
                }
            }
            if (ok) return b;
            b++;
        }
    }

public:
    DoubleArrayTrie() { temp.push_back(TempNode()); }

    void insert(const string& w) {
        int cur = 0;
        for (char ch : w) {
            if (ch < 'a' || ch > 'z') return;
            int c = ch - 'a' + 1;
            int nxt = -1;
            for (auto &p : temp[cur].child) {
                if (p.first == c) { nxt = p.second; break; }
            }
            if (nxt == -1) {
                nxt = (int)temp.size();
                temp[cur].child.push_back({c, nxt});
                temp.push_back(TempNode());
            }
            cur = nxt;
        }
        temp[cur].endWord = true;
    }

    void freeze() {
        baseArr.assign(temp.size() * 2 + 100, 0);
        checkArr.assign(temp.size() * 2 + 100, -1);
        endArr.assign(temp.size() * 2 + 100, 0);

        vector<int> stateOf(temp.size(), -1);
        stateOf[0] = 0;
        checkArr[0] = 0;
        endArr[0] = temp[0].endWord;

        queue<int> q;
        q.push(0);

        while (!q.empty()) {
            int node = q.front();
            q.pop();
            int state = stateOf[node];

            if (temp[node].child.empty()) continue;

            vector<int> chars;
            for (auto &p : temp[node].child) chars.push_back(p.first);
            int b = findBase(chars);
            baseArr[state] = b;

            for (auto &p : temp[node].child) {
                int c = p.first;
                int childNode = p.second;
                int nextState = b + c;
                ensureSize(nextState);
                checkArr[nextState] = state;
                endArr[nextState] = temp[childNode].endWord;
                stateOf[childNode] = nextState;
                q.push(childNode);
            }
        }

        temp.clear();
        temp.shrink_to_fit();
    }

    bool search(const string& w) const {
        int state = 0;
        for (char ch : w) {
            if (ch < 'a' || ch > 'z') return false;
            int c = ch - 'a' + 1;
            if (state >= (int)baseArr.size()) return false;
            int nextState = baseArr[state] + c;
            if (nextState >= (int)checkArr.size()) return false;
            if (checkArr[nextState] != state) return false;
            state = nextState;
        }
        return state < (int)endArr.size() && endArr[state];
    }

    size_t memoryBytes() const {
        return baseArr.capacity() * sizeof(int)
             + checkArr.capacity() * sizeof(int)
             + endArr.capacity() * sizeof(char);
    }
};

// ============================================================
// 4) RED-BLACK TREE
// ============================================================
enum Color { RED, BLACK };

struct RBNode {
    string key;
    Color color;
    RBNode *left, *right, *parent;
    RBNode(const string& k, RBNode* nil)
        : key(k), color(RED), left(nil), right(nil), parent(nil) {}
};

class RBTree {
    RBNode* NIL;
    RBNode* root;
    size_t nodes;

    void rotateLeft(RBNode* x) {
        RBNode* y = x->right;
        x->right = y->left;
        if (y->left != NIL) y->left->parent = x;
        y->parent = x->parent;
        if (x->parent == NIL) root = y;
        else if (x == x->parent->left) x->parent->left = y;
        else x->parent->right = y;
        y->left = x;
        x->parent = y;
    }

    void rotateRight(RBNode* y) {
        RBNode* x = y->left;
        y->left = x->right;
        if (x->right != NIL) x->right->parent = y;
        x->parent = y->parent;
        if (y->parent == NIL) root = x;
        else if (y == y->parent->right) y->parent->right = x;
        else y->parent->left = x;
        x->right = y;
        y->parent = x;
    }

    void fixInsert(RBNode* z) {
        while (z->parent->color == RED) {
            if (z->parent == z->parent->parent->left) {
                RBNode* y = z->parent->parent->right;
                if (y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {
                        z = z->parent;
                        rotateLeft(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    rotateRight(z->parent->parent);
                }
            } else {
                RBNode* y = z->parent->parent->left;
                if (y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        rotateRight(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    rotateLeft(z->parent->parent);
                }
            }
        }
        root->color = BLACK;
    }

    void destroy(RBNode* n) {
        if (n == NIL) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

public:
    RBTree() : nodes(0) {
        NIL = new RBNode("", nullptr);
        NIL->color = BLACK;
        NIL->left = NIL->right = NIL->parent = NIL;
        root = NIL;
    }

    ~RBTree() {
        destroy(root);
        delete NIL;
    }

    void insert(const string& key) {
        RBNode* y = NIL;
        RBNode* x = root;
        while (x != NIL) {
            y = x;
            if (key == x->key) return;
            if (key < x->key) x = x->left;
            else x = x->right;
        }

        RBNode* z = new RBNode(key, NIL);
        z->parent = y;
        if (y == NIL) root = z;
        else if (key < y->key) y->left = z;
        else y->right = z;
        nodes++;
        fixInsert(z);
    }

    bool search(const string& key) const {
        RBNode* cur = root;
        while (cur != NIL) {
            if (key == cur->key) return true;
            if (key < cur->key) cur = cur->left;
            else cur = cur->right;
        }
        return false;
    }

    size_t memoryBytes() const {
        size_t total = (nodes + 1) * sizeof(RBNode);
        for (RBNode* cur = root; cur != NIL; ) {
            // Full string capacity traversal would be complex; this is an estimate.
            break;
        }
        return total;
    }
};

struct Result {
    string structure;
    int wordCount;
    double buildMs;
    double lookupMs;
    double memoryKB;
    int correctHits;
    int wrongHits;
};

template <class Builder, class Searcher, class Memory>
Result runOne(const string& name, int wordCount, const vector<string>& words,
              const vector<string>& presentQueries, const vector<string>& absentQueries,
              Builder build, Searcher search, Memory memory) {
    auto t1 = Clock::now();
    build();
    auto t2 = Clock::now();

    int correctHits = 0;
    int wrongHits = 0;

    auto t3 = Clock::now();
    for (const string& q : presentQueries) if (search(q)) correctHits++;
    for (const string& q : absentQueries) if (search(q)) wrongHits++;
    auto t4 = Clock::now();

    return {name, wordCount, ms(t1, t2), ms(t3, t4), memory() / 1024.0, correctHits, wrongHits};
}

static void writeCSV(const string& file, const vector<Result>& results) {
    ofstream out(file);
    out << "structure,word_count,build_ms,lookup_ms,memory_kb,correct_hits,wrong_hits\n";
    out << fixed << setprecision(4);
    for (const auto& r : results) {
        out << quoted(r.structure) << ',' << r.wordCount << ',' << r.buildMs << ','
            << r.lookupMs << ',' << r.memoryKB << ',' << r.correctHits << ',' << r.wrongHits << '\n';
    }
}

int main(int argc, char** argv) {
    string file = argc >= 2 ? argv[1] : "English_words.txt";
    vector<string> allWords = loadWords(file);

    if (allWords.empty()) {
        cout << "Dictionary file not found or empty. Using built-in sample dictionary.\n";
        allWords = sampleWords();
        sort(allWords.begin(), allWords.end());
    }

    vector<int> sizes = {100, 500, 1000, 5000, 10000, 25000, 50000};
    vector<int> actualSizes;
    for (int s : sizes) {
        if (s <= (int)allWords.size()) actualSizes.push_back(s);
    }
    if (actualSizes.empty() || actualSizes.back() != (int)allWords.size()) {
        actualSizes.push_back((int)allWords.size());
    }
    sort(actualSizes.begin(), actualSizes.end());
    actualSizes.erase(unique(actualSizes.begin(), actualSizes.end()), actualSizes.end());

    vector<Result> results;

    cout << "Part Three - Fast & Furious: Dictionary Races\n";
    cout << "Loaded unique words: " << allWords.size() << "\n\n";

    cout << left << setw(34) << "Structure"
         << right << setw(10) << "Words"
         << setw(12) << "Build ms"
         << setw(12) << "Lookup ms"
         << setw(12) << "Mem KB"
         << setw(10) << "OK"
         << setw(10) << "Bad" << "\n";
    cout << string(100, '-') << "\n";

    for (int n : actualSizes) {
        vector<string> words(allWords.begin(), allWords.begin() + n);
        int qCount = min(2000, n);
        vector<string> presentQueries(words.begin(), words.begin() + qCount);
        vector<string> absentQueries;
        for (int i = 0; i < qCount; ++i) absentQueries.push_back("zzmissingword" + to_string(i));

        {
            OrdinaryTrie t;
            Result r = runOne("Ordinary Trie", n, words, presentQueries, absentQueries,
                [&](){ for (const string& w : words) t.insert(w); },
                [&](const string& q){ return t.search(q); },
                [&](){ return t.memoryBytes(); });
            results.push_back(r);
        }
        {
            BalancedTST t;
            Result r = runOne("Balanced Ternary Prefix Tree", n, words, presentQueries, absentQueries,
                [&](){ for (const string& w : words) t.insert(w); },
                [&](const string& q){ return t.search(q); },
                [&](){ return t.memoryBytes(); });
            results.push_back(r);
        }
        {
            DoubleArrayTrie t;
            Result r = runOne("Double Array Trie", n, words, presentQueries, absentQueries,
                [&](){ for (const string& w : words) t.insert(w); t.freeze(); },
                [&](const string& q){ return t.search(q); },
                [&](){ return t.memoryBytes(); });
            results.push_back(r);
        }
        {
            RBTree t;
            Result r = runOne("Red-Black Tree", n, words, presentQueries, absentQueries,
                [&](){ for (const string& w : words) t.insert(w); },
                [&](const string& q){ return t.search(q); },
                [&](){ return t.memoryBytes(); });
            results.push_back(r);
        }

        for (int i = (int)results.size() - 4; i < (int)results.size(); ++i) {
            const auto& r = results[i];
            cout << left << setw(34) << r.structure
                 << right << setw(10) << r.wordCount
                 << setw(12) << fixed << setprecision(3) << r.buildMs
                 << setw(12) << fixed << setprecision(3) << r.lookupMs
                 << setw(12) << fixed << setprecision(2) << r.memoryKB
                 << setw(10) << r.correctHits
                 << setw(10) << r.wrongHits << "\n";
        }
        cout << string(100, '-') << "\n";
    }

    writeCSV("part3_results.csv", results);

    int largest = actualSizes.back();
    vector<Result> summary;
    for (const auto& r : results) if (r.wordCount == largest) summary.push_back(r);
    writeCSV("part3_summary.csv", summary);

    cout << "\nCorrectness meaning:\n";
    cout << "OK  = number of real dictionary words found. It should equal the number of present queries.\n";
    cout << "Bad = number of fake words found. It should be 0.\n";
    cout << "\nFiles written:\n";
    cout << "  part3_results.csv\n";
    cout << "  part3_summary.csv\n";
    cout << "\nNote: memory is an estimated internal memory value, not exact OS memory usage.\n";
    return 0;
}
