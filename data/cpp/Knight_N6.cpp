/*El "punto de quiebre". Aquí es donde algoritmos como BFS suelen quedarse sin memoria o tiempo, pero A* debería seguir funcionando bien. */

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
#include <array>
#include <random>

using namespace std;
using namespace std::chrono;

// --- Definiciones y Funciones Auxiliares ---
const array<int, 8> movX = {2, 1, -1, -2, -2, -1, 1, 2};
const array<int, 8> movY = {1, 2, 2, 1, -1, -2, -2, -1};

struct Result
{
    string name;
    double duration_ms;
    bool found;
    long long metric_value = 0;
};

bool is_valid_position(int x, int y, int N, const vector<vector<int>> &board)
{
    return (x >= 0 && x < N && y >= 0 && y < N && board[x][y] == 0);
}

int count_possible_moves(int x, int y, int N, const vector<vector<int>> &board)
{
    int count = 0;
    for (int i = 0; i < 8; ++i)
    {
        int nx = x + movX[i];
        int ny = y + movY[i];
        if (is_valid_position(nx, ny, N, board))
            count++;
    }
    return count;
}

// --- Algoritmos ---

// Backtracking
long long bt_nodes_visited = 0;
bool run_bt_dfs_util(int x, int y, int move_count, int N, vector<vector<int>> &board)
{
    bt_nodes_visited++;
    if (move_count == N * N)
        return true;
    for (int k = 0; k < 8; ++k)
    {
        int nx = x + movX[k];
        int ny = y + movY[k];
        if (is_valid_position(nx, ny, N, board))
        {
            board[nx][ny] = move_count + 1;
            if (run_bt_dfs_util(nx, ny, move_count + 1, N, board))
                return true;
            board[nx][ny] = 0;
        }
    }
    return false;
}

Result run_backtracking_knight(int N, int startX, int startY)
{
    auto start = high_resolution_clock::now();
    vector<vector<int>> board(N, vector<int>(N, 0));
    board[startX][startY] = 1;
    bt_nodes_visited = 0;
    bool found = run_bt_dfs_util(startX, startY, 1, N, board);
    auto end = high_resolution_clock::now();
    return {"Backtracking / DFS", duration_cast<microseconds>(end - start).count() / 1000.0, found, bt_nodes_visited};
}

// A* (A-Star)
struct AStarNode
{
    int x, y, g, h;
    vector<vector<int>> board;
    bool operator>(const AStarNode &other) const
    {
        if (h != other.h)
            return h > other.h;
        return g < other.g;
    }
};

Result run_a_star_knight(int N, int startX, int startY)
{
    auto start_time = high_resolution_clock::now();
    bool found = false;
    // N=6 empieza a ser pesado, pero lo permitimos. Cortamos en >8.
    if (N > 8)
        return {"A* (Inviable para N > 8)", 0.0, false, 0};

    priority_queue<AStarNode, vector<AStarNode>, greater<AStarNode>> pq;
    vector<vector<int>> initial_board(N, vector<int>(N, 0));
    initial_board[startX][startY] = 1;
    int h_ini = count_possible_moves(startX, startY, N, initial_board);
    AStarNode start_node = {startX, startY, 1, h_ini, initial_board};
    pq.push(start_node);

    long long nodes_visited = 0;
    while (!pq.empty())
    {
        AStarNode current = pq.top();
        pq.pop();
        nodes_visited++;
        if (current.g == N * N)
        {
            found = true;
            break;
        }
        int g_successor = current.g + 1;
        for (int i = 0; i < 8; ++i)
        {
            int nx = current.x + movX[i];
            int ny = current.y + movY[i];
            if (is_valid_position(nx, ny, N, current.board))
            {
                vector<vector<int>> new_board = current.board;
                new_board[nx][ny] = g_successor;
                int h_successor = count_possible_moves(nx, ny, N, new_board);
                AStarNode successor_node = {nx, ny, g_successor, h_successor, new_board};
                pq.push(successor_node);
            }
        }
        if (nodes_visited > 2000000)
            break; // Límite aumentado para N=6
    }
    auto end_time = high_resolution_clock::now();
    return {"A* (Warnsdorff)", duration_cast<microseconds>(end_time - start_time).count() / 1000.0, found, nodes_visited};
}

