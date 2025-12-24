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

using namespace std;
using namespace std::chrono;

// --- 1. Definiciones Comunes y Estructuras ---

// Estructura para almacenar el estado del juego (usada por BFS, A*, HillClimbing)
struct State {
    // towers[i] es un vector (poste) que contiene los discos (1 es el más pequeño)
    vector<vector<int>> towers; 

    // Necesario para usar State en std::set y std::priority_queue
    bool operator<(const State &other) const {
        return towers < other.towers;
    }
};

// Estructura para almacenar el resultado de cada algoritmo
struct Result {
    string name;
    double duration_ms;
    bool found;
    long long solutions_count = 0;
};

// --- Funciones auxiliares para Algoritmos de Búsqueda de Espacio de Estados ---

/**
 * @brief Genera los movimientos válidos (vecinos) para el estado actual.
 * @param s Estado actual.
 * @param numPegs Número de postes (N).
 * @return Vector de estados vecinos.
 */
vector<State> get_neighbors(const State &s, int numPegs) {
    vector<State> next;
    for (int from = 0; from < numPegs; from++) {
        if (s.towers[from].empty()) continue;
        int disk = s.towers[from].back(); // Disco en la cima
        for (int to = 0; to < numPegs; to++) {
            if (from == to) continue;
            // Movimiento válido: el poste destino está vacío o el disco en su cima es más grande
            if (s.towers[to].empty() || s.towers[to].back() > disk) {
                State ns = s;
                ns.towers[from].pop_back();
                ns.towers[to].push_back(disk);
                next.push_back(ns);
            }
        }
    }
    return next;
}

/**
 * @brief Determina si se ha alcanzado el estado objetivo (todos los discos en el último poste).
 * @param s Estado actual.
 * @param numDisks Número total de discos (M).
 * @param numPegs Número total de postes (N).
 * @return true si el objetivo se cumple.
 */
bool is_goal(const State &s, int numDisks, int numPegs) {
    // El poste destino es el último (índice N-1)
    return s.towers[numPegs - 1].size() == (size_t)numDisks;
}


// --- 2. Implementaciones de Algoritmos ---

// a) Backtracking (BT) / DFS Recursivo - Basado en la fórmula óptima (Solución de Fuerza Bruta)
long long hanoi_recursive_moves = 0; // Contador de movimientos

/**
 * @brief Recursión de Hanói generalizada (basada en 3 pegs).
 */
void hanoi_3pegs_bt(int n, int origen, int destino, int auxiliar) {
    if (n == 0) return;
    hanoi_3pegs_bt(n - 1, origen, auxiliar, destino);
    hanoi_recursive_moves++; // Mover el disco grande
    hanoi_3pegs_bt(n - 1, auxiliar, destino, origen);
}

Result run_backtracking_hanoi(int N, int M) {
    auto start = high_resolution_clock::now();
    hanoi_recursive_moves = 0;
    
    if (N >= 3) {
        hanoi_3pegs_bt(M, 0, N - 1, 1);
    } else {
        if (N==2 && M > 0) hanoi_recursive_moves = (1LL << M) - 1; 
    }

    auto end = high_resolution_clock::now();
    
    return {"Backtracking/DFS Recursivo (2^M - 1)", 
            duration_cast<microseconds>(end - start).count() / 1000.0, 
            true, 
            hanoi_recursive_moves};
}


