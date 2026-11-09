#ifndef CHESS2_EVAL_H_NORMALEXISTING
#define CHESS2_EVAL_H_NORMALEXISTING

#include "Chess2.h"
#include "ChessUtils.h"

// Evaluation function

typedef int64_t eval_t;
constexpr eval_t CH2_INF = (INT32_MAX>>1);
constexpr eval_t CHECKMT = -(CH2_INF>>2);

eval_t relu(eval_t v) {
	return (v > 0) ? v : 0;
}

// BEGIN EVALUATION FUNCTIONS


// This is the purest form of an evaluation system. Subclasses can derive the evaluation (more specifically, eval) function.
// WARNING - All evaluations evaluate relative -- if the evaluation is high then sideToMove() has an advantage, if low then !sideToMove() has the advantage.
struct Evaluation {
	// simple evaluation system based on material, checks, and checkmates

	// evaluateSingleSide computes evaluation of a single side. This function may or may not correspond exactly to the evaluate method (e.g. evaluation = ess(1) - ess(0))
	virtual eval_t evaluateSingleSide(ChessPosition& pos, bool stm) {
		bool originalSTM = pos.sideToMove();
		pos.setSTM(stm);
		eval_t res = 0;
		const int weights[5] = {100, 300, 300, 500, 900}; // everything is scaled up by 100 to adhere to the centipawns metric
		for (int i = 0; i < 5; i++) {
			res += weights[i] * popCount((stm ? pos.black : pos.white)[i]);
		}
		if (pos.isInCheck()) {
			if (res > 0) res>>=1;
			else res = res + res;
		}
		pos.setSTM(originalSTM);
		return res;
	}
	virtual eval_t evaluate(ChessPosition& pos) {
		if (pos.noLegalMoves() || pos.forcedStalemate()) {
			if (pos.isInCheck()) return CHECKMT;
			return 0;
		}
		eval_t res = evaluateSingleSide(pos, 1) - evaluateSingleSide(pos, 0);
		if (!pos.sideToMove()) res = -res;
		return res;
	}

	eval_t operator()(ChessPosition& pos) { return evaluate(pos); }
};

struct Turochamp : Evaluation {
	// Turing-Champernowne method, with some slight variaions
	virtual eval_t evaluateSingleSide(ChessPosition& pos, bool stm) {
		bool originalSTM = pos.sideToMove();
		pos.setSTM(stm);
		eval_t res = 0;
		const int weights[5] = {100, 300, 350, 500, 1000}; // everything is scaled up by 100 to adhere to the centipawns metric
		for (int i = 0; i < 5; i++) {
			res += weights[i] * popCount((stm ? pos.black : pos.white)[i]);
		}

		// 1. add square root of number of moves per piece except kings and pawns
		uint64_t rbn = stm ? (pos.black[1] | pos.black[2] | pos.black[3]) : (pos.white[1] | pos.white[2] | pos.white[3]);
		uint64_t notKPs = rbn | (stm ? pos.black : pos.white)[4];
		uint64_t pawns = (stm ? pos.black : pos.white)[0];
		uint64_t king = (stm ? pos.black : pos.white)[5];
		uint64_t allPieces = (notKPs | pawns | king) | pos.getAllPieces(!stm);
		for (int i = 0; i < 64; i++) {
			uint64_t sq = onehot(i);
			if (sq & notKPs) {
				eval_t ctr = 0;
				for (auto i : pos.possibleLegalMovesPerSquare(stm, i)) ctr += i.capture() ? 2 : 1;
				res += sqrtf(10000 * ctr);

				if (sq & rbn) {
					eval_t defending = popCount(pos.isAttackedBy(i, stm));
					if (defending) res += 100 + 50 * defending;
				}
			}
			if (sq & king) {
				std::vector<ChessMove> nocastle;
				eval_t ctr = 0;
				bool castleImmediatelyEligible = pos.lastMove2.castle();
				for (auto i : pos.possibleLegalMovesPerSquare(stm, i)) {
					if (!i.castle()) ctr += i.capture() ? 2 : 1;
					else castleImmediatelyEligible = 1;
				}
				res += sqrtf(10000 * ctr);

				// castling rules
				if (pos.canCastle()) res += (castleImmediatelyEligible ? 100 : 0);

				// king safety
				int8_t diff = (i>>3) - (i & 7);
				int8_t sum = (i>>3) + (i & 7);
				uint64_t diagonals = rayAttack(diagonalBB(diff), allPieces, i, 0) | rayAttack(diagonalBB(diff), allPieces, i, 1) | rayAttack(bisectorBB[sum], allPieces, i, 0) | rayAttack(bisectorBB[sum], allPieces, i, 1);
				uint64_t orthos = rayAttack(rankBB[i>>3], allPieces, i, 0)| rayAttack(rankBB[i>>3], allPieces, i, 1) | rayAttack(fileBB[i & 7], allPieces, i, 0) | rayAttack(fileBB[i & 7], allPieces, i, 1);
				res -= popCount(diagonals | orthos);
			} if (sq & pawns) {
				res += (bool(pos.isAttackedBy(i, stm) & (notKPs | king)) ? 30 : 0);
			}
		}

		// evaluate pawns by rank
		for (int8_t i = 2; i < 7; i++) {
			int8_t r = stm ? 7 - i : i;
			res += 20 * (i - 1) * popCount(rankBB[r] & pawns);
		}

		if (pos.isInCheck()) {
			if (res > 0) res>>=1;
			else res = res + res;
		}

		pos.setSTM(originalSTM);
		return res;
	}
};

// Individual pieces and values. A Neural evaluation function has a 2D array of weights that can be scanned through.
struct NNUE : Evaluation {
	static const int INPUT_SIZE = 64 * 12;
	static const int ACCUM_SIZE = 64 * 4; // Feel free to change this
	std::vector<std::vector<int8_t>> hiddenWeights; // This is the thing that will be iterated on in the training program
	std::vector<int8_t> outputWeights; // Weights of strictly the final layer
	std::vector<uint64_t> bitmasks; // These will store the six white and then six black bitmasks

	// accums
	
	std::vector<eval_t> accum[2];

	NNUE() {
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
		return res;
	}
	
	bool operator<(const NNUE& other) const {
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

NNUE randomNNUE() {
	NNUE nnue;
	for (int i = 0; i < nnue.hiddenWeights.size(); i++) {
		for (int j = 0; j < nnue.hiddenWeights[i].size(); j++) nnue.hiddenWeights[i][j] = randByte(CH2_PRNG) - 128;
	}
	for (int i = 0; i < nnue.outputWeights.size(); i++) nnue.outputWeights[i] = randByte(CH2_PRNG) - 128;
	return nnue;
}

NNUE crossbreed(NNUE a, NNUE b) {
	NNUE x;
	for (int i = 0; i < x.hiddenWeights.size(); i++) {
		for (int j = 0; j < x.hiddenWeights[i].size(); j++) x.hiddenWeights[i][j] = randBool(CH2_PRNG) ? a.hiddenWeights[i][j] : b.hiddenWeights[i][j];
	}
	for (int i = 0; i < x.outputWeights.size(); i++) x.outputWeights[i] = randBool(CH2_PRNG) ? a.outputWeights[i] : b.outputWeights[i];
	return x;
}

NNUE mutate(NNUE n, double threshold = 0.005) {
	NNUE res = n;
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