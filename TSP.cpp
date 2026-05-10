#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <queue>

using namespace std;

int adjMatrix[100][100];
int numCities;
string cityNames[100];

struct Edge {
    int to, weight;
};

vector<Edge> adjList[100];

bool loadGraph(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Cannot open file '" << filename << "'\n";
        return false;
    }

    file >> numCities;

    for (int i = 0; i < numCities; i++)
        file >> cityNames[i];

    for (int i = 0; i < numCities; i++)
        for (int j = 0; j < numCities; j++)
            file >> adjMatrix[i][j];

    for (int i = 0; i < numCities; i++) {
        adjList[i].clear();

        for (int j = 0; j < numCities; j++)
            if (i != j && adjMatrix[i][j] != 0)
                adjList[i].push_back({ j, adjMatrix[i][j] });
    }

    file.close();
    return true;
}

void printMatrix() {
    cout << "\n--- Adjacency Matrix (Static) ---\n";
    cout << "    ";

    for (int i = 0; i < numCities; i++)
        cout << cityNames[i] << " ";

    cout << "\n";

    for (int i = 0; i < numCities; i++) {
        cout << cityNames[i] << " ";

        for (int j = 0; j < numCities; j++)
            cout << adjMatrix[i][j] << " ";

        cout << "\n";
    }
}

void printList() {
    cout << "\n--- Adjacency List (Dynamic) ---\n";

    for (int i = 0; i < numCities; i++) {
        cout << cityNames[i] << " -> ";

        for (int j = 0; j < (int)adjList[i].size(); j++)
            cout << cityNames[adjList[i][j].to]
                 << "(" << adjList[i][j].weight << ") ";

        cout << "\n";
    }
}

int routeCost(const vector<int>& route) {
    int cost = 0;

    for (int i = 0; i < (int)route.size() - 1; i++)
        cost += adjMatrix[route[i]][route[i + 1]];

    cost += adjMatrix[route.back()][route[0]];

    return cost;
}

void printRoute(const vector<int>& route) {
    for (int i = 0; i < (int)route.size(); i++) {
        cout << cityNames[route[i]];

        if (i < (int)route.size() - 1)
            cout << " -> ";
    }

    cout << " -> " << cityNames[route[0]] << "\n";
    cout << "Cost: " << routeCost(route) << "\n";
}

vector<int> bruteForceTSP(int startCity) {
    vector<int> cities;

    for (int i = 0; i < numCities; i++)
        if (i != startCity)
            cities.push_back(i);

    sort(cities.begin(), cities.end());

    int bestCost = 999999999;
    vector<int> bestRoute;

    do {
        vector<int> route = { startCity };
        route.insert(route.end(), cities.begin(), cities.end());

        bool valid = true;

        for (int i = 0; i < (int)route.size() - 1; i++) {
            if (adjMatrix[route[i]][route[i + 1]] == 0) {
                valid = false;
                break;
            }
        }

        if (adjMatrix[route.back()][route[0]] == 0)
            valid = false;

        if (!valid)
            continue;

        int cost = routeCost(route);

        if (cost < bestCost) {
            bestCost = cost;
            bestRoute = route;
        }

    } while (next_permutation(cities.begin(), cities.end()));

    return bestRoute;
}

vector<int> greedyTSP(int startCity) {
    vector<bool> visited(numCities, false);
    vector<int> route;

    int current = startCity;

    route.push_back(current);
    visited[current] = true;

    for (int step = 1; step < numCities; step++) {

        int nearest = -1;
        int minDist = 999999999;

        for (int j = 0; j < numCities; j++) {

            if (!visited[j] &&
                adjMatrix[current][j] > 0 &&
                adjMatrix[current][j] < minDist) {

                minDist = adjMatrix[current][j];
                nearest = j;
            }
        }

        if (nearest == -1)
            break;

        route.push_back(nearest);
        visited[nearest] = true;
        current = nearest;
    }

    return route;
}

