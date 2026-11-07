#ifndef CHESS2_EVAL_H_NORMALEXISTING
#define CHESS2_EVAL_H_NORMALEXISTING

#include <climits>
#include <cfloat>

#include "Chess2.h"
#include "ChessUtils.h"

// Evaluation function

constexpr float CH2_INF = 1.0e12;

// This is the purest form of an evaluation system. Subclasses can derive the evaluation function.
// WARNING - All evaluations evaluate in absolute terms. If black holds an advantage then the evaluation scores high, otherwise the evaluation scores low.
struct Evaluation {
	// simple evaluation system based on material, checks, and checkmates

	// evaluateSingleSide computes evaluation of a single side. This function may or may not correspond exactly to the evaluate method (e.g. evaluation = ess(1) - ess(0))
	virtual float evaluateSingleSide(ChessPosition& pos, bool stm) {
		bool originalSTM = pos.sideToMove();
		pos.setSTM(stm);
		if (pos.checkmated()) return stm ? -CH2_INF : CH2_INF;
		float res = 0;
		const int weights[5] = {1, 3, 3, 5, 9};
		for (int i = 0; i < 5; i++) {
			res += weights[i] * popCount((stm ? pos.black : pos.white)[i]);
		}
		if (pos.isInCheck()) {
			if (res > 0) res *= 0.5;
			else res *= 1.5;
		}
		pos.setSTM(originalSTM);
		return res;
	}
	virtual float evaluate(ChessPosition& pos) {
		return evaluateSingleSide(pos, 1) - evaluateSingleSide(pos, 0);
	}

	float operator()(ChessPosition& pos) { return evaluate(pos); }
};

struct Turochamp : Evaluation {
	// Turing-Champernowne method, with some slight variaions
	virtual float evaluateSingleSide(ChessPosition& pos, bool stm) {
		bool originalSTM = pos.sideToMove();
		pos.setSTM(stm);
		if (pos.checkmated()) return stm ? -CH2_INF : CH2_INF;
		float res = -pos.isInCheck();
		const float weights[5] = {1, 3, 3.5, 5, 10};

		// 1. add square root of number of moves per piece except kings and pawns
		uint64_t rbn = stm ? (pos.black[1] | pos.black[2] | pos.black[3]) : (pos.white[1] | pos.white[2] | pos.white[3]);
		uint64_t notKPs = rbn | (stm ? pos.black : pos.white)[4];
		uint64_t pawns = (stm ? pos.black : pos.white)[0];
		uint64_t king = (stm ? pos.black : pos.white)[5];
		uint64_t allPieces = (notKPs | pawns | king) | pos.getAllPieces(!stm);
		for (int i = 0; i < 64; i++) {
			uint64_t sq = onehot(i);
			if (sq & notKPs) {
				int ctr = 0;
				for (auto i : pos.possibleLegalMovesPerSquare(stm, i)) ctr += i.capture() ? 2 : 1;
				res += sqrtf(ctr);

				if (sq & rbn) {
					int8_t defending = popCount(pos.isAttackedBy(i, stm));
					if (defending) res += 1 + defending * 0.5;
				}
			}
			if (sq & king) {
				std::vector<ChessMove> nocastle;
				int ctr = 0;
				bool castleImmediatelyEligible = 0;
				for (auto i : pos.possibleLegalMovesPerSquare(stm, i)) {
					if (!i.castle()) ctr += i.capture() ? 2 : 1;
					else castleImmediatelyEligible = 1;
				}
				res += sqrtf(ctr);

				// castling rules
				if (pos.canCastle()) res += castleImmediatelyEligible;

				// king safety
				int8_t diff = (i>>3) - (i & 7);
				int8_t sum = (i>>3) + (i & 7);
				uint64_t diagonals = rayAttack(diagonalBB(diff), allPieces, i, 0) | rayAttack(diagonalBB(diff), allPieces, i, 1) | rayAttack(bisectorBB[sum], allPieces, i, 0) | rayAttack(bisectorBB[sum], allPieces, i, 1);
				uint64_t orthos = rayAttack(rankBB[i>>3], allPieces, i, 0)| rayAttack(rankBB[i>>3], allPieces, i, 1) | rayAttack(fileBB[i & 7], allPieces, i, 0) | rayAttack(fileBB[i & 7], allPieces, i, 1);
				res -= popCount(diagonals | orthos);
			}
		}

		// evaluate pawns by rank
		for (int8_t i = 2; i < 7; i++) {
			int8_t r = stm ? 7 - i : i;
			res += 0.2 * (i - 1) * popCount(rankBB[r] & pawns);
		}
		pos.setSTM(originalSTM);
		return res;
	}
};

#endif