// ============================================================
//  Fast & Furious: Dictionary Races
//  ─────────────────────────────────────────────────────────
//  Data structures:
//    1. Ordinary Trie
//    2. Ternary Search Tree  (balanced via random shuffle)
//    3. Double Array Trie    (offline freeze construction)
//    4. Red-Black Tree       ("Me Spell Rite" spell checker)
//
//  Compile:  g++ -O2 -std=c++17 dictionary_races.cpp -o races
//  Run:      ./races words.txt
//            (on Linux: cp /usr/share/dict/words words.txt)
// ============================================================

#include <bits/stdc++.h>
using namespace std;
using clk = chrono::high_resolution_clock;

static double msec(clk::time_point a, clk::time_point b) {
    return chrono::duration<double, milli>(b - a).count();
}

// ============================================================
// 1. ORDINARY TRIE
//    Each node has 26 child pointers (one per letter a-z).
//    insert:  walk / create nodes for each letter
//    search:  walk; return false if any pointer is null
// ============================================================
struct ONode {
    ONode* ch[26]{};   // child for 'a'..'z'
    bool   end = false;
};

class OrdTrie {
    ONode* root;
    size_t cnt = 1;    // count nodes for memory estimate
public:
    OrdTrie() : root(new ONode) {}

    void insert(const string& w) {
        ONode* n = root;
        for (char c : w) {
            int i = c - 'a';
            if (!n->ch[i]) { n->ch[i] = new ONode; ++cnt; }
            n = n->ch[i];
        }
        n->end = true;
    }

    bool search(const string& w) const {
        ONode* n = root;
        for (char c : w) {
            n = n->ch[c - 'a'];
            if (!n) return false;
        }
        return n->end;
    }

    size_t memBytes() const { return cnt * sizeof(ONode); }
};

// ============================================================
// 2. TERNARY SEARCH TREE  (TST)
//    Each node stores ONE character and has three children:
//      L  – current char is less than node char
//      M  – current char matches; move to next char in word
//      R  – current char is greater than node char
//
//    Balancing strategy: RANDOM SHUFFLE before insertion.
//    Inserting words in random order gives expected O(log n)
//    tree height (Sedgewick & Bentley, 1997).
//    This avoids the worst-case O(n) height of sorted input.
// ============================================================
struct TNode {
    char   c;
    bool   end = false;
    TNode *L{}, *M{}, *R{};
    explicit TNode(char c) : c(c) {}
};

class TernaryTrie {
    TNode* root = nullptr;
    size_t cnt  = 0;

    TNode* ins(TNode* n, const string& w, int i) {
        if (!n) { n = new TNode(w[i]); ++cnt; }
        if      (w[i] < n->c)            n->L = ins(n->L, w, i);
        else if (w[i] > n->c)            n->R = ins(n->R, w, i);
        else if (i + 1 < (int)w.size())  n->M = ins(n->M, w, i + 1);
        else                             n->end = true;
        return n;
    }

    bool srch(TNode* n, const string& w, int i) const {
        if (!n) return false;
        if      (w[i] < n->c)            return srch(n->L, w, i);
        else if (w[i] > n->c)            return srch(n->R, w, i);
        else if (i + 1 < (int)w.size())  return srch(n->M, w, i + 1);
        else                             return n->end;
    }

public:
    // ← The balancing step: shuffle BEFORE inserting
    void buildBalanced(vector<string> words) {
        mt19937 rng(42);
        shuffle(words.begin(), words.end(), rng);
        for (const auto& w : words)
            if (!w.empty()) root = ins(root, w, 0);
    }

    bool search(const string& w) const {
        return !w.empty() && srch(root, w, 0);
    }

    size_t memBytes() const { return cnt * sizeof(TNode); }
};

