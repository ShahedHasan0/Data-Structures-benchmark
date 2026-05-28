#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <fstream>

using namespace std;

// Triwizard labyrinth solver:
// Reads a grid and three wizards, computes who reaches exit first.

struct Position {
    int row, col;
};

struct Wizard {
    string name;
    Position position;
    double speed;
};

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

// Read wizard entries: name row col speed (three lines expected)

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

vector<vector<int>> bfsDistances(const vector<vector<char>>& labyrinth, Position start) {
    int rows = labyrinth.size();
    if (rows == 0) return {};
    int cols = labyrinth[0].size();

    int dr[] = {-1, 0, 1, 0};
    int dc[] = {0, 1, 0, -1};

    vector<vector<bool>> visited(rows, vector<bool>(cols, false));
    vector<vector<int>> distance(rows, vector<int>(cols, -1));

    // Standard BFS from `start` (we will use exit as start) to get
    // minimum steps from every cell to the exit. This is the single
    // BFS required by the assignment.
    //
    // Setup:
    // - `visited` prevents revisiting cells
    // - `distance` stores number of corridor steps from `start` (-1 if unreachable)
    // - queue holds frontier cells to expand in FIFO order (ensures shortest paths)
    queue<Position> q;
    q.push(start);
    visited[start.row][start.col] = true;
    distance[start.row][start.col] = 0;

    while (!q.empty()) {
        Position current = q.front();
        q.pop();

        for (int i = 0; i < 4; i++) {
            int newRow = current.row + dr[i];
            int newCol = current.col + dc[i];

            if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols &&
                !visited[newRow][newCol] && labyrinth[newRow][newCol] != '#') {
                // neighbor is valid and not a wall: mark visited and set distance
                visited[newRow][newCol] = true;
                // one more corridor step from current
                distance[newRow][newCol] = distance[current.row][current.col] + 1;
                Position next = {newRow, newCol};
                // push to queue to continue BFS
                q.push(next);
            }
        }
    }

    return distance;
}

string predictWinner(const vector<vector<char>>& labyrinth, const vector<Wizard>& wizards, const Position& exit) {
    if (wizards.size() != 3) {
        return "Error: Need exactly 3 wizards";
    }

    // Run BFS exactly once from the exit to compute distances to every cell
    vector<vector<int>> distances = bfsDistances(labyrinth, exit);
    if (distances.empty()) return "Error: labyrinth empty";

    vector<double> arrivalTimes;
    for (const Wizard& wizard : wizards) {
        // Look up shortest path length (in corridors) from wizard to exit.
        // If -1, the cell cannot reach the exit.
        int pathLength = distances[wizard.position.row][wizard.position.col];
        if (pathLength == -1) {
            // unreachable wizard: assign very large time so they cannot win
            arrivalTimes.push_back(1e12);
        } else {
            // compute travel time in minutes: corridors / (corridors per minute)
            double time = static_cast<double>(pathLength) / wizard.speed;
            arrivalTimes.push_back(time);
        }
    }

    int winnerIndex = 0;
    for (int i = 1; i < arrivalTimes.size(); i++) {
        if (arrivalTimes[i] < arrivalTimes[winnerIndex]) {
            winnerIndex = i;
        }
    }

    return wizards[winnerIndex].name;
}

void printLabyrinth(const vector<vector<char>>& labyrinth, const vector<Wizard>& wizards) {
    if (labyrinth.empty()) return;

    cout << "Labyrinth:" << endl;
    for (int i = 0; i < labyrinth.size(); i++) {
        for (int j = 0; j < labyrinth[i].size(); j++) {
            bool isWizard = false;
            for (int k = 0; k < wizards.size(); k++) {
                if (wizards[k].position.row == i && wizards[k].position.col == j) {
                    cout << k+1; // Show wizard number
                    isWizard = true;
                    break;
                }
            }
            if (!isWizard) {
                cout << labyrinth[i][j];
            }
        }
        cout << endl;
    }
}

int main() {
    // Use relative file names; run from the folder containing these files
    string labyrinthFile = "labyrinth.txt";
    string wizardFile = "wizards.txt";

    Position exit;
    vector<vector<char>> labyrinth = readLabyrinth(labyrinthFile, exit);
    if (labyrinth.empty()) {
        cout << "Failed to read labyrinth" << endl;
        return 1;
    }

    vector<Wizard> wizards = readWizards(wizardFile);
    if (wizards.empty() || wizards.size() != 3) {
        cout << "Failed to read wizard data or not enough wizards" << endl;
        return 1;
    }

    printLabyrinth(labyrinth, wizards);

    cout << "\nWizards:" << endl;
    for (const Wizard& wizard : wizards) {
        cout << wizard.name << " at position (" << wizard.position.row << ","
             << wizard.position.col << ") with speed " << wizard.speed << " corridors/min" << endl;
    }

    string winner = predictWinner(labyrinth, wizards, exit);
    cout << "\nThe winner will be: " << winner << endl;

    return 0;
}