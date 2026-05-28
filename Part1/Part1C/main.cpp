#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stack>
#include <utility>

using namespace std;

// Reads the guest names and conflict pairs from the input file.
pair<vector<string>, vector<pair<int, int>>> readGuestData(string filename) {
    ifstream file(filename);
    if (!file.is_open() && filename == "guests.txt") {
        filename = "guest.txt";
        file.open(filename);
    }
    if (!file.is_open()) {
        cout << "Error: Could not open guest file" << endl;
        return {{}, {}};
    }

    int numGuests, numAnimosities;
    // First two numbers tell how many guests and conflicts are in the file.
    file >> numGuests >> numAnimosities;

    vector<string> guests(numGuests);
    // Read all guest names in the same order as their indexes.
    for (int i = 0; i < numGuests; i++) {
        file >> guests[i];
    }

    vector<pair<int, int>> animosities(numAnimosities);
    // Each pair means two guests cannot sit at the same table.
    for (int i = 0; i < numAnimosities; i++) {
        int guest1, guest2;
        file >> guest1 >> guest2;
        animosities[i] = {guest1, guest2};
    }

    return {guests, animosities};
}

// Builds an undirected graph where each edge means two guests cannot sit together.
vector<vector<int>> createGraph(int numGuests, const vector<pair<int, int>>& animosities) {
    vector<vector<int>> graph(numGuests);

    // Add both directions because the conflict is mutual.
    for (const auto& animosity : animosities) {
        int guest1 = animosity.first;
        int guest2 = animosity.second;

        graph[guest1].push_back(guest2);
        graph[guest2].push_back(guest1);
    }

    return graph;
}

// Uses a stack-based coloring check to see whether the graph can be split into two tables.
bool isBipartite(const vector<vector<int>>& graph, vector<int>& colors) {
    int n = graph.size();
    colors.assign(n, -1);

    // Try every unvisited guest in case the graph has more than one group.
    for (int start = 0; start < n; start++) {
        if (colors[start] != -1) continue;

        stack<int> stk;
        // Start the first table color from this guest.
        stk.push(start);
        colors[start] = 0;

        while (!stk.empty()) {
            int vertex = stk.top();
            stk.pop();

            // Neighbors must get the opposite table color.
            for (int neighbor : graph[vertex]) {
                if (colors[neighbor] == -1) {
                    colors[neighbor] = 1 - colors[vertex];
                    stk.push(neighbor);
                } else if (colors[neighbor] == colors[vertex]) {
                    // Same color on both ends means the 2-table plan fails.
                    return false;
                }
            }
        }
    }

    return true;
}

// Prints the guests assigned to each table based on the coloring result.
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
    // Default input file for this task.
    string guestFile = "guests.txt";

    auto [guests, animosities] = readGuestData(guestFile);
    if (guests.empty()) {
        cout << "Failed to read guest data" << endl;
        return 1;
    }

    // Turn the conflicts into a graph before checking the seating plan.
    vector<vector<int>> graph = createGraph(guests.size(), animosities);

    vector<int> colors;
    bool possible = isBipartite(graph, colors);

    // If the graph is bipartite, the guests can be split into two valid tables.
    if (possible) {
        cout << "Good news! A 2-table arrangement is possible!" << endl;
        printSeatingArrangement(guests, colors);
    } else {
        cout << "Sorry, it's impossible to arrange guests in 2 tables without conflicts." << endl;
        cout << "Aunt Petunia needs more tables or fewer guests!" << endl;
    }

    return 0;
}