// ============================================================
// 3. DOUBLE ARRAY TRIE  (Aoe 1989)
//    Stores the trie in two integer arrays: base[] and check[]
//
//    Transition rule:
//      From state s, reading character c:
//        next_state = base[s] + c
//        valid only if check[next_state] == s
//
//    Construction (two-phase):
//      Phase 1: insert all words into a plain temporary trie
//      Phase 2: BFS "freeze" — assign a base value for each
//               node so all children fit without collision,
//               then fill base[] and check[]
//
//    Lookup is O(|word|) and very cache-friendly (two arrays).
// ============================================================
class DAT {
    // ── Phase 1: temporary trie ──────────────────────────────
    struct TmpNode {
        map<int,int> ch;    // char_code (1..26) → child node id
        bool end = false;
    };
    vector<TmpNode> tmp;    // tmp[0] = root

    // ── Phase 2: double arrays ───────────────────────────────
    vector<int>  base_a;   // base_a[s]  = base value for state s
    vector<int>  check_a;  // check_a[s] = parent state, -1 if free
    vector<bool> end_a;    // end_a[s]   = true if word ends here
    bool frozen = false;

    // Grow arrays until index 'need' exists
    void grow(int need) {
        while ((int)check_a.size() <= need) {
            base_a.push_back(1);
            check_a.push_back(-1);
            end_a.push_back(false);
        }
    }

    // Find smallest b ≥ 1 such that base[s]+c is FREE for all c in chars
    int findBase(const vector<int>& chars) {
        int mc = *max_element(chars.begin(), chars.end());
        for (int b = 1; ; ++b) {
            grow(b + mc);
            bool ok = true;
            for (int c : chars) {
                if (check_a[b + c] != -1) { ok = false; break; }
            }
            if (ok) return b;
        }
    }

public:
    DAT() { tmp.emplace_back(); }  // node 0 = root

    // Phase 1: build temp trie
    void insert(const string& w) {
        int cur = 0;
        for (char ch : w) {
            int c = ch - 'a' + 1;              // map 'a'→1, 'z'→26
            auto it = tmp[cur].ch.find(c);
            if (it == tmp[cur].ch.end()) {
                tmp[cur].ch[c] = (int)tmp.size();
                tmp.emplace_back();
            }
            cur = tmp[cur].ch[c];
        }
        tmp[cur].end = true;
    }

    // Phase 2: BFS freeze — MUST call after all inserts
    void freeze() {
        int N = (int)tmp.size();
        base_a.assign(N * 2 + 128, 1);
        check_a.assign(N * 2 + 128, -1);
        end_a.assign(N * 2 + 128, false);

        // Map: tmp node id → DAT state index
        vector<int> toState(N, -1);
        toState[0] = 0;
        check_a[0] = 0;           // root's check points to itself
        end_a[0]   = tmp[0].end;

        queue<int> q;
        q.push(0);

        while (!q.empty()) {
            int nid = q.front(); q.pop();
            int s   = toState[nid];

            if (tmp[nid].ch.empty()) {
                base_a[s] = 0;   // leaf – no children, base unused
                continue;
            }

            // Collect child char codes and find a fitting base
            vector<int> chars;
            for (auto& [c, _] : tmp[nid].ch) chars.push_back(c);
            int b = findBase(chars);
            base_a[s] = b;

            // Place each child in the double arrays
            for (auto& [c, cid] : tmp[nid].ch) {
                int t = b + c;
                grow(t);
                check_a[t]   = s;
                end_a[t]     = tmp[cid].end;
                toState[cid] = t;
                q.push(cid);
            }
        }

        frozen = true;
        tmp.clear();
        tmp.shrink_to_fit();   // release temporary trie memory
    }

    bool search(const string& w) const {
        int s = 0;
        for (char ch : w) {
            int c = ch - 'a' + 1;
            int t = base_a[s] + c;
            if (t >= (int)check_a.size() || check_a[t] != s)
                return false;
            s = t;
        }
        return end_a[s];
    }

    size_t memBytes() const {
        return base_a.size()  * sizeof(int)
             + check_a.size() * sizeof(int)
             + end_a.size()   * sizeof(bool);
    }
};

