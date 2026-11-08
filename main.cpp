#include <bits/stdc++.h>
using namespace std;
#include "Chess2.h"
#include "Chess2GameUtils.h"
#include "Chess2Search.h"
#include "blackboxies.h"

int main() {
	Turochamp eval;
	AlphaBetaEngine mmSearch{&eval};
	AlphaBetaEngine noobSearch{&eval};
	// playAgainstEngine(mmSearch, 1, 4);
	EngineOnEngine(mmSearch, mmSearch, 2, 2);
	return 0;
}
