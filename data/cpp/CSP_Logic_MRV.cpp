#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>

using namespace std;

// --- CSP: Mini Logic Puzzle (Scheduling) ---
// Tareas: A, B, C, D
// Dominios (Horas): {1, 2, 3, 4}
// Restricciones: A != B, B > A, C > B, D != C

map<string, int> assignment;
vector<int> domain = {1, 2, 3, 4};
vector<string> vars = {"A", "B", "C", "D"};

bool check_constraints() {
    // A != B
    if (assignment.count("A") && assignment.count("B") && assignment["A"] == assignment["B"]) return false;
    // B > A
    if (assignment.count("A") && assignment.count("B") && !(assignment["B"] > assignment["A"])) return false;
    // C > B
    if (assignment.count("B") && assignment.count("C") && !(assignment["C"] > assignment["B"])) return false;
    // D != C
    if (assignment.count("D") && assignment.count("C") && assignment["D"] == assignment["C"]) return false;
    
    return true;
}

// Simulación de MRV (Minimum Remaining Values):
// Ordenamos variables por 'dificultad' (más restringidas primero en este caso estático)
// B y C están muy restringidas por las desigualdades.
vector<string> ordered_vars = {"A", "B", "C", "D"}; 

bool solve(int idx) {
    if (idx == ordered_vars.size()) return true;
    string var = ordered_vars[idx];

    for (int val : domain) {
        assignment[var] = val;
        if (check_constraints()) {
            if (solve(idx + 1)) return true;
        }
        assignment.erase(var);
    }
    return false;
}

int main() {
    cout << "--- CSP Solver: Scheduling Logic (MRV Heuristic) ---" << endl;
    // Partial Assignment: D=2
    assignment["D"] = 2;
    
    // Remove D from vars to solve (since it's assigned)
    ordered_vars = {"A", "B", "C"};

    if (solve(0)) {
        // Formato esperado: A=1, B=2...
        string res = "A=" + to_string(assignment["A"]) + 
                     ", B=" + to_string(assignment["B"]) + 
                     ", C=" + to_string(assignment["C"]);
        
        cout << "\n🏆 SOLUCIÓN ENCONTRADA:" << endl;
        cout << "   > Algoritmo: " << res << endl;
    }
    return 0;
}