// ============================================================
// 4. RED-BLACK TREE  ("Me Spell Rite")
//    CLRS Chapter 13 textbook implementation.
//
//    Properties guaranteed after every insert:
//      1. Every node is RED or BLACK
//      2. Root is BLACK
//      3. No two consecutive RED nodes (red nodes have black parents)
//      4. Every path from a node to its null leaves has the
//         same number of BLACK nodes  ← guarantees O(log n) height
//
//    fixIns() restores these properties after each insertion
//    using at most two rotations and O(log n) recolorings.
// ============================================================
enum Color { RED, BLACK };

struct RBNode {
    string key;
    Color  col;
    RBNode *L, *R, *P;    // left, right, parent
    RBNode(const string& k, RBNode* nil)
        : key(k), col(RED), L(nil), R(nil), P(nil) {}
};

class RBTree {
    RBNode* NIL;   // sentinel (black dummy leaf)
    RBNode* root;
    size_t  cnt = 0;

    // ── rotations ────────────────────────────────────────────
    void rotL(RBNode* x) {
        RBNode* y = x->R;
        x->R = y->L;
        if (y->L != NIL) y->L->P = x;
        y->P = x->P;
        if      (x->P == NIL)    root    = y;
        else if (x == x->P->L)  x->P->L = y;
        else                     x->P->R = y;
        y->L = x;  x->P = y;
    }

    void rotR(RBNode* x) {
        RBNode* y = x->L;
        x->L = y->R;
        if (y->R != NIL) y->R->P = x;
        y->P = x->P;
        if      (x->P == NIL)    root    = y;
        else if (x == x->P->R)  x->P->R = y;
        else                     x->P->L = y;
        y->R = x;  x->P = y;
    }

    // ── fix RB properties after insertion ────────────────────
    void fixIns(RBNode* z) {
        while (z->P->col == RED) {
            if (z->P == z->P->P->L) {       // parent is LEFT child
                RBNode* y = z->P->P->R;      // uncle
                if (y->col == RED) {
                    // Case 1: uncle RED → recolor, move z up
                    z->P->col    = BLACK;
                    y->col       = BLACK;
                    z->P->P->col = RED;
                    z = z->P->P;
                } else {
                    if (z == z->P->R) {      // Case 2: z is right child
                        z = z->P;            //   → rotate to make it left
                        rotL(z);
                    }
                    // Case 3: z is left child → rotate grandparent
                    z->P->col    = BLACK;
                    z->P->P->col = RED;
                    rotR(z->P->P);
                }
            } else {                         // parent is RIGHT child (mirror)
                RBNode* y = z->P->P->L;
                if (y->col == RED) {
                    z->P->col    = BLACK;
                    y->col       = BLACK;
                    z->P->P->col = RED;
                    z = z->P->P;
                } else {
                    if (z == z->P->L) {
                        z = z->P;
                        rotR(z);
                    }
                    z->P->col    = BLACK;
                    z->P->P->col = RED;
                    rotL(z->P->P);
                }
            }
        }
        root->col = BLACK;   // Property 2: root is always black
    }

public:
    RBTree() {
        NIL       = new RBNode("", nullptr);
        NIL->col  = BLACK;
        NIL->L = NIL->R = NIL->P = NIL;   // sentinel points to itself
        root = NIL;
    }

    void insert(const string& key) {
        RBNode* z = new RBNode(key, NIL);
        RBNode* y = NIL, *x = root;
        // BST walk to find insertion point
        while (x != NIL) {
            y = x;
            x = (key < x->key) ? x->L : x->R;
        }
        z->P = y;
        if      (y == NIL)     root  = z;
        else if (key < y->key) y->L  = z;
        else                   y->R  = z;
        ++cnt;
        fixIns(z);   // restore RB properties
    }

    bool search(const string& key) const {
        RBNode* x = root;
        while (x != NIL) {
            if      (key == x->key) return true;
            else if (key <  x->key) x = x->L;
            else                    x = x->R;
        }
        return false;
    }

    size_t memBytes() const { return (cnt + 1) * sizeof(RBNode); }
};

// ============================================================
// UTILITIES
// ============================================================

