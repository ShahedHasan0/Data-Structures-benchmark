#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <fstream>

using namespace std;

// This program solves Assignment 2 Part 1B:
// "Triwizard Tournament".
//
// Goal:
// Predict which wizard reaches the labyrinth exit first.
//
// Important:
// BFS is used exactly once as required by the assignment.


// Stores coordinates inside the labyrinth grid.
struct Position {
    int row, col;
};

// Stores wizard information:
// - name
// - starting position
// - speed in corridors per minute
struct Wizard {
    string name;
    Position position;
    double speed;
};


// Reads the labyrinth from a text file.
//
// '#' = wall
// '.' = corridor
// 'E' = exit
//
// The labyrinth is stored as a 2D vector of characters.
// While reading the map, we also locate the exit position.
vector<vector<char>> readLabyrinth(string filename, Position& exit) {
    ifstream file(filename);

    if (!file.is_open()) {
        cout << "Error: Could not open labyrinth file" << endl;
        return {};
    }

    vector<vector<char>> labyrinth;
    string line;
    int row = 0;

    while (getline(file, line)) {
        vector<char> rowData;

        for (int col = 0; col < line.length(); col++) {
            rowData.push_back(line[col]);

            // Save exit coordinates.
            if (line[col] == 'E') {
                exit.row = row;
                exit.col = col;
            }
        }

        labyrinth.push_back(rowData);
        row++;
    }

    return labyrinth;
}


// Reads wizard data from a text file.
//
// Expected format:
// name row col speed
//
// Example:
// Harry 1 1 2.0
vector<Wizard> readWizards(string filename) {
    ifstream file(filename);

    if (!file.is_open()) {
        cout << "Error: Could not open wizard file" << endl;
        return {};
    }

    vector<Wizard> wizards;

    string name;
    int row, col;
    double speed;

    // Read exactly 3 wizards.
    for (int i = 0; i < 3; i++) {
        if (file >> name >> row >> col >> speed) {

            Wizard wizard;

            wizard.name = name;
            wizard.position.row = row;
            wizard.position.col = col;
            wizard.speed = speed;

            wizards.push_back(wizard);
        }
    }

    return wizards;
}


// BFS computes shortest distances from the exit
// to every reachable cell.
//
// BFS is used exactly once in the whole program.
//
// Why BFS?
// Because the labyrinth is an unweighted graph,
// and BFS guarantees shortest paths.
vector<vector<int>> bfsDistances(
    const vector<vector<char>>& labyrinth,
    Position start
) {
    int rows = labyrinth.size();

    if (rows == 0) return {};

    int cols = labyrinth[0].size();

    // Movement directions:
    // up, right, down, left
    int dr[] = {-1, 0, 1, 0};
    int dc[] = {0, 1, 0, -1};

    // visited prevents revisiting cells.
    vector<vector<bool>> visited(
        rows,
        vector<bool>(cols, false)
    );

    // distance[row][col] stores minimum corridor steps
    // from that cell to the exit.
    //
    // -1 means unreachable.
    vector<vector<int>> distance(
        rows,
        vector<int>(cols, -1)
    );

    // BFS queue.
    // FIFO order guarantees shortest paths.
    queue<Position> q;

    q.push(start);

    visited[start.row][start.col] = true;
    distance[start.row][start.col] = 0;

    while (!q.empty()) {

        Position current = q.front();
        q.pop();

        // Explore all 4 neighbors.
        for (int i = 0; i < 4; i++) {

            int newRow = current.row + dr[i];
            int newCol = current.col + dc[i];

            bool inside =
                newRow >= 0 &&
                newRow < rows &&
                newCol >= 0 &&
                newCol < cols;

            // Valid cell:
            // - inside labyrinth
            // - not visited
            // - not a wall
            if (
                inside &&
                !visited[newRow][newCol] &&
                labyrinth[newRow][newCol] != '#'
            ) {

                visited[newRow][newCol] = true;

                // One more corridor step.
                distance[newRow][newCol] =
                    distance[current.row][current.col] + 1;

                Position next = {newRow, newCol};

                q.push(next);
            }
        }
    }

    return distance;
}


