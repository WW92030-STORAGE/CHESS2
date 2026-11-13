#ifndef CHESS2TRAINING_HXHXHX_NROMALEXSTIGN
#define CHESS2TRAINING_HXHXHX_NROMALEXSTIGN

#include <iostream>
#include <type_traits>

#include "Chess2.h"
#include "ChessUtils.h"
#include "Chess2Evaluation.h"
#include "Chess2Search.h"

#include "Chess2PST.h"
#include "Chess2PstNNUE.h"

// TODO modularize all this

// Test two engines. Returns -1 if white wins, 1 if black wins, and 0 on a draw.
int testEngines(SearchEngine& white, SearchEngine& black, int depth = 4, bool verbose = false) {
	ChessPosition pos;
	pos.init();
	while (true) {
		if (verbose) std::cout << pos.to_string() << "\nEVAL(" << pos.sideToMove() << "): " << (pos.sideToMove() ? black : white).eval->evaluate(pos) << "\n";
		// printMovesWithEvals(pos, *((pos.sideToMove() ? black : white).eval), 3);
		if (pos.checkmated()) { // pos.checkmated() checks if sideToMove() is checkmated -- sideToMove() is the LOSING SIDE if checkmated() returns 0.
			if (verbose) std::cout << "CHECKMATE - Game end. Player " << !pos.sideToMove() << " WINS\n";
			return (pos.sideToMove()) ? 1 : -1;
		} else if (pos.noLegalMoves() || pos.forcedStalemate()) {
			if (verbose) std::cout << "STALEMATE - Game end\n";
			break;
		}
		ChessMove move;
		move = (pos.sideToMove() ? black : white).search(pos, depth);
		if (verbose)std::cout << "Chosen move: " << move.to_string() << "\n"; 
		if (pos.apply(move)) pos.setSTM(!pos.sideToMove());
		else {
			if (verbose) std::cout << "Error move\n";
			break;
		}
		continue;
	}
	return 0;
}

// Runs a tournament of N machines and I iterations in the following steps:
// Round-robins each machine against another machine, in (games) games each of either side, and keeps track of the end score (win = 2, draw = 1, lose = 0)
// The top 2 machines get cross bred and mutated to form N - 2 additional machines to form the next generation. (Keep the two survivors in case they come out on top again)
template <typename EVAL>
std::vector<EVAL> breedEvaluation(int N = 16, int iters = 4, int games = 4, int depth = 4) {
	EVAL_EXTENDS_EVALUATION
	if (N < 4) N = 4;
	if (iters < 1) iters = 1;
	if (games < 1) games = 1;
	if (depth < 1) depth = 1;

	std::vector<EVAL> machines(N);
	for (int i = 0; i < N; i++) machines[i] = randomEvaluation<EVAL>();
	std::vector<int> scores(N, 0);

	AlphaBetaEngine A, B;

	for (int iter = 0; iter < iters; iter++) {
		std::cout << "ROUND " << iter + 1 << "/" << iters << "\n";
		for (int i = 0; i < scores.size(); i++) scores[i] = 0;
		for (int a = 0; a < N; a++) {
			for (int b = 0; b < a; b++) {
				A.eval = &machines[a];
				B.eval = &machines[b];

				for (int g = 0; g < games; g++) {
					int verdict = testEngines(A, B, depth);
					std::cout << "[" << a << "/" << b << ": " << verdict << "] ";
					std::cout.flush(); 
					if (verdict == -1) scores[a] += 2;
					else if (verdict == 1) scores[b] += 2;
					else if (verdict == 0) {
						scores[a]++; scores[b]++;
					}

					verdict = testEngines(B, A, depth);
					std::cout << "[" << b << "/" << a << ": " << verdict << "] ";
					std::cout.flush(); 
					if (verdict == -1) scores[b] += 2;
					else if (verdict == 1) scores[a] += 2;
					else if (verdict == 0) {
						scores[a]++; scores[b]++;
					}
				}
			}
		}
		std::cout << std::endl;
		std::cout << "SCORE: ";
		for (int i = 0; i < N; i++) std::cout << "[" << i << ": " << scores[i] << "]";
		std::cout << std::endl;
		
		// end testing
		std::vector<std::pair<int, EVAL>> toSort(N);
		for (int i = 0; i < N; i++) toSort[i] = {-scores[i], machines[i]};
		std::sort(toSort.begin(), toSort.end());
		machines[0] = toSort[0].second;
		machines[1] = toSort[1].second;
		for (int i = 2; i < N; i++) machines[i] = mutateEvaluation<EVAL>(crossbreedEvaluation<EVAL>(machines[0], machines[1]));
	}

	return machines;
}

// Runs a tournament of N machines and I iterations in the following steps:
// Round-robins each machine against another machine, in (games) games each of either side, and keeps track of the end score (win = 2, draw = 1, lose = 0)
// The top 2 machines get cross bred and mutated to form N - 2 additional machines to form the next generation. (Keep the two survivors in case they come out on top again)
std::vector<pstNNUE> breedPstNNUE(int N = 16, int iters = 4, int games = 4, int depth = 4) {
	if (N < 4) N = 4;
	if (iters < 1) iters = 1;
	if (games < 1) games = 1;
	if (depth < 1) depth = 1;

	std::vector<pstNNUE> machines(N);
	for (int i = 0; i < N; i++) machines[i] = randomPstNNUE();
	std::vector<int> scores(N, 0);

	AlphaBetaEngine A, B;

	for (int iter = 0; iter < iters; iter++) {
		std::cout << "ROUND " << iter + 1 << "/" << iters << "\n";
		for (int i = 0; i < scores.size(); i++) scores[i] = 0;
		for (int a = 0; a < N; a++) {
			for (int b = 0; b < a; b++) {
				A.eval = &machines[a];
				B.eval = &machines[b];

				for (int g = 0; g < games; g++) {
					int verdict = testEngines(A, B, depth);
					std::cout << "[" << a << "/" << b << ": " << verdict << "] ";
					std::cout.flush(); 
					if (verdict == -1) scores[a] += 2;
					else if (verdict == 1) scores[b] += 2;
					else if (verdict == 0) {
						scores[a]++; scores[b]++;
					}

					verdict = testEngines(B, A, depth);
					std::cout << "[" << b << "/" << a << ": " << verdict << "] ";
					std::cout.flush(); 
					if (verdict == -1) scores[b] += 2;
					else if (verdict == 1) scores[a] += 2;
					else if (verdict == 0) {
						scores[a]++; scores[b]++;
					}
				}
			}
		}
		std::cout << std::endl;
		std::cout << "SCORE: ";
		for (int i = 0; i < 8; i++) std::cout << "[" << i << ": " << scores[i] << "]";
		std::cout << std::endl;
		
		// end testing
		std::vector<std::pair<int, pstNNUE>> toSort(N);
		for (int i = 0; i < N; i++) toSort[i] = {-scores[i], machines[i]};
		std::sort(toSort.begin(), toSort.end());
		machines[0] = toSort[0].second;
		machines[1] = toSort[1].second;
		for (int i = 2; i < N; i++) machines[i] = mutatePstNNUE(crossbreedPstNNUE(machines[0], machines[1]));
	}

	return machines;
}

#endif