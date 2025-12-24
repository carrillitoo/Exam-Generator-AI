#include <iostream>
#include <vector>
#include <queue>
#include <set>
#include <map>
#include <memory>
#include <cmath>
#include <chrono>
#include <string>
#include <sstream>
#include <algorithm>
#include <limits>
#include <iomanip>
#include <cstdlib>
#include <ctime>

using namespace std;
using namespace std::chrono;

using Graph = vector<vector<int>>;
using ColorAssignment = vector<int>;

struct State {
    ColorAssignment colors;
    bool operator<(const State &other) const { return colors < other.colors; }
};

struct Result {
    string name;
    double duration_ms;
    bool found;
    long long metric_value = 0;
};

bool is_safe(int v, int color, const Graph& graph, const ColorAssignment& colorDeV) {
    int V = graph.size();
    for (int i = 0; i < V; ++i)
        if (graph[v][i] && colorDeV[i] == color) return false;
    return true;
}

bool is_goal_valid(const ColorAssignment& colors, int V, const Graph& graph) {
    for (int c : colors) if (c == 0) return false;
    for (int i = 0; i < V; ++i) {
        for (int j = i + 1; j < V; ++j) {
            if (graph[i][j] && colors[i] == colors[j]) return false;
        }
    }
    return true;
}

long long bt_nodes_visited = 0;
bool run_bt_dfs_util(int v, const Graph& graph, ColorAssignment& colorDeV, int M, bool find_first_only) {
    int V = graph.size();
    bt_nodes_visited++;
    if (v == V) return true;
    for (int c = 1; c <= M; ++c) {
        if (is_safe(v, c, graph, colorDeV)) {
            colorDeV[v] = c;
            if (run_bt_dfs_util(v + 1, graph, colorDeV, M, find_first_only)) return true;
            colorDeV[v] = 0;
        }
    }
    return false;
}

Result run_backtracking(int V, int M, const Graph& graph) {
    auto start = high_resolution_clock::now();
    ColorAssignment colors(V, 0);
    bt_nodes_visited = 0;
    bool found = run_bt_dfs_util(0, graph, colors, M, true);
    auto end = high_resolution_clock::now();
    return {"Backtracking (Primera Solución - CSP)", duration_cast<microseconds>(end - start).count() / 1000.0, found, bt_nodes_visited};
}

Result run_bfs_coloring(int V, int M, const Graph& graph) {
    auto start_time = high_resolution_clock::now();
    bool found = false;
    State start_state; start_state.colors = vector<int>(V, 0);
    queue<State> q; set<State> visited;
    q.push(start_state); visited.insert(start_state);
    
    auto get_neighbors = [&](const State &s) -> vector<State> {
        vector<State> next;
        int node = -1;
        for (int i = 0; i < V; i++) { if (s.colors[i] == 0) { node = i; break; } }
        if (node == -1) return next;
        for (int color = 1; color <= M; color++) {
            State ns = s; ns.colors[node] = color;
            if (is_safe(node, color, graph, s.colors)) next.push_back(ns);
        }
        return next;
    };

    while (!q.empty()) {
        State current = q.front(); q.pop();
        if (current.colors.size() == (size_t)V && is_goal_valid(current.colors, V, graph)) { found = true; break; }
        for (auto &next : get_neighbors(current)) {
            if (!visited.count(next)) { visited.insert(next); q.push(next); }
        }
        if (visited.size() > 50000) break;
    }
    auto end_time = high_resolution_clock::now();
    return {"BFS (Espacio de Estados)", duration_cast<microseconds>(end_time - start_time).count() / 1000.0, found, (long long)visited.size()};
}

struct AStarNode {
    ColorAssignment colors; int g, h, f, colored_count;
    bool operator>(const AStarNode& other) const { if (f != other.f) return f > other.f; return colored_count < other.colored_count; }
};

