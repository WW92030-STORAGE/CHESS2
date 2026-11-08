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

struct AlphaBetaEngine : SearchEngine {
	AlphaBetaEngine(Evaluation* e = 0) : SearchEngine(e) {  }

	std::pair<ChessMove, float> maxi(ChessPosition& pos, int depth = 1, float alpha = -CH2_INF, float beta = CH2_INF) {
		if (depth <= 0) return {ChessMove(), evaluate(pos)};
		ChessMove res;
		float maxScore = -CH2_INF;
		for (ChessMove x : pos.possibleLegalMoves(pos.sideToMove())) {
			ChessPosition p2 = pos;
			if (p2.apply(x)) {
				p2.setSTM(!p2.sideToMove());
				auto p = mini(p2, depth - 1, alpha, beta);
				if (p.second > maxScore) {
					maxScore = p.second;
					res = x;
					if (p.second > alpha) alpha = p.second;
				}
				if (p.second > beta) break;
			}
		}
		return {res, maxScore};
	} 

	std::pair<ChessMove, float> mini(ChessPosition& pos, int depth = 1, float alpha = -CH2_INF, float beta = CH2_INF) {
		if (depth <= 0) return {ChessMove(), evaluate(pos)};
		ChessMove res;
		float minScore = CH2_INF;
		for (ChessMove x : pos.possibleLegalMoves(pos.sideToMove())) {
			ChessPosition p2 = pos;
			if (p2.apply(x)) {
				p2.setSTM(!p2.sideToMove());
				auto p = maxi(p2, depth - 1, alpha, beta);
				if (p.second < minScore) {
					minScore = p.second;
					res = x;
					if (p.second < beta) beta = p.second;
				}
				if (p.second < alpha) break;
			}
		}
		return {res, minScore};
	} 

	virtual ChessMove search(ChessPosition& pos, int depth = 1) {
		return maxi(pos, depth).first;
	}
};

#endif