// Predicts which wizard reaches the exit first.
//
// Formula:
// time = distance / speed
string predictWinner(
    const vector<vector<int>>& distances,
    const vector<Wizard>& wizards
) {
    if (wizards.size() != 3) {
        return "Error: Need exactly 3 wizards";
    }

    double bestTime = 1e12;

    string winner = "No reachable wizard";

    for (const Wizard& wizard : wizards) {

        int r = wizard.position.row;
        int c = wizard.position.col;

        // Ignore invalid positions.
        if (
            r < 0 || r >= distances.size() ||
            c < 0 || c >= distances[0].size()
        ) {
            continue;
        }

        int pathLength = distances[r][c];

        // Ignore unreachable wizards.
        if (pathLength == -1 || wizard.speed <= 0) {
            continue;
        }

        // Travel time:
        // corridors / corridors per minute
        double time =
            static_cast<double>(pathLength) / wizard.speed;

        // Keep smallest time.
        if (time < bestTime) {
            bestTime = time;
            winner = wizard.name;
        }
    }

    return winner;
}


// Prints the labyrinth visually.
//
// Wizards are shown as:
// 1, 2, 3
void printLabyrinth(
    const vector<vector<char>>& labyrinth,
    const vector<Wizard>& wizards
) {
    if (labyrinth.empty()) return;

    cout << "Labyrinth:" << endl;

    for (int i = 0; i < labyrinth.size(); i++) {

        for (int j = 0; j < labyrinth[i].size(); j++) {

            bool isWizard = false;

            // Check whether a wizard is standing here.
            for (int k = 0; k < wizards.size(); k++) {

                if (
                    wizards[k].position.row == i &&
                    wizards[k].position.col == j
                ) {

                    // Print wizard number.
                    cout << k + 1;

                    isWizard = true;

                    break;
                }
            }

            // Otherwise print labyrinth cell.
            if (!isWizard) {
                cout << labyrinth[i][j];
            }
        }

        cout << endl;
    }
}


int main() {

    // Input file names.
    string labyrinthFile = "labyrinth.txt";
    string wizardFile = "wizards.txt";

    // Exit position.
    Position exit = {-1, -1};

    // Read labyrinth.
    vector<vector<char>> labyrinth =
        readLabyrinth(labyrinthFile, exit);

    if (labyrinth.empty()) {
        cout << "Failed to read labyrinth" << endl;
        return 1;
    }

    // Ensure exit exists.
    if (exit.row == -1 || exit.col == -1) {
        cout << "Error: No exit E found in labyrinth" << endl;
        return 1;
    }

    // Read wizard data.
    vector<Wizard> wizards =
        readWizards(wizardFile);

    if (wizards.size() != 3) {
        cout << "Failed to read wizard data" << endl;
        return 1;
    }

    // Display labyrinth.
    printLabyrinth(labyrinth, wizards);

    // Print wizard information.
    cout << "\nWizards:" << endl;

    for (const Wizard& wizard : wizards) {

        cout << wizard.name
             << " at position ("
             << wizard.position.row
             << ", "
             << wizard.position.col
             << ") with speed "
             << wizard.speed
             << " corridors/min"
             << endl;
    }

    // BFS is called EXACTLY ONCE here.
    vector<vector<int>> distances =
        bfsDistances(labyrinth, exit);

    // Show distances and times.
    cout << "\nResults:" << endl;

    for (const Wizard& wizard : wizards) {

        int r = wizard.position.row;
        int c = wizard.position.col;

        cout << wizard.name << ": ";

        int distance = distances[r][c];

        if (distance == -1) {

            cout << "cannot reach the exit" << endl;

        } else {

            double time =
            static_cast<double>(distance) / wizard.speed;

            cout << "distance = "
                 << distance
                 << ", time = "
                 << time
                 << " minutes"
                 << endl;
        }
    }

    // Predict winner.
    string winner =
        predictWinner(distances, wizards);

    cout << "\nThe winner will be: "
         << winner
         << endl;

    return 0;
}