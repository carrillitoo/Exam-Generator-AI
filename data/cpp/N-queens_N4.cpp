#include <iostream>
#include <vector>
#include <queue>
#include <set>
#include <cmath>
#include <chrono> 
#include <string>
#include <sstream>
#include <algorithm>
#include <limits>
#include <map>
#include <iomanip>
#include <random> // Soluciona el error: Añadido para default_random_engine y shuffle

using namespace std;
using namespace std::chrono;

// --- 1. Definiciones Comunes y Estructuras ---

// Estructura para almacenar el estado del tablero (reina en cada fila)
struct State {
    vector<int> queens; 

    bool operator<(const State &other) const {
        return queens < other.queens; 
    }
};

// Estructura para almacenar el resultado de cada algoritmo
struct Result {
    string name;
    double duration_ms;
    bool found;
    int solutions_count = 0;
};

/**
 * @brief Verifica si colocar una reina en (row, col) es seguro respecto a las reinas anteriores.
 */
bool is_safe(const vector<int> &queens, int row, int col) {
    for (int i = 0; i < row; i++) {
        int qcol = queens[i];
        if (qcol == col || abs(qcol - col) == abs(i - row))
            return false;
    }
    return true;
}

/**
 * @brief Genera los siguientes estados válidos (vecinos) para el problema N-Queens.
 */
vector<State> get_neighbors(const State &s, int N) {
    vector<State> next;
    int row = s.queens.size();
    
    if (row == N) return next; 

    for (int col = 0; col < N; col++) {
        if (is_safe(s.queens, row, col)) {
            State ns = s;
            ns.queens.push_back(col);
            next.push_back(ns);
        }
    }
    return next;
}

/**
 * @brief Determina si se ha alcanzado el estado objetivo (tablero completo).
 */
bool is_goal(const State &s, int N) {
    return (int)s.queens.size() == N;
}

// --- 2. Implementaciones de Algoritmos de Búsqueda ---

// a) Backtracking (BT) - Encontrando todas las soluciones
int solve_backtracking_util(vector<int>& col_of_row, int row, int N, int& count) {
    if (row == N) {
        count++;
        return 0; // Continúa buscando más soluciones
    }

    for (int col = 0; col < N; ++col) {
        if (is_safe(col_of_row, row, col)) {
            col_of_row.push_back(col);
            solve_backtracking_util(col_of_row, row + 1, N, count);
            col_of_row.pop_back(); // BACKTRACK
        }
    }
    return 0;
}

Result run_backtracking(int N) {
    auto start = high_resolution_clock::now();
    int count = 0;
    vector<int> col_of_row;
    solve_backtracking_util(col_of_row, 0, N, count);
    auto end = high_resolution_clock::now();
    
    return {"Backtracking (Todas las Soluciones)", 
            duration_cast<microseconds>(end - start).count() / 1000.0, 
            count > 0, 
            count};
}


// b) DFS - Encontrando la primera solución
bool solve_dfs_util(vector<int>& col_of_row, int row, int N) {
    if (row == N) return true; // ¡Solución encontrada!

    for (int col = 0; col < N; ++col) {
        if (is_safe(col_of_row, row, col)) {
            col_of_row.push_back(col);
            if (solve_dfs_util(col_of_row, row + 1, N))
                return true; // Propagar éxito
            col_of_row.pop_back(); // BACKTRACK
        }
    }
    return false;
}

Result run_dfs(int N) {
    auto start = high_resolution_clock::now();
    vector<int> solution;
    bool found = solve_dfs_util(solution, 0, N);
    auto end = high_resolution_clock::now();
    
    return {"DFS (Primera Solución)", 
            duration_cast<microseconds>(end - start).count() / 1000.0, 
            found};
}

// c) BFS - Encontrando la primera solución
Result run_bfs(int N) {
    auto start = high_resolution_clock::now();
    queue<State> q;
    set<State> visited;
    State start_state;
    q.push(start_state);
    visited.insert(start_state);
    bool found = false;

    while (!q.empty()) {
        State current = q.front(); q.pop();
        
        if (is_goal(current, N)) {
            found = true;
            break;
        }
        
        for (auto &next : get_neighbors(current, N)) {
            if (visited.find(next) == visited.end()) {
                visited.insert(next);
                q.push(next);
            }
        }
        // Límite de seguridad
        if (visited.size() > 500000) break; 
    }

    auto end = high_resolution_clock::now();
    return {"BFS (Primera Solución)", 
            duration_cast<microseconds>(end - start).count() / 1000.0, 
            found};
}

// d) A* (A-Star) - Encontrando la primera solución
struct AStarState {
    State state;
    int cost; // g(n) = profundidad
    int heuristic; // h(n) = 0
    bool operator>(const AStarState& other) const {
        return (cost + heuristic) > (other.cost + other.heuristic);
    }
};