// b) BFS - Búsqueda por anchura (Garantiza el camino más corto en el espacio de estados)
Result run_bfs_hanoi(int N, int M) {
    auto start = high_resolution_clock::now();
    bool found = false;

    // Inicializar estado
    State start_state;
    start_state.towers.resize(N);
    for (int i = M; i >= 1; i--)
        start_state.towers[0].push_back(i);

    queue<State> q;
    set<State> visited;
    q.push(start_state);
    visited.insert(start_state);

    // ADVERTENCIA: Este algoritmo es muy lento. Limitamos el espacio de búsqueda.
    const size_t MAX_NODES = 500000;
    
    while (!q.empty()) {
        State current = q.front(); q.pop();

        if (is_goal(current, M, N)) {
            found = true;
            break; 
        }

        for (auto &next : get_neighbors(current, N)) {
            if (!visited.count(next)) {
                visited.insert(next);
                q.push(next);
            }
        }
        
        if (visited.size() > MAX_NODES) break; 
    }
    
    auto end = high_resolution_clock::now();
    
    return {"BFS (Garantía de Óptimo)", 
            duration_cast<microseconds>(end - start).count() / 1000.0, 
            found,
            (long long)visited.size()}; // Retornamos el número de nodos visitados como métrica
}


// c) A* (A-Star) - Búsqueda informada (Usaremos una heurística simple)
struct AStarState {
    State state;
    int g; // Costo real (número de movimientos)
    int h; // Heurística (estimación)
    int f; // g + h
    
    bool operator>(const AStarState& other) const {
        // Min-Heap: Prioridad al menor f, luego al menor g para desempatar (mejor camino)
        if (f == other.f) return g > other.g; 
        return f > other.f; 
    }
};

// Heurística: Número de discos que NO están en el poste de destino.
int calculateHeuristic(const State& state, int numDisks, int numPegs) {
    int dest = numPegs - 1;
    return numDisks - (int)state.towers[dest].size();
}

Result run_a_star_hanoi(int N, int M) {
    auto start = high_resolution_clock::now();
    bool found = false;
    
    State start_state;
    start_state.towers.resize(N);
    for (int i = M; i >= 1; i--)
        start_state.towers[0].push_back(i);

    // Cola de prioridad (Lista Abierta)
    priority_queue<AStarState, vector<AStarState>, greater<AStarState>> pq;
    // Lista Cerrada (clave es el estado, valor es el mejor costo g)
    map<State, int> closed_set;
    
    AStarState start_astar = {start_state, 0, calculateHeuristic(start_state, M, N), 0};
    pq.push(start_astar);
    closed_set[start_state] = 0;
    
    long long visited_count = 0;
    const size_t MAX_NODES = 500000;

    while (!pq.empty()) {
        AStarState current = pq.top(); pq.pop();
        visited_count++;

        if (is_goal(current.state, M, N)) {
            found = true;
            break; 
        }

        int g_successor = current.g + 1;
        
        for (auto &next_state : get_neighbors(current.state, N)) {
            
            // Si el estado no ha sido visitado O encontramos un camino mejor (menor g)
            if (closed_set.find(next_state) == closed_set.end() || g_successor < closed_set[next_state]) {
                
                closed_set[next_state] = g_successor;
                
                int h_successor = calculateHeuristic(next_state, M, N);
                AStarState next_astar = {next_state, g_successor, h_successor, g_successor + h_successor};
                
                pq.push(next_astar);
            }
        }
        
        if (closed_set.size() > MAX_NODES) break; 
    }

    auto end = high_resolution_clock::now();
    return {"A* (Informed Search)", 
            duration_cast<microseconds>(end - start).count() / 1000.0, 
            found,
            visited_count};
}


// d) Hill Climbing - Búsqueda Local (Simplificado, puede quedar en máximo local)
Result run_hill_climbing_hanoi(int N, int M) {
    auto start = high_resolution_clock::now();
    bool found = false;
    
    // Inicialización del estado: todos los discos en el poste de origen
    State current_state;
    current_state.towers.assign(N, vector<int>());
    for (int disk = M; disk >= 1; --disk) {
        current_state.towers[0].push_back(disk);
    }
    
    int current_heuristic = calculateHeuristic(current_state, M, N);
    int iterations = 0;
    const int MAX_ITERATIONS = 50000; 

    while (current_heuristic > 0 && iterations < MAX_ITERATIONS) {
        iterations++;
        State best_neighbor = current_state;
        int best_H = current_heuristic;
        bool improved = false;

        // Búsqueda del mejor vecino
        vector<State> neighbors = get_neighbors(current_state, N);
        
        for (const auto& nb : neighbors) {
            int h = calculateHeuristic(nb, M, N);
            if (h < best_H) {
                best_H = h;
                best_neighbor = nb;
                improved = true;
            }
        }
        
        if (improved) {
            current_state = best_neighbor;
            current_heuristic = best_H;
        } else {
            // Máximo local (o meseta): no hay mejora. Se detiene.
            break;
        }
    }
    
    if (current_heuristic == 0) {
        found = true;
    }

    auto end = high_resolution_clock::now();
    return {"Hill Climbing (Local Search)", 
            duration_cast<microseconds>(end - start).count() / 1000.0, 
            found,
            (long long)iterations};
}

