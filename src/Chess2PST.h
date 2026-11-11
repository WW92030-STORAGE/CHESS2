#ifndef CHESS2_PST_H_NORMALEXISTING
#define CHESS2_PST_H_NORMALEXISTING

#include "Chess2Evaluation.h"

// A NNUE that operates solely off of a piece square table system with no hidden layers.

// Individual pieces and values. A Neural evaluation function has a 2D array of weights that can be scanned through.
struct PieceSquareTable : Evaluation {
	static const int INPUT_SIZE = 64 * 12;
	std::vector<int8_t> weights; // This is the thing that will be iterated on in the training program
	std::vector<uint64_t> bitmasks; // These will store the six white and then six black bitmasks
	eval_t res = 0;

	PieceSquareTable() {
		bitmasks = std::vector<uint64_t>(12, 0);
		weights = std::vector<int8_t>(INPUT_SIZE, 0);
	}

	// For this neural network 
	eval_t evaluateSingleSide(ChessPosition& pos, bool stm) {
		// side * 64 * 6 + pieceType * 64 + square
		std::vector<uint64_t> incoming(12);
		std::vector<uint64_t> off(12);
		std::vector<uint64_t> on(12);
		for (int i = 0; i < 6; i++) {
			incoming[i] = (stm ? pos.white : pos.black)[i];
			incoming[i + 6] = (stm ? pos.black : pos.white)[i];
		}
		for (int i = 0; i < 12; i++) {
			off[i] = (~incoming[i]) & bitmasks[i];
			on[i] = incoming[i] & (~bitmasks[i]);
		}

		for (int i = 0; i < 12; i++) bitmasks[i] = incoming[i];

		for (int no = 0; no < 12; no++) {
			if (on[no]) {
				for (int square = 0; square < 64; square++) {
					if (on[no] & onehot(square)) res += weights[(no<<6) | square];
				}
			}
			if (off[no]) {
				for (int square = 0; square < 64; square++) {
					if (off[no] & onehot(square)) res -= weights[(no<<6) | square];
				}
			}
		}
		return res;
	}
	
	bool operator<(const PieceSquareTable& other) const {
		if (weights != other.weights) return weights < other.weights;
		if (bitmasks != other.bitmasks) return bitmasks < other.bitmasks;
		return res < other.res;
	}

	std::string formatWeights() {
		std::string res = "std::vector<int8_t> weights({";
		for (int j = 0; j < weights.size(); j++) {
			if (j) res += ", ";
			res += std::to_string(weights[j]);
		}
		res += "});\n";
		return res;
	}

	std::string printWeights() {
		std::string res = "";
		for (int j = 0; j < weights.size(); j++) {
			if (j) res += ", ";
			res += std::to_string(weights[j]);
		}
		res += "\n";
		return res;
	}
};

PieceSquareTable randomPST() {
	PieceSquareTable nnue;
	for (int i = 0; i < nnue.weights.size(); i++) nnue.weights[i] = randByte(CH2_PRNG) - 128;
	return nnue;
}

PieceSquareTable crossbreedPST(PieceSquareTable a, PieceSquareTable b) {
	PieceSquareTable x;
	for (int i = 0; i < x.weights.size(); i++) x.weights[i] = randBool(CH2_PRNG) ? a.weights[i] : b.weights[i];
	return x;
}

PieceSquareTable mutatePST(PieceSquareTable n, double threshold = 0.005) {
	PieceSquareTable res = n;
	for (int i = 0; i < res.weights.size(); i++) {
		if (rand01(CH2_PRNG) < threshold) res.weights[i] = randByte(CH2_PRNG) - 128;
	}
	return res;
}

#endif