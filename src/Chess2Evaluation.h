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

#endif