#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stack>

using namespace std;

// Reads guest names from guests.txt.
// One guest name per line.
vector<string> readGuests(string filename) {
    ifstream file(filename);

    if (!file.is_open()) {
        cout << "Error: Could not open guests file" << endl;
        return {};
    }

    vector<string> guests;
    string name;

    while (file >> name) {
        guests.push_back(name);
    }

    return guests;
}

// Finds the index of a guest name.
// Returns -1 if the name is not found.
int findGuestIndex(const vector<string>& guests, string name) {
    for (int i = 0; i < guests.size(); i++) {
        if (guests[i] == name) {
            return i;
        }
    }

    return -1;
}

// Reads dislike pairs from dislikes.txt.
// Each line has two names:
// Alice Bob
vector<pair<int, int>> readDislikes(string filename, const vector<string>& guests) {
    ifstream file(filename);

    if (!file.is_open()) {
        cout << "Error: Could not open dislikes file" << endl;
        return {};
    }

    vector<pair<int, int>> dislikes;
    string name1, name2;

    while (file >> name1 >> name2) {
        int guest1 = findGuestIndex(guests, name1);
        int guest2 = findGuestIndex(guests, name2);

        if (guest1 == -1 || guest2 == -1) {
            cout << "Warning: Unknown guest in dislike pair: "
                 << name1 << " " << name2 << endl;
        } else {
            dislikes.push_back({guest1, guest2});
        }
    }

    return dislikes;
}

// Builds an undirected graph.
// Each guest is a node.
// Each dislike pair is an edge.
vector<vector<int>> createGraph(int numGuests, const vector<pair<int, int>>& dislikes) {
    vector<vector<int>> graph(numGuests);

    for (const auto& dislike : dislikes) {
        int guest1 = dislike.first;
        int guest2 = dislike.second;

        graph[guest1].push_back(guest2);
        graph[guest2].push_back(guest1);
    }

    return graph;
}

// Non-recursive DFS using a stack.
// This checks if the graph is bipartite.
//
// color 0 = Table 1
// color 1 = Table 2
// color -1 = not assigned yet
bool isBipartite(const vector<vector<int>>& graph, vector<int>& colors) {
    int n = graph.size();
    colors.assign(n, -1);

    for (int start = 0; start < n; start++) {
        if (colors[start] != -1) {
            continue;
        }

        stack<int> stk;
        stk.push(start);
        colors[start] = 0;

        while (!stk.empty()) {
            int current = stk.top();
            stk.pop();

            for (int neighbor : graph[current]) {
                if (colors[neighbor] == -1) {
                    colors[neighbor] = 1 - colors[current];
                    stk.push(neighbor);
                } else if (colors[neighbor] == colors[current]) {
                    return false;
                }
            }
        }
    }

    return true;
}

// Prints the two-table seating arrangement.
void printSeatingArrangement(const vector<string>& guests, const vector<int>& colors) {
    cout << "Table 1 guests:" << endl;

    for (int i = 0; i < guests.size(); i++) {
        if (colors[i] == 0) {
            cout << "  " << guests[i] << endl;
        }
    }

    cout << "\nTable 2 guests:" << endl;

    for (int i = 0; i < guests.size(); i++) {
        if (colors[i] == 1) {
            cout << "  " << guests[i] << endl;
        }
    }
}

int main() {
    string guestsFile = "guests.txt";
    string dislikesFile = "dislikes.txt";

    vector<string> guests = readGuests(guestsFile);

    if (guests.empty()) {
        cout << "Failed to read guests" << endl;
        return 1;
    }

    vector<pair<int, int>> dislikes = readDislikes(dislikesFile, guests);

    vector<vector<int>> graph = createGraph(guests.size(), dislikes);

    vector<int> colors;

    bool possible = isBipartite(graph, colors);

    if (possible) {
        cout << "A 2-table arrangement is possible." << endl;
        printSeatingArrangement(guests, colors);
    } else {
        cout << "It is impossible to arrange the guests at only 2 tables without conflicts." << endl;
        cout << "More than two tables would be needed." << endl;
    }

    return 0;
}