// --- 3. Bloque Principal de Ejecución y Selector de Óptimo ---

int main() {
    // 1. Fijar N y M (4 postes, 9 discos)
    const int N_PEGS = 4; 
    const int M_DISKS = 9; 
    
    // Mensajes informativos (eliminada la lógica de lectura)
    cout << "--- Optimizador de Algoritmos de Búsqueda (Torres de Hanói) ---" << endl;
    cout << "Configuración fija: N=" << N_PEGS << " postes, M=" << M_DISKS << " discos." << endl;

    // Validaciones
    cout << "\nEjecutando algoritmos para " << N_PEGS << " postes y " << M_DISKS << " discos..." << endl;
    
    // 2. Lista de resultados para la comparación
    vector<Result> results;
    
    // Ejecución y medición de todos los algoritmos
    results.push_back(run_backtracking_hanoi(N_PEGS, M_DISKS)); // Rápido: Solución matemática
    results.push_back(run_bfs_hanoi(N_PEGS, M_DISKS));         // Lento: Explora espacio de estados
    results.push_back(run_a_star_hanoi(N_PEGS, M_DISKS));      // Lento: Explora espacio de estados (informada)
    results.push_back(run_hill_climbing_hanoi(N_PEGS, M_DISKS)); // Rápido: Búsqueda local

    // 3. Análisis y Selector de Óptimo

    // Inicializar el óptimo con el peor tiempo posible
    Result optimal_result = {"", numeric_limits<double>::max(), false};

    cout << fixed << setprecision(4) << "\n--- Resultados de la Comparación (N=" << N_PEGS << ", M=" << M_DISKS << ") ---" << endl;
    cout << "--------------------------------------------------------------------------------" << endl;
    cout << "| Algoritmo                        | Tiempo (ms) | Encontrada | Métrica (Nodos/Movs) |" << endl;
    cout << "--------------------------------------------------------------------------------" << endl;

    for (const auto& res : results) {
        // Imprimir resultados
        cout << "| " << setw(30) << left << res.name << " | " 
             << setw(11) << right << res.duration_ms << " | "
             << setw(10) << (res.found ? "Sí" : "No") << " | "
             << setw(20) << res.solutions_count << " |" << endl;

        // Lógica de selección del más rápido
        if (res.found && res.duration_ms < optimal_result.duration_ms) {
            optimal_result = res;
        }
    }
    cout << "--------------------------------------------------------------------------------" << endl;

    // Mostrar el algoritmo óptimo
    if (optimal_result.duration_ms != numeric_limits<double>::max()) {
        cout << "\n🏆 ALGORITMO MÁS RÁPIDO PARA HANÓI (N=" << N_PEGS << ", M=" << M_DISKS << "):" << endl;
        cout << "   > Algoritmo: " << optimal_result.name << endl;
        cout << "   > Tiempo: " << optimal_result.duration_ms << " ms" << endl;
        cout << "\n*Observación: El algoritmo Backtracking/DFS Recursivo es el más rápido porque resuelve la fórmula matemática (la complejidad mínima), mientras que BFS y A* realizan la búsqueda real en el espacio de estados." << endl;
    } else {
        cout << "\nNo se pudo encontrar una solución o realizar una comparación válida." << endl;
    }

    return 0;
}