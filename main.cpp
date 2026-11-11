
#include "Chess2ALL.h"

#include <bits/stdc++.h>
using namespace std;

#include "blackboxies.h"


#include "put_nnue_weights_here.h"

void breed() {
	int N = 16;
	int ITERS = 8;
	int GAMES = 4;
	int DEPTH = 3;
	auto nnue = breedPstNNUE(16, 8, 4, DEPTH); 
	ofstream fout("nnue_" + std::to_string(N) + "_" + std::to_string(ITERS) + "_" + std::to_string(GAMES) + "_" + std::to_string(DEPTH) + ".txt");
	fout << "int DEPTH = " << DEPTH << "\n";
	fout << nnue[0].formatHiddenWeights() << "\n" << nnue[0].formatOutputWeights() << "\n";
	fout << nnue[1].formatHiddenWeights() << "\n" << nnue[1].formatOutputWeights() << "\n";

	fout.close();

	std::cout << "File closed\n";
}

int main() {
	srand(time(0));
	initPRNG(rand());

	pstNNUE nnue;
	nnue.hiddenWeights = hiddenWeights00;
	nnue.outputWeights = outputWeights00;

	AlphaBetaEngine engine(&nnue);
	// EngineOnEngine(engine, engine, 3, 3);
	playAgainstEngine(engine);
	return 0;
}