// Load words from file, lowercase, letters only, deduplicated
vector<string> loadWords(const string& path) {
    ifstream f(path);
    vector<string> words;
    string w;
    while (f >> w) {
        string clean;
        for (char c : w)
            if (isalpha((unsigned char)c)) clean += tolower(c);
        if (!clean.empty()) words.push_back(move(clean));
    }
    sort(words.begin(), words.end());
    words.erase(unique(words.begin(), words.end()), words.end());
    return words;
}

struct Stats {
    string name;
    double buildMs;
    double lookupMs;
        double memKB;
};

// ============================================================
// CSV REPORT
// ============================================================
void writeCSV(const vector<Stats>& st) {
        ofstream f("results.csv");
        f << "structure,build_ms,lookup_ms,mem_kb\n";
        f << fixed << setprecision(4);
        for (const auto& s : st) {
                f << quoted(s.name) << ','
                    << s.buildMs << ','
                    << s.lookupMs << ','
                    << s.memKB << '\n';
        }
        cout << "Results saved to results.csv\n";
}

// ============================================================
// MAIN
// ============================================================
int main(int argc, char* argv[]) {
    const string path = (argc > 1) ? argv[1] : "words.txt";

    cout << "Loading: " << path << "\n";
    auto words = loadWords(path);

    if (words.empty()) {
        cerr << "No words found at \"" << path << "\"\n";
        cerr << "On Linux: cp /usr/share/dict/words words.txt\n";
        cerr << "On Mac:   cp /usr/share/dict/web2 words.txt\n";
        return 1;
    }

    printf("Loaded %zu unique words.\n\n", words.size());

    // Build query set: first 1000 dictionary words + 2 non-words
    vector<string> queries;
    for (int i = 0; i < min((int)words.size(), 1000); ++i)
        queries.push_back(words[i]);
    queries.push_back("zzznope");
    queries.push_back("xyzwrong");

    printf("%-35s  %10s  %12s  %10s\n",
           "Structure", "Build(ms)", "Lookup(ms)", "Mem(KB)");
    printf("%s\n", string(75, '-').c_str());

    vector<Stats> results;

    // Helper: time build, time lookup, record stats
    auto bench = [&](const string& name, auto buildFn, auto searchFn, auto memFn) {
        auto t0 = clk::now();
        buildFn();
        auto t1 = clk::now();

        volatile int hits = 0;
        for (const auto& q : queries) hits += searchFn(q);
        auto t2 = clk::now();

        double bms = msec(t0, t1), lms = msec(t1, t2);
         double mem  = memFn() / 1024.0;
         printf("%-35s  %10.2f  %12.4f  %10.2f\n",
               name.c_str(), bms, lms, mem);
        results.push_back({name, bms, lms, mem});
        (void)hits;
    };

    // 1. Ordinary Trie
    {
        OrdTrie t;
        bench("Ordinary Trie",
              [&]{ for (const auto& w : words) t.insert(w); },
              [&](const string& q){ return t.search(q); },
              [&]{ return t.memBytes(); });
    }

    // 2. Ternary Search Tree (shuffle-balanced)
    {
        TernaryTrie t;
        bench("Ternary Trie (shuffle-balanced)",
              [&]{ t.buildBalanced(words); },     // shuffle happens inside
              [&](const string& q){ return t.search(q); },
              [&]{ return t.memBytes(); });
    }

    // 3. Double Array Trie
    {
        DAT d;
        bench("Double Array Trie",
              [&]{
                  for (const auto& w : words) d.insert(w);
                  d.freeze();    // convert temp trie → double arrays
              },
              [&](const string& q){ return d.search(q); },
              [&]{ return d.memBytes(); });
    }

    // 4. Red-Black Tree ("Me Spell Rite")
    {
        RBTree rb;
        bench("Red-Black Tree (Spell Rite)",
              [&]{ for (const auto& w : words) rb.insert(w); },
              [&](const string& q){ return rb.search(q); },
              [&]{ return rb.memBytes(); });
    }

    printf("\n");
    writeCSV(results);
    return 0;
}