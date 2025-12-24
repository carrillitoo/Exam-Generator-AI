#include <iostream>
#include <vector>
#include <string>

using namespace std;

struct Payoff { int p1; int p2; };

int main() {
    vector<vector<Payoff>> matrix = {
        { {3, 3}, {0, 5} }, // C
        { {5, 0}, {1, 1} }  // D
    };

    string strategyFound = "None";

    for (int r = 0; r < 2; ++r) {
        for (int c = 0; c < 2; ++c) {
            bool rBest = true; bool cBest = true;
            for (int ra = 0; ra < 2; ++ra) if (matrix[ra][c].p1 > matrix[r][c].p1) rBest = false;
            for (int ca = 0; ca < 2; ++ca) if (matrix[r][ca].p2 > matrix[r][c].p2) cBest = false;

            if (rBest && cBest) {
                // Simplificado a la notación estándar del dilema
                if (r==1 && c==1) strategyFound = "(Betray, Betray)";
                else if (r==0 && c==0) strategyFound = "(Coop, Coop)";
                else strategyFound = "Other";
            }
        }
    }

    // --- OUTPUT SIMPLIFICADO ---
    cout << "\n🏆 FINAL RESULT:" << endl;
    cout << "   > Algoritmo: " << strategyFound << endl;

    return 0;
}