#include <iostream>
#include <vector>
#include <string>
#include <cmath>

using namespace std;

// --- CSP: 4-Queens con Forward Checking ---
// Tablero 4x4. Q[i] = columna de la reina en la fila i.
// Partial: Q[0] = 1 (Reina en fila 0, columna 1)

int N = 4;
vector<int> board(N, -1); // -1 vacio

bool is_safe(int row, int col) {
    for (int i = 0; i < row; ++i) {
        if (board[i] == col || abs(board[i] - col) == abs(i - row))
            return false;
    }
    return true;
}

bool solve(int row) {
    if (row == N) return true;
    
    // Si la fila ya está pre-asignada (Partial assignment)
    if (board[row] != -1) {
        if (is_safe(row, board[row])) return solve(row + 1);
        else return false;
    }

    for (int col = 0; col < N; ++col) {
        if (is_safe(row, col)) {
            board[row] = col; // Asignar
            if (solve(row + 1)) return true;
            board[row] = -1;  // Backtrack
        }
    }
    return false;
}

int main() {
    // Partial Assignment: Fila 0 en Columna 1 (segunda casilla)
    board[0] = 1; 

    cout << "--- CSP Solver: 4-Queens (Forward Checking) ---" << endl;
    
    if (solve(0)) {
        // Output format: [1, 3, 0, 2]
        string res = "[";
        for(int i=0; i<N; ++i) {
            res += to_string(board[i]);
            if(i < N-1) res += ", ";
        }
        res += "]";

        cout << "\n🏆 SOLUCIÓN ENCONTRADA:" << endl;
        cout << "   > Algoritmo: " << res << endl;
    } else {
        cout << "   > Algoritmo: No Solution" << endl;
    }
    return 0;
}