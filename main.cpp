
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

template <typename EVAL>
void playGameAgainstWhatever(std::vector<int8_t> x) {
	EVAL_EXTENDS_EVALUATION
	srand(time(0));
	initPRNG(rand());

	EVAL nnue = randomEvaluation<EVAL>();
	if (x.size() == nnue.weights.size()) nnue.weights = x;


	AlphaBetaEngine engine(&nnue);
	playAgainstEngine(engine);
}

int main() {
	playGameAgainstWhatever<Turochamp>(weightsxx);
	return 0;
}
