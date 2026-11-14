
#include "Chess2ALL.h"

#include <bits/stdc++.h>
using namespace std;

#include "blackboxies.h"


#include "put_nnue_weights_here.h"

std::string timestamp() { 
	return "_" + std::to_string(time(0)) + "s";
}

void trainPstNNUE() {
	int N = 16;
	int ITERS = 8;
	int GAMES = 4;
	int DEPTH = 2;
	auto nnue = breedPstNNUE(N, ITERS, GAMES, DEPTH); 
	ofstream fout("nnue_" + std::to_string(N) + "_" + std::to_string(ITERS) + "_" + std::to_string(GAMES) + "_" + std::to_string(DEPTH) + timestamp() + ".txt");
	fout << "int DEPTH = " << DEPTH << "\n";
	fout << nnue[0].formatHiddenWeights() << "\n" << nnue[0].formatOutputWeights() << "\n";
	fout << nnue[1].formatHiddenWeights() << "\n" << nnue[1].formatOutputWeights() << "\n";

	fout.close();

	std::cout << "File closed\n";
}

// PST 8/8/8/2 had draws by the end

template <typename EVAL>
void trainEvaluation(int N = 16, int ITERS = 8, int GAMES = 4, int DEPTH = 3) {
	EVAL_EXTENDS_EVALUATION
	auto nnue = breedEvaluation<EVAL>(N, ITERS, GAMES, DEPTH); 
	ofstream fout(std::string(typeid(EVAL).name()) + "_" + std::to_string(N) + "_" + std::to_string(ITERS) + "_" + std::to_string(GAMES) + "_" + std::to_string(DEPTH) + timestamp() + ".txt");
	fout << "int DEPTH = " << DEPTH << "\n";
	fout << nnue[0].formatWeights() << "\n";
	fout << nnue[1].formatWeights() << "\n";

	fout.close();

	std::cout << "File closed\n";
}

void playGameAgainstWhatever() {
	srand(time(0));
	initPRNG(rand());

	SymmetricPST pst = randomEvaluation<SymmetricPST>();


	AlphaBetaEngine engine(&pst);
	EngineOnEngine(engine, engine, 3, 3);
	playAgainstEngine(engine);
}

int main() {
	trainEvaluation<Turochamp>(8, 8, 4, 2);
	return 0;
}
