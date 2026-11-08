#include <bits/stdc++.h>
using namespace std;
#include "Chess2.h"
#include "Chess2GameUtils.h"
#include "Chess2Search.h"
#include "blackboxies.h"

int main() {
	// testMateIn3();
	testMateIn3Hard();
	return 0;

	Turochamp tcEval;
	Evaluation noobEval;
	AlphaBetaEngine mmSearch{&tcEval};
	AlphaBetaEngine noobSearch{&noobEval};
	// playAgainstEngine(mmSearch, 1, 4);
	EngineOnEngine(mmSearch, noobSearch, 4, 3);
	return 0;
}