Result run_a_star(int N) {
    auto start = high_resolution_clock::now();
    priority_queue<AStarState, vector<AStarState>, greater<AStarState>> pq;
    set<vector<int>> visited_queens; 
    AStarState start_state = {{}, 0, 0};
    pq.push(start_state);
    visited_queens.insert(start_state.state.queens);
    bool found = false;

    while (!pq.empty()) {
        AStarState current = pq.top(); pq.pop();

        if (is_goal(current.state, N)) {
            found = true;
            break;
        }

        for (auto &next_state : get_neighbors(current.state, N)) {
            if (visited_queens.find(next_state.queens) == visited_queens.end()) {
                AStarState next_astar = {next_state, current.cost + 1, 0};
                visited_queens.insert(next_state.queens);
                pq.push(next_astar);
            }
        }
        // Límite de seguridad
        if (visited_queens.size() > 500000) break;
    }

    auto end = high_resolution_clock::now();
    return {"A* (Heurística h=0)", 
            duration_cast<microseconds>(end - start).count() / 1000.0, 
            found};
}

// e) Hill Climbing (HC) - Encontrando una solución
int count_conflicts(const vector<int>& col_of_row) {
    int conflicts = 0;
    int N = col_of_row.size();
    for (int i = 0; i < N; ++i) {
        for (int j = i + 1; j < N; ++j) {
            if (col_of_row[i] == col_of_row[j] || 
                abs(col_of_row[i] - col_of_row[j]) == abs(i - j)) {
                conflicts++;
            }
        }
    }
    return conflicts;
}

Result run_hill_climbing(int N) {
    auto start = high_resolution_clock::now();
    vector<int> current_queens(N);
    bool found = false;

    // Inicialización aleatoria (la clave de HC)
    for (int i = 0; i < N; ++i) {
        current_queens[i] = i; 
    }
    
    // Generador de números aleatorios para std::shuffle
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    default_random_engine rng(seed); 
    shuffle(current_queens.begin(), current_queens.end(), rng);


    int current_conflicts = count_conflicts(current_queens);
    int iterations = 0;
    const int MAX_ITERATIONS = 10000; 

    while (current_conflicts > 0 && iterations < MAX_ITERATIONS) {
        iterations++;
        int best_conflicts = current_conflicts;
        vector<int> best_queens = current_queens;
        bool improvement = false;

        for (int i = 0; i < N; ++i) { // Fila a mover
            int original_col = current_queens[i];
            for (int j = 0; j < N; ++j) { // Nueva columna
                if (j != original_col) {
                    current_queens[i] = j;
                    int new_conflicts = count_conflicts(current_queens);
                    
                    if (new_conflicts < best_conflicts) {
                        best_conflicts = new_conflicts;
                        best_queens = current_queens;
                        improvement = true;
                    }
                }
            }
            current_queens[i] = original_col; // Restaurar
        }

        if (improvement) {
            current_queens = best_queens;
            current_conflicts = best_conflicts;
        } else {
            // Máximo local: se detiene.
            break;
        }
    }
    
    if (current_conflicts == 0) {
        found = true;
    }

    auto end = high_resolution_clock::now();
    return {"Hill Climbing (Búsqueda Local)", 
            duration_cast<microseconds>(end - start).count() / 1000.0, 
            found};
}


// --- 3. Bloque Principal de Ejecución y Selector de Óptimo ---

int main() {
    // 1. N se fija, eliminando toda entrada de lectura.
    const int N = 4;
    
    // Mensajes informativos (eliminado cin.ignore y cout interactivo)
    cout << "--- Optimizador de Algoritmos de Búsqueda (N-Queens) ---" << endl;
    cout << "Tablero fijo: N=" << N << endl;

    // Lista de resultados para la comparación
    vector<Result> results;
    
    // 2. Ejecución y medición de todos los algoritmos
    results.push_back(run_backtracking(N)); 
    results.push_back(run_dfs(N));
    results.push_back(run_bfs(N));
    results.push_back(run_a_star(N)); 
    results.push_back(run_hill_climbing(N));

    // --- 3. Análisis y Selector de Óptimo ---

    Result optimal_result = {"", numeric_limits<double>::max(), false};

    cout << fixed << setprecision(4) << "\n--- Resultados de la Comparación (N=" << N << ") ---" << endl;
    cout << "-------------------------------------------------------------------" << endl;
    cout << "| Algoritmo             | Tiempo (ms) | Encontrada | Soluciones |" << endl;
    cout << "-------------------------------------------------------------------" << endl;

    for (const auto& res : results) {
        // Imprimir resultados
        cout << "| " << setw(20) << left << res.name << " | " 
             << setw(11) << right << res.duration_ms << " | "
             << setw(10) << (res.found ? "Sí" : "No") << " | "
             << setw(10) << res.solutions_count << " |" << endl;

        // Lógica de selección del más rápido
        if (res.duration_ms < optimal_result.duration_ms) {
            optimal_result = res;
        }
    }
    cout << "-------------------------------------------------------------------" << endl;

    // Mostrar el algoritmo óptimo
    if (optimal_result.duration_ms != numeric_limits<double>::max()) {
        cout << "\n🏆 ALGORITMO MÁS RÁPIDO PARA N=" << N << ":" << endl;
        cout << "   > Algoritmo: " << optimal_result.name << endl;
        cout << "   > Tiempo: " << optimal_result.duration_ms << " ms" << endl;
        cout << "\n*Observación: El Backtracking es teóricamente el mejor para CSPs, mientras que Hill Climbing es muy rápido (pero solo encuentra UNA solución y puede fallar)." << endl;
    } else {
        cout << "\nNo se pudieron obtener resultados válidos." << endl;
    }

    return 0;
}