#include <iostream>
#include <vector>
#include <string>

using namespace std;

struct Payoff { int p1; int p2; };

int main() {
    // Matching Pennies
    vector<vector<Payoff>> matrix = {
        { {1, -1}, {-1, 1} },
        { {-1, 1}, {1, -1} } 
    };

    bool found = false;
    cout << "--- Matching Pennies Analyzer ---" << endl;

    for (int r = 0; r < 2; ++r) {
        for (int c = 0; c < 2; ++c) {
            bool rBest = true; bool cBest = true;
            for (int ra = 0; ra < 2; ++ra) if (matrix[ra][c].p1 > matrix[r][c].p1) rBest = false;
            for (int ca = 0; ca < 2; ++ca) if (matrix[r][ca].p2 > matrix[r][c].p2) cBest = false;
            if (rBest && cBest) found = true;
        }
    }

    // --- OUTPUT SIMPLIFICADO ---
    // Solo responde "No" o "Yes"
    string outputMsg = found ? "Yes" : "No";

    cout << "\n🏆 FINAL RESULT:" << endl;
    cout << "   > Algoritmo: " << outputMsg << endl;

    return 0;
}