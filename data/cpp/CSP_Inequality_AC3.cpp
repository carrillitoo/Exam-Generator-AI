#include <iostream>
#include <vector>
#include <string>
#include <map>

using namespace std;

// --- CSP: Desigualdades Numéricas ---
// Variables: X, Y, Z
// Dominios: {1, 2, 3}
// Restricciones: X < Y, Y < Z

vector<string> variables = {"X", "Y", "Z"};
vector<int> domain = {1, 2, 3};
map<string, int> assignment;

bool is_consistent(string var, int val) {
    if (var == "X") {
        if (assignment.count("Y") && !(val < assignment["Y"])) return false;
    }
    if (var == "Y") {
        if (assignment.count("X") && !(assignment["X"] < val)) return false;
        if (assignment.count("Z") && !(val < assignment["Z"])) return false;
    }
    if (var == "Z") {
        if (assignment.count("Y") && !(assignment["Y"] < val)) return false;
    }
    return true;
}

bool solve(int idx) {
    if (idx == variables.size()) return true;
    string var = variables[idx];

    for (int val : domain) {
        if (is_consistent(var, val)) {
            assignment[var] = val;
            if (solve(idx + 1)) return true;
            assignment.erase(var);
        }
    }
    return false;
}

int main() {
    cout << "--- CSP Solver: Numeric Inequalities ---" << endl;
    cout << "Constraints: X < Y < Z | Domains: {1, 2, 3}" << endl;

    if (solve(0)) {
        string res = "X=" + to_string(assignment["X"]) + 
                     ", Y=" + to_string(assignment["Y"]) + 
                     ", Z=" + to_string(assignment["Z"]);
        
        cout << "\n🏆 SOLUCIÓN ENCONTRADA:" << endl;
        cout << "   > Algoritmo: " << res << endl;
    } else {
        cout << "   > Algoritmo: No Solution" << endl;
    }
    return 0;
}