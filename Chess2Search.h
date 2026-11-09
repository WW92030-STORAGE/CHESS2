

#ifndef CHESS2_SEARCH_H_NORMALEXISTING
#define CHESS2_SEARCH_H_NORMALEXISTING

#include "Chess2.h"
#include "Chess2Evaluation.h"

// Base class of a search system. The required fields are the Evaluation*, and a function search(ChessPosition& pos, int depth) --> ChessMove.
struct SearchEngine {
	Evaluation* eval = 0; // not owned by Chess2Search

	SearchEngine(Evaluation* e = 0) { eval = e; }

	float evaluate(ChessPosition& pos) {
		if (!eval) return 0;
		return eval->evaluate(pos);
	}

	virtual std::pair<ChessMove, eval_t> maxi(ChessPosition& pos, int maxDepth = 1, int depth = 0) {
		if (depth >= maxDepth) {
			return {ChessMove(), evaluate(pos)};
		}
		if (pos.noLegalMoves()) {
			if (pos.isInCheck()) return {ChessMove(), CHECKMT + depth};
			return {ChessMove(), 0};
		}
		ChessMove res;
		eval_t maxScore = -CH2_INF;
		std::vector<ChessMove> moves = pos.possibleLegalMoves(pos.sideToMove());
		std::shuffle(moves.begin(), moves.end(), CH2_PRNG);
		for (ChessMove x : moves) {
			if (!res.move) res = x;
			ChessPosition p2 = pos;
			if (p2.apply(x)) {
				p2.setSTM(!p2.sideToMove());
				auto p = mini(p2, maxDepth, depth + 1);
				if (p.second > maxScore) {
					maxScore = p.second;
					res = x;
				}
			}
		}
		return {res, maxScore};
	} 

	virtual std::pair<ChessMove, eval_t> mini(ChessPosition& pos, int maxDepth = 1, int depth = 0) {
		if (depth >= maxDepth) {
			return {ChessMove(), -evaluate(pos)};
		}
		if (pos.noLegalMoves()) {
			if (pos.isInCheck()) return {ChessMove(), -CHECKMT - depth};
			return {ChessMove(), 0};
		}
		ChessMove res;
		eval_t minScore = CH2_INF;
		std::vector<ChessMove> moves = pos.possibleLegalMoves(pos.sideToMove());
		std::shuffle(moves.begin(), moves.end(), CH2_PRNG);
		for (ChessMove x : moves) {
			if (!res.move) res = x;
			ChessPosition p2 = pos;
			if (p2.apply(x)) {
				p2.setSTM(!p2.sideToMove());
				auto p = maxi(p2, maxDepth, depth + 1);
				if (p.second < minScore) {
					minScore = p.second;
					res = x;
				}
			}
		}
		return {res, minScore};
	} 

	virtual ChessMove search(ChessPosition& pos, int depth = 1) {
		return maxi(pos, depth, 0).first;
	}
};

struct AlphaBetaEngine : SearchEngine {
	AlphaBetaEngine(Evaluation* e = 0) : SearchEngine(e) {  }

	virtual std::pair<ChessMove, eval_t> maxi(ChessPosition& pos, int maxDepth = 1, int depth = 0, eval_t alpha = -CH2_INF, eval_t beta = CH2_INF) {
		if (depth >= maxDepth) {
			return {ChessMove(), evaluate(pos)};
		}
		if (pos.noLegalMoves()) {
			if (pos.isInCheck()) return {ChessMove(), CHECKMT + depth};
			return {ChessMove(), 0};
		}
		ChessMove res;
		eval_t maxScore = -CH2_INF;
		std::vector<ChessMove> moves = pos.possibleLegalMoves(pos.sideToMove());
		std::shuffle(moves.begin(), moves.end(), CH2_PRNG);
		for (ChessMove x : moves) {
			if (!res.move) res = x;
			ChessPosition p2 = pos;
			if (p2.apply(x)) {
				p2.setSTM(!p2.sideToMove());
				auto p = mini(p2, maxDepth, depth + 1, alpha, beta);
				if (p.second > maxScore) {
					maxScore = p.second;
					res = x;
					if (p.second > alpha) alpha = p.second;
				}
				if (p.second >= beta) break;
			}
		}
		// cout << "depth " << depth << " max: " << maxScore << " " << res.to_string() << " [" << pos.possibleLegalMoves(pos.sideToMove()).size() << "]\n";
		return {res, maxScore};
	} 

	virtual std::pair<ChessMove, eval_t> mini(ChessPosition& pos, int maxDepth = 1, int depth = 0, eval_t alpha = -CH2_INF, eval_t beta = CH2_INF) {
		if (depth >= maxDepth) {
			return {ChessMove(), -evaluate(pos)};
		}
		if (pos.noLegalMoves()) {
			if (pos.isInCheck()) return {ChessMove(), -CHECKMT - depth};
			return {ChessMove(), 0};
		}
		ChessMove res;
		eval_t minScore = CH2_INF;
		std::vector<ChessMove> moves = pos.possibleLegalMoves(pos.sideToMove());
		std::shuffle(moves.begin(), moves.end(), CH2_PRNG);
		for (ChessMove x : moves) {
			if (!res.move) res = x;
			ChessPosition p2 = pos;
			if (p2.apply(x)) {
				p2.setSTM(!p2.sideToMove());
				auto p = maxi(p2, maxDepth, depth + 1, alpha, beta);
				if (p.second < minScore) {
					minScore = p.second;
					res = x;
					if (p.second < beta) beta = p.second;
				}
				if (p.second <= alpha) break;
			}
		}
		// cout << "depth " << depth << " min: " << minScore << " " << res.to_string() << " [" << pos.possibleLegalMoves(pos.sideToMove()).size() << "]\n";
		return {res, minScore};
	} 

	virtual ChessMove search(ChessPosition& pos, int depth = 1) {
		return maxi(pos, depth).first;
	}
};

#endif