// Hill Climbing
Result run_hill_climbing_knight(int N, int startX, int startY)
{
    auto start_time = high_resolution_clock::now();
    bool found = false;
    vector<vector<int>> board(N, vector<int>(N, 0));
    int currentX = startX, currentY = startY;
    board[currentX][currentY] = 1;
    int moves = 1;

    while (moves < N * N)
    {
        int best_move_index = -1;
        int min_onward_moves = 9;
        for (int i = 0; i < 8; ++i)
        {
            int nx = currentX + movX[i];
            int ny = currentY + movY[i];
            if (is_valid_position(nx, ny, N, board))
            {
                board[nx][ny] = -1;
                int onward_moves = count_possible_moves(nx, ny, N, board);
                board[nx][ny] = 0;
                if (onward_moves < min_onward_moves)
                {
                    min_onward_moves = onward_moves;
                    best_move_index = i;
                }
            }
        }
        if (best_move_index != -1)
        {
            currentX += movX[best_move_index];
            currentY += movY[best_move_index];
            moves++;
            board[currentX][currentY] = moves;
        }
        else
        {
            break;
        }
    }
    if (moves == N * N)
        found = true;
    auto end_time = high_resolution_clock::now();
    return {"Hill Climbing", duration_cast<microseconds>(end_time - start_time).count() / 1000.0, found, (long long)moves};
}

// BFS
struct BFSState
{
    int x, y;
    vector<vector<bool>> visited;
    bool operator<(const BFSState &other) const
    {
        if (x != other.x)
            return x < other.x;
        if (y != other.y)
            return y < other.y;
        return visited < other.visited;
    }
};

Result run_bfs_knight(int N, int startX, int startY)
{
    auto start_time = high_resolution_clock::now();
    bool found = false;
    long long nodes_visited = 0;
    // N=6 es el límite absoluto para BFS
    if (N > 6)
        return {"BFS (Inviable para N > 6)", 0.0, false, 0};

    BFSState start_state;
    start_state.x = startX;
    start_state.y = startY;
    start_state.visited = vector<vector<bool>>(N, vector<bool>(N, false));
    start_state.visited[startX][startY] = true;

    queue<BFSState> q;
    set<BFSState> visited_set;
    q.push(start_state);
    visited_set.insert(start_state);

    while (!q.empty())
    {
        BFSState current = q.front();
        q.pop();
        nodes_visited++;
        bool all_visited = true;
        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                if (!current.visited[i][j])
                {
                    all_visited = false;
                    break;
                }
            }
        }
        if (all_visited)
        {
            found = true;
            break;
        }

        for (int i = 0; i < 8; i++)
        {
            int nx = current.x + movX[i];
            int ny = current.y + movY[i];
            if (nx >= 0 && nx < N && ny >= 0 && ny < N && !current.visited[nx][ny])
            {
                BFSState ns = current;
                ns.x = nx;
                ns.y = ny;
                ns.visited[nx][ny] = true;
                if (visited_set.find(ns) == visited_set.end())
                {
                    visited_set.insert(ns);
                    q.push(ns);
                }
            }
        }
        if (nodes_visited > 100000)
            break; // Límite estricto para que no colapse la RAM
    }
    auto end_time = high_resolution_clock::now();
    return {"BFS (Espacio de Estados)", duration_cast<microseconds>(end_time - start_time).count() / 1000.0, found, nodes_visited};
}

// --- MAIN FIJO N=6 ---
int main()
{
    const int N = 6; // Punto intermedio
    const int startX = 0;
    const int startY = 0;

    cout << "--- Optimizador de Algoritmos (Recorrido del Caballo) ---" << endl;
    cout << "CONFIGURACION: Tablero " << N << "x" << N << " | Inicio (" << startX << "," << startY << ")" << endl;

    vector<Result> results;

    // Probamos todos. BFS probablemente sufra aquí.
    results.push_back(run_backtracking_knight(N, startX, startY));
    results.push_back(run_bfs_knight(N, startX, startY));
    results.push_back(run_a_star_knight(N, startX, startY));
    results.push_back(run_hill_climbing_knight(N, startX, startY));

    Result optimal_result = {"", numeric_limits<double>::max(), false};

    cout << fixed << setprecision(4) << "\n--- Resultados (N=" << N << ") ---" << endl;
    cout << "--------------------------------------------------------------------------------" << endl;
    cout << "| Algoritmo                        | Tiempo (ms) | Encontrada | Métrica (Nodos)       |" << endl;
    cout << "--------------------------------------------------------------------------------" << endl;

    for (const auto &res : results)
    {
        cout << "| " << setw(30) << left << res.name << " | "
             << setw(11) << right << res.duration_ms << " | "
             << setw(10) << (res.found ? "Sí" : "No") << " | "
             << setw(20) << res.metric_value << " |" << endl;
        if (res.found && res.duration_ms < optimal_result.duration_ms)
            optimal_result = res;
    }
    cout << "--------------------------------------------------------------------------------" << endl;

    if (optimal_result.found)
    {
        cout << "\n🏆 GANADOR PARA N=" << N << ":" << endl;
        cout << "   > Algoritmo: " << optimal_result.name << endl;
        cout << "   > Tiempo: " << optimal_result.duration_ms << " ms" << endl;
    }
    else
    {
        cout << "\nNo se encontró solución." << endl;
    }
    return 0;
}