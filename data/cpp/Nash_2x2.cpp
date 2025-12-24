#include <iostream>
#include <vector>
#include <string>

using namespace std;

struct Payoff { int p1; int p2; };

int main() {
    // 2x2 Matrix
    vector<vector<Payoff>> matrix = {
        { {2, 1}, {0, 0} }, // U
        { {0, 0}, {1, 2} }  // D
    };

    int rows = matrix.size();
    int cols = matrix[0].size();
    string resultString = "None";
    bool found = false;

    cout << "--- Nash Equilibrium Analyzer (2x2 Game) ---" << endl;
    
    // Search
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            bool rowBest = true;
            bool colBest = true;

            for (int ra = 0; ra < rows; ++ra) if (matrix[ra][c].p1 > matrix[r][c].p1) rowBest = false;
            for (int ca = 0; ca < cols; ++ca) if (matrix[r][ca].p2 > matrix[r][c].p2) colBest = false;

            if (rowBest && colBest) {
                string rName = (r == 0) ? "U" : "D";
                string cName = (c == 0) ? "L" : "R";
                resultString = "(" + rName + ", " + cName + ")";
                found = true;
            }
        }
    }

    // --- OUTPUT SIMPLIFICADO ---
    // Antes: Frases largas
    // Ahora: Coordenadas directas exactas. Ej: "(D, R)"
    cout << "\n🏆 FINAL RESULT:" << endl;
    cout << "   > Algoritmo: " << resultString << endl;

    return 0;
}