void dijkstraShortestPath() {

    cout << "\nAvailable cities:\n";

    for (int i = 0; i < numCities; i++)
        cout << "  " << i << " = " << cityNames[i] << "\n";

    int src, dst;

    cout << "Enter source city index: ";
    cin >> src;

    if (src < 0 || src >= numCities) {
        cout << "Invalid source.\n";
        return;
    }

    cout << "Enter destination city index: ";
    cin >> dst;

    if (dst < 0 || dst >= numCities) {
        cout << "Invalid destination.\n";
        return;
    }

    if (src == dst) {
        cout << "Source and destination are the same. Cost = 0.\n";
        return;
    }

    const int INF = 999999999;

    vector<int> dist(numCities, INF);
    vector<int> prev(numCities, -1);

    priority_queue<
        pair<int, int>,
        vector<pair<int, int>>,
        greater<pair<int, int>>
    > pq;

    dist[src] = 0;

    pq.push({ 0, src });

    while (!pq.empty()) {

        pair<int, int> top = pq.top();
        pq.pop();

        int d = top.first;
        int u = top.second;

        if (d > dist[u])
            continue;

        for (int v = 0; v < numCities; v++) {

            if (v == u || adjMatrix[u][v] <= 0 || dist[u] == INF)
                continue;

            int newDist = dist[u] + adjMatrix[u][v];

            if (newDist < dist[v]) {
                dist[v] = newDist;
                prev[v] = u;

                pq.push({ newDist, v });
            }
        }
    }

    if (dist[dst] == INF) {
        cout << "\nNo path found from "
             << cityNames[src]
             << " to "
             << cityNames[dst]
             << ".\n";

        return;
    }

    vector<int> path;

    for (int at = dst; at != -1; at = prev[at])
        path.push_back(at);

    reverse(path.begin(), path.end());

    cout << "\n=== Shortest Path (Dijkstra) ===\n";

    cout << "From: " << cityNames[src]
         << "  To: " << cityNames[dst] << "\n";

    cout << "Path: ";

    for (int i = 0; i < (int)path.size(); i++) {
        cout << cityNames[path[i]];

        if (i < (int)path.size() - 1)
            cout << " -> ";
    }

    cout << "\nTotal Cost: " << dist[dst] << "\n";
}

void runAlgorithm(int choice, int startCity) {

    vector<int> route;
    string name;

    if (choice == 1) {

        name = "Brute Force";

        route = bruteForceTSP(startCity);

        if (route.empty()) {

            cout << "\n=== Brute Force ===\n";

            cout << "No complete route found.\n";

            return;
        }
    }
    else if (choice == 2) {

        name = "Greedy (Nearest Neighbor)";

        route = greedyTSP(startCity);
    }

    cout << "\n=== " << name << " ===\n";

    cout << "Route: ";

    printRoute(route);
}

void compareAlgorithms(int startCity) {

    cout << "\n========== Algorithm Comparison ==========\n";

    cout << "Algorithm              Cost    Representation\n";

    cout << "------------------------------------------\n";

    vector<int> route1 = bruteForceTSP(startCity);

    if (route1.empty())
        cout << "Brute Force  ->  No valid route  |  Matrix\n";
    else
        cout << "Brute Force  ->  Cost: "
             << routeCost(route1)
             << "  |  Matrix\n";

    vector<int> route2 = greedyTSP(startCity);

    cout << "Greedy       ->  Cost: "
         << routeCost(route2)
         << "  |  List\n";

    cout << "==========================================\n";
}

bool handleEdgeCases() {

    if (numCities == 0) {
        cout << "Edge Case: No cities found.\n";
        return true;
    }

    if (numCities == 1) {
        cout << "Edge Case: Only 1 city. Cost = 0.\n";
        return true;
    }

    for (int i = 0; i < numCities; i++)
        for (int j = 0; j < numCities; j++)
            if (i != j && adjMatrix[i][j] == 0)
                cout << "Warning: Missing edge ("
                     << cityNames[i]
                     << " -> "
                     << cityNames[j]
                     << ")\n";

    return false;
}

int main() {

    cout << "============================================\n";
    cout << "   Travelling Salesperson Problem (TSP)\n";
    cout << "============================================\n";

    string filename;

    cout << "Enter graph file name: ";
    cin >> filename;

    if (!loadGraph(filename)) {

        cout << "Using built-in example...\n";

        numCities = 4;

        cityNames[0] = "A";
        cityNames[1] = "B";
        cityNames[2] = "C";
        cityNames[3] = "D";

        int m[4][4] = {
            {0, 10, 15, 20},
            {10, 0, 35, 25},
            {15, 35, 0, 30},
            {20, 25, 30, 0}
        };

        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                adjMatrix[i][j] = m[i][j];

        for (int i = 0; i < 4; i++) {

            adjList[i].clear();

            for (int j = 0; j < 4; j++)
                if (i != j)
                    adjList[i].push_back({ j, adjMatrix[i][j] });
        }
    }

    printMatrix();

    printList();

    if (handleEdgeCases())
        return 0;

    int startCity = 0;

    cout << "\nEnter starting city index: ";
    cin >> startCity;

    if (startCity < 0 || startCity >= numCities)
        startCity = 0;

    int choice;

    do {

        cout << "\n--- Menu ---\n";

        cout << "1. Brute Force TSP\n";
        cout << "2. Greedy TSP\n";
        cout << "3. Compare Algorithms\n";
        cout << "4. Dijkstra Shortest Path\n";
        cout << "0. Exit\n";

        cout << "Choice: ";
        cin >> choice;

        if (choice == 1 || choice == 2)
            runAlgorithm(choice, startCity);

        else if (choice == 3)
            compareAlgorithms(startCity);

        else if (choice == 4)
            dijkstraShortestPath();

    } while (choice != 0);

    cout << "Goodbye!\n";

    return 0;
}
