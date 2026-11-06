#include <bits/stdc++.h>
using namespace std;
#include "Chess2.h"


int main() {
	ChessMove cm(getIndex("d2"), getIndex("d4"), 1);
	cout << cm.to_string() << endl;

	uint64_t aBitboard = bisectorBB[2];
	cout << i64togrid(aBitboard) << endl;

	ChessPosition cp;
	cp.clear();
	cp.white[2] |= 1ULL<<38;
	cp.white[5] |= 1ULL<<3;
	cout << cp.to_string() << "\n";

	for (auto x : cp.possiblePseudoLegals(0)) cout << x.to_string() << endl;

	return 0;
}
