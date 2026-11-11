
#include "Chess2ALL.h"

#include <bits/stdc++.h>
using namespace std;

#include "blackboxies.h"


// #include "put_nnue_weights_here.h"

void trainPstNNUE() {
	int N = 16;
	int ITERS = 8;
	int GAMES = 4;
	int DEPTH = 3;
	auto nnue = breedPstNNUE(N, ITERS, GAMES, DEPTH); 
	ofstream fout("nnue_" + std::to_string(N) + "_" + std::to_string(ITERS) + "_" + std::to_string(GAMES) + "_" + std::to_string(DEPTH) + ".txt");
	fout << "int DEPTH = " << DEPTH << "\n";
	fout << nnue[0].formatHiddenWeights() << "\n" << nnue[0].formatOutputWeights() << "\n";
	fout << nnue[1].formatHiddenWeights() << "\n" << nnue[1].formatOutputWeights() << "\n";

	fout.close();

	std::cout << "File closed\n";
}

void trainPST() {
	int N = 16;
	int ITERS = 16;
	int GAMES = 8;
	int DEPTH = 4;
	auto nnue = breedPST(N, ITERS, GAMES, DEPTH); 
	ofstream fout("pst_" + std::to_string(N) + "_" + std::to_string(ITERS) + "_" + std::to_string(GAMES) + "_" + std::to_string(DEPTH) + ".txt");
	fout << "int DEPTH = " << DEPTH << "\n";
	fout << nnue[0].formatWeights() << "\n";
	fout << nnue[1].formatWeights() << "\n";

	fout.close();

	std::cout << "File closed\n";
}

void playGameAgainstWhatever() {
	srand(time(0));
	initPRNG(rand());

	pstNNUE nnue = randomPstNNUE();

	// Feel free to set weights here e.g.
	// nnue.hiddenWeights = hiddenWeights00;
	// nnue.outputWeights = outputWeights00;



	AlphaBetaEngine engine(&nnue);
	// EngineOnEngine(engine, engine, 3, 3);
	playAgainstEngine(engine);
}

int main() {
	trainPST();
	return 0;
}
