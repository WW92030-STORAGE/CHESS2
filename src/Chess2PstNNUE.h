#ifndef CHESS2_PST_NNUE_H_NORMALEXISTING
#define CHESS2_PST_NNUE_H_NORMALEXISTING

#include "Chess2Evaluation.h"

// TODO make the weights of this thing a 1D array similar to PieceSquareTable.

// A failed attempt at making a PST based NNUE with one hidden layer.

// Individual pieces and values. A Neural evaluation function has a 2D array of weights that can be scanned through.
struct pstNNUE : Evaluation {
	static const int INPUT_SIZE = 64 * 12;
	static const int ACCUM_SIZE = 64; // Feel free to change this
	std::vector<std::vector<int8_t>> hiddenWeights; // This is the thing that will be iterated on in the training program
	std::vector<int8_t> outputWeights; // Weights of strictly the final layer
	std::vector<uint64_t> bitmasks; // These will store the six white and then six black bitmasks

	// accums
	
	std::vector<eval_t> accum[2];

	pstNNUE() {
		bitmasks = std::vector<uint64_t>(12, 0);
		hiddenWeights = std::vector<std::vector<int8_t>>(INPUT_SIZE + 1, std::vector<int8_t>(ACCUM_SIZE, 0));
		outputWeights = std::vector<int8_t>((ACCUM_SIZE<<1) + 1, 0);
		for (int i = 0; i < 2; i++) accum[i] = std::vector<eval_t>(ACCUM_SIZE, 0);
		// The last row in each vector is the bias and will be directly added to everything. In particular, the last row of hiddenWeights are the accum biases and will be added to the accumulators before performing ReLU and whatnot.
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

		for (int i = 0; i < INPUT_SIZE; i++) {
			uint64_t square = onehot(i & 63);
			uint64_t no = (i>>6);
			bool side = no >= 6;

			if (on[no] & square) {
				for (int j = 0; j < ACCUM_SIZE; j++) {
					accum[side][j] += hiddenWeights[i][j];
				}
			}
			if (off[no] & square) {
				for (int j = 0; j < ACCUM_SIZE; j++) {
					accum[side][j] -= hiddenWeights[i][j];
				}
			}
		}

		eval_t res = 0;
		for (int i = 0; i < ACCUM_SIZE; i++) res += outputWeights[i] * relu(accum[0][i] + hiddenWeights[INPUT_SIZE][i]) + outputWeights[i + ACCUM_SIZE] + relu(accum[1][i] + hiddenWeights[INPUT_SIZE][i]);
		return relu(res);
	}
	
	bool operator<(const pstNNUE& other) const {
		if (hiddenWeights != other.hiddenWeights) return hiddenWeights < other.hiddenWeights;
		if (outputWeights != other.outputWeights) return outputWeights < other.outputWeights;
		if (bitmasks != other.bitmasks) return bitmasks < other.bitmasks;
		for (int i = 0; i < 2; i++) {
			if (accum[i] != other.accum[i]) return accum[i] < other.accum[i];
		}
		return 0;
	}

	std::string formatHiddenWeights() {
		std::string res = "std::vector<std::vector<int8_t>> hiddenWeights({";
		for (int i = 0; i < hiddenWeights.size(); i++) {
			if (i) res = res + ", ";
			res = res + "{";
			for (int j = 0; j < hiddenWeights[i].size(); j++) {
				if (j) res += ", ";
				res += std::to_string(hiddenWeights[i][j]);
			} res += "}";
		}
		res += "});";
		return res;
	}

	std::string formatOutputWeights() {
		std::string res = "std::vector<int8_t> outputWeights({";
		for (int j = 0; j < outputWeights.size(); j++) {
			if (j) res += ", ";
			res += std::to_string(outputWeights[j]);
		}
		res += "});\n";
		return res;
	}

	std::string printWeights() {
		std::string res = "";
		for (int i = 0; i < hiddenWeights.size(); i++) {
			for (int j = 0; j < hiddenWeights[i].size(); j++) {
				if (j) res += ", ";
				res += std::to_string(hiddenWeights[i][j]);
			}
			res = res + "\n";
		}
		for (int j = 0; j < outputWeights.size(); j++) {
			if (j) res += ", ";
			res += std::to_string(outputWeights[j]);
		}
		res += "\n";
		return res;
	}
};

pstNNUE randomPstNNUE() {
	pstNNUE nnue;
	for (int i = 0; i < nnue.hiddenWeights.size(); i++) {
		for (int j = 0; j < nnue.hiddenWeights[i].size(); j++) nnue.hiddenWeights[i][j] = randByte(CH2_PRNG) - 128;
	}
	for (int i = 0; i < nnue.outputWeights.size(); i++) nnue.outputWeights[i] = randByte(CH2_PRNG) - 128;
	return nnue;
}

pstNNUE crossbreedPstNNUE(pstNNUE a, pstNNUE b) {
	pstNNUE x;
	for (int i = 0; i < x.hiddenWeights.size(); i++) {
		for (int j = 0; j < x.hiddenWeights[i].size(); j++) x.hiddenWeights[i][j] = randBool(CH2_PRNG) ? a.hiddenWeights[i][j] : b.hiddenWeights[i][j];
	}
	for (int i = 0; i < x.outputWeights.size(); i++) x.outputWeights[i] = randBool(CH2_PRNG) ? a.outputWeights[i] : b.outputWeights[i];
	return x;
}

pstNNUE mutatePstNNUE(pstNNUE n, double threshold = 0.005) {
	pstNNUE res = n;
	for (int i = 0; i < res.hiddenWeights.size(); i++) {
		for (int j = 0; j < res.hiddenWeights[i].size(); j++) {
			if (rand01(CH2_PRNG) < threshold) res.hiddenWeights[i][j] = randByte(CH2_PRNG) - 128;
		}
	}
	for (int i = 0; i < res.outputWeights.size(); i++) {
		if (rand01(CH2_PRNG) < threshold) res.outputWeights[i] = randByte(CH2_PRNG) - 128;
	}
	return res;
}

#endif