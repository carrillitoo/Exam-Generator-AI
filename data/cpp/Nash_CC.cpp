#include <iostream>
#include <vector>
#include <string>

using namespace std;

struct Payoff { int p1; int p2; };

int main() {
    vector<vector<Payoff>> matrix = {
        { {4, 4}, {0, 2} }, // Stag
        { {2, 0}, {2, 2} }  // Hare
    };

    int count = 0;
    bool stagStag = false;
    bool hareHare = false;

    for (int r = 0; r < 2; ++r) {
        for (int c = 0; c < 2; ++c) {
            bool rBest = true; bool cBest = true;
            for (int ra = 0; ra < 2; ++ra) if (matrix[ra][c].p1 > matrix[r][c].p1) rBest = false;
            for (int ca = 0; ca < 2; ++ca) if (matrix[r][ca].p2 > matrix[r][c].p2) cBest = false;

            if (rBest && cBest) {
                if (r==0 && c==0) stagStag = true;
                if (r==1 && c==1) hareHare = true;
            }
        }
    }

    string finalRes = "None";
    // Simplificado a lista separada por comas
    if (stagStag && hareHare) finalRes = "(Stag, Stag), (Hare, Hare)";
    else if (stagStag) finalRes = "(Stag, Stag)";
    else if (hareHare) finalRes = "(Hare, Hare)";

    // --- OUTPUT SIMPLIFICADO ---
    cout << "\n🏆 FINAL RESULT:" << endl;
    cout << "   > Algoritmo: " << finalRes << endl;

    return 0;
}