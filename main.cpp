#include <bits/stdc++.h>
using namespace std;
#include "Chess2.h"

#include "blackboxies.h"

void playGame() {
	ChessPosition pos;
	pos.init();
	while (true) {
		std::cout << pos.to_string() << "\n";
		int r0, r1, c0, c1;
		std::cin >> c0 >> r0 >> c1 >> r1;
		ChessMove move((r0<<3)|c0, (r1<<3)|c1, 0);
		if (!pos.isLegalMove(move)) {
			std::cout << "Invalid move\n";
			continue;
		}
		if (pos.apply(move)) pos.setSTM(!pos.sideToMove());
		else std::cout << "Error move\n";
		continue;
	}
}

/*

A sequence of moves from a hypothetical very low rated chess game that creates a variety of scenarios admitting edge case testing:
4 1 4 3
4 6 4 4
6 0 5 2
3 6 3 4
4 3 3 4
5 7 1 3

*/

int main() {
	testActualPromotion();
	return 0;
}
