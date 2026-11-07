#ifndef CHESS2_SEARCH_H_NORMALEXISTING
#define CHESS2_SEARCH_H_NORMALEXISTING

#include "Chess2.h"
#include "ChessUtils.h"
#include "Chess2Evaluation.h"

// Base class of a search system. The required fields are the Evaluation*, and a function search(ChessPosition& pos, int depth) --> ChessMove.
struct SearchEngine {
	Evaluation* eval = 0; // not owned by Chess2Search

	SearchEngine(Evaluation* e = 0) { eval = e; }

	float evaluate(ChessPosition& pos) {
		if (!eval) return 0;
		return eval->evaluate(pos);
	}

	std::pair<ChessMove, float> maxi(ChessPosition& pos, int depth = 1) {
		if (depth <= 0) return {ChessMove(), evaluate(pos)};
		ChessMove res;
		float maxScore = -CH2_INF;
		for (ChessMove x : pos.possibleLegalMoves(pos.sideToMove())) {
			ChessPosition p2 = pos;
			if (p2.apply(x)) {
				p2.setSTM(!p2.sideToMove());
				auto p = mini(p2, depth - 1);
				if (p.second > maxScore) {
					maxScore = p.second;
					res = x;
				}
			}
		}
		return {res, maxScore};
	} 

	std::pair<ChessMove, float> mini(ChessPosition& pos, int depth = 1) {
		if (depth <= 0) return {ChessMove(), evaluate(pos)};
		ChessMove res;
		float minScore = CH2_INF;
		for (ChessMove x : pos.possibleLegalMoves(pos.sideToMove())) {
			ChessPosition p2 = pos;
			if (p2.apply(x)) {
				p2.setSTM(!p2.sideToMove());
				auto p = maxi(p2, depth - 1);
				if (p.second < minScore) {
					minScore = p.second;
					res = x;
				}
			}
		}
		return {res, minScore};
	} 

	virtual ChessMove search(ChessPosition& pos, int depth = 1) {
		return maxi(pos, depth).first;
	}
};

#endif