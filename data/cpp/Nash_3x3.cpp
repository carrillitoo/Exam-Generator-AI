#include <iostream>
#include <vector>
#include <string>

using namespace std;

struct Payoff { int p1; int p2; };

int main() {
    // 3x3 Matrix
    vector<vector<Payoff>> matrix = {
        { {3, 3}, {0, 4}, {1, 0} }, 
        { {4, 0}, {2, 2}, {0, 0} }, 
        { {0, 1}, {0, 0}, {5, 5} } 
    };

    int rows = 3; int cols = 3;
    string resultLog = "";
    int count = 0;

    cout << "--- Nash Analyzer 3x3 ---" << endl;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            bool rBest = true; bool cBest = true;
            for (int ra = 0; ra < rows; ++ra) if (matrix[ra][c].p1 > matrix[r][c].p1) rBest = false;
            for (int ca = 0; ca < cols; ++ca) if (matrix[r][ca].p2 > matrix[r][c].p2) cBest = false;

            if (rBest && cBest) {
                count++;
                if (count > 1) resultLog += ", "; // Separador simple coma
                resultLog += "(" + to_string(matrix[r][c].p1) + "," + to_string(matrix[r][c].p2) + ")";
            }
        }
    }

    string finalOutput;
    if (count == 0) finalOutput = "None";
    else finalOutput = resultLog;

    // --- OUTPUT SIMPLIFICADO ---
    // Ahora devuelve solo los pares. Ej: "(3,3), (5,5)"
    cout << "\n🏆 FINAL RESULT:" << endl;
    cout << "   > Algoritmo: " << finalOutput << endl;
    
    return 0;
}