Result run_a_star_coloring(int V, int M, const Graph& graph) {
    auto start_time = high_resolution_clock::now();
    bool found = false;
    ColorAssignment initial_colors(V, 0);
    priority_queue<AStarNode, vector<AStarNode>, greater<AStarNode>> pq;
    map<ColorAssignment, int> closed_set;
    pq.push({initial_colors, 0, 0, 0, 0});
    closed_set[initial_colors] = 0;
    long long nodes_visited = 0;

    while (!pq.empty()) {
        AStarNode current = pq.top(); pq.pop();
        nodes_visited++;
        if (current.colored_count == V) { found = true; break; }
        int node_to_color = -1;
        for (int i = 0; i < V; ++i) { if (current.colors[i] == 0) { node_to_color = i; break; } }
        if (node_to_color == -1) continue;
        int g_current = current.g;
        int max_color_to_try = (g_current + 1 > M) ? M : g_current + 1;

        for (int color_to_try = 1; color_to_try <= max_color_to_try; ++color_to_try) {
            if (is_safe(node_to_color, color_to_try, graph, current.colors)) {
                ColorAssignment next_colors = current.colors;
                next_colors[node_to_color] = color_to_try;
                int g_successor = (color_to_try == g_current + 1) ? g_current + 1 : g_current;
                if (closed_set.find(next_colors) == closed_set.end() || g_successor < closed_set[next_colors]) {
                    closed_set[next_colors] = g_successor;
                    pq.push({next_colors, g_successor, 0, g_successor, current.colored_count + 1});
                }
            }
        }
        if (closed_set.size() > 50000) break;
    }
    auto end_time = high_resolution_clock::now();
    return {"A* (Costo Uniforme)", duration_cast<microseconds>(end_time - start_time).count() / 1000.0, found, nodes_visited};
}

int count_conflicts_hc(const ColorAssignment& colors, const Graph& graph) {
    int conflicts = 0; int V = colors.size();
    for (int i = 0; i < V; ++i) {
        if (colors[i] == 0) continue; 
        for (int j = i + 1; j < V; ++j) { if (graph[i][j] && colors[i] == colors[j]) conflicts++; }
    }
    return conflicts;
}

Result run_hill_climbing_coloring(int V, int M, const Graph& graph) {
    auto start_time = high_resolution_clock::now();
    bool found = false;
    ColorAssignment current_colors(V);
    srand(time(0));
    for (int i = 0; i < V; ++i) current_colors[i] = (rand() % M) + 1;
    int current_conflicts = count_conflicts_hc(current_colors, graph);
    int iterations = 0;

    while (current_conflicts > 0 && iterations < 10000) {
        iterations++;
        int best_conflicts = current_conflicts;
        ColorAssignment best_colors = current_colors;
        bool improvement = false;
        for (int v = 0; v < V; ++v) {
            int original_color = current_colors[v];
            for (int new_color = 1; new_color <= M; ++new_color) {
                if (new_color != original_color) {
                    current_colors[v] = new_color;
                    int new_conflicts = count_conflicts_hc(current_colors, graph);
                    if (new_conflicts < best_conflicts) {
                        best_conflicts = new_conflicts; best_colors = current_colors; improvement = true;
                    }
                }
            }
            current_colors[v] = original_color;
        }
        if (improvement) { current_colors = best_colors; current_conflicts = best_conflicts; }
        else break;
    }
    if (current_conflicts == 0) found = true;
    auto end_time = high_resolution_clock::now();
    return {"Hill Climbing (Búsqueda Local)", duration_cast<microseconds>(end_time - start_time).count() / 1000.0, found, (long long)iterations};
}

int main() {
    const int V = 8; const int M = 3; 
    const Graph graph = {
        {0, 1, 0, 0, 0, 0, 0, 1}, {1, 0, 1, 0, 0, 0, 0, 0}, {0, 1, 0, 1, 0, 0, 0, 0},
        {0, 0, 1, 0, 1, 0, 0, 0}, {0, 0, 0, 1, 0, 1, 0, 0}, {0, 0, 0, 0, 1, 0, 1, 0},
        {0, 0, 0, 0, 0, 1, 0, 1}, {1, 0, 0, 0, 0, 0, 1, 0}
    };
    
    vector<Result> results;
    results.push_back(run_backtracking(V, M, graph)); 
    results.push_back(run_bfs_coloring(V, M, graph));
    results.push_back(run_a_star_coloring(V, M, graph)); 
    results.push_back(run_hill_climbing_coloring(V, M, graph));

    Result optimal_result = {"", numeric_limits<double>::max(), false};
    for (const auto& res : results) { if (res.found && res.duration_ms < optimal_result.duration_ms) optimal_result = res; }

    cout << "\n🏆 ALGORITMO MÁS RÁPIDO PARA COLOREADO DE GRAFOS (V=" << V << ", M=" << M << "):" << endl;
    if (optimal_result.duration_ms != numeric_limits<double>::max()) {
        cout << "   > Algoritmo: " << optimal_result.name << endl;
    } else {
        cout << "   > Algoritmo: Ninguno (Fallo)" << endl;
    }
    return 0;
}