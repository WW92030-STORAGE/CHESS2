#ifndef CHESS2_GAMEUTILS_NORMALEXISTING
#define CHESS2_GAMEUTILS_NORMALEXISTING

#include "Chess2.h"
#include "ChessUtils.h"
#include "Chess2Evaluation.h"
#include "Chess2Search.h"
#include <iostream>

// UTILS

void printMoves(ChessPosition& pos, bool side = 0, int cols = 3) {
	if (cols < 0) cols = 1;
	std::vector<ChessMove> pm = pos.possibleLegalMoves(side);
	std::cout << (side ? "Black" : "White") << " possible moves: [" << std::to_string(pm.size()) << "]\n";
	int ctr = 0;
	for (auto x : pm) {
		cout << x.to_string() << " ";
		if (++ctr % cols == 0) std::cout << "\n";
	}
	if (ctr % cols) std::cout << "\n";
}

void printMovesBySquare(ChessPosition& pos, bool side = 0, int cols = 3, uint8_t sq = 0) {
	if (cols < 0) cols = 1;
	std::vector<ChessMove> pm = pos.possibleLegalMovesPerSquare(side, sq);
	std::cout << (side ? "Black" : "White") << " possible moves from square " << coords(sq) << ": [" << std::to_string(pm.size()) << "]\n";
	int ctr = 0;
	for (auto x : pm) {
		cout << x.to_string() << " ";
		if (++ctr % cols == 0) std::cout << "\n";
	}
	if (ctr % cols) std::cout << "\n";
}

void printMovesWithEvals(ChessPosition& pos, Evaluation& eval, bool side = 0, int cols = 3) {
	if (cols < 0) cols = 1;
	std::vector<ChessMove> pm = pos.possibleLegalMoves(side);
	std::cout << (side ? "Black" : "White") << " possible moves: [" << std::to_string(pm.size()) << "]\n";
	int ctr = 0;
	for (auto x : pm) {
		ChessPosition newPos = pos;
		newPos.apply(x);
		newPos.setSTM(!newPos.sideToMove());
		cout << x.to_string() << ":" << eval(newPos) << " ";
		if (++ctr % cols == 0) std::cout << "\n";
	}
	if (ctr % cols) std::cout << "\n";
}

// END UTILS

// BEGIN PLAYGAME

void playGame() {
	ChessPosition pos;
	pos.init();
	while (true) {
		std::cout << pos.to_string() << "\n";
		if (pos.checkmated()) {
			cout << "CHECKMATE - Game end. Player " << !pos.sideToMove() << " WINS\n";
			break;
		}
		int r0, r1, c0, c1;
		std::cin >> c0 >> r0 >> c1 >> r1;
		if (c0 == -1) {
			printMoves(pos, pos.sideToMove(), std::max(r0, 1));
			continue;
		}
		if (c0 == -2) {
			printMovesBySquare(pos, pos.sideToMove(), std::max(r0, 1), (r1<<3) | c1);
			continue;
		}
		ChessMove move((r0<<3)|c0, (r1<<3)|c1, 0);
		if (!pos.isLegalMove(move)) {
			std::cout << "Invalid move\n";
			continue;
		}
		if (move.move & PROMOTION) {
			int xx;
			std::cout << "PROMOTE [0=N, 1=B, 2=R, 3=Q]: ";
			std::cin >> xx;
			std::cout << "\n";
			move.move |= xx<<12;
		}
		if (pos.apply(move)) pos.setSTM(!pos.sideToMove());
		else std::cout << "Error move\n";
		continue;
	}
}

void playGameWithEvaluation(Evaluation& eval) {
	ChessPosition pos;
	pos.init();
	while (true) {
		std::cout << pos.to_string() << "\n";
		std::cout << "EVALUATION: " << eval(pos) << "\n";
		if (pos.checkmated()) {
			cout << "CHECKMATE - Game end. Player " << !pos.sideToMove() << " WINS\n";
			break;
		}
		int r0, r1, c0, c1;
		std::cin >> c0 >> r0 >> c1 >> r1;
		if (c0 == -1) {
			printMovesWithEvals(pos, eval, pos.sideToMove(), std::max(r0, 1));
			continue;
		}
		if (c0 == -2) {
			printMovesBySquare(pos, pos.sideToMove(), std::max(r0, 1), (r1<<3) | c1);
			continue;
		}
		ChessMove move((r0<<3)|c0, (r1<<3)|c1, 0);
		if (!pos.isLegalMove(move)) {
			std::cout << "Invalid move\n";
			continue;
		}
		if (move.move & PROMOTION) {
			int xx;
			std::cout << "PROMOTE [0=N, 1=B, 2=R, 3=Q]: ";
			std::cin >> xx;
			std::cout << "\n";
			move.move |= xx<<12;
		}
		if (pos.apply(move)) pos.setSTM(!pos.sideToMove());
		else std::cout << "Error move\n";
		continue;
	}
}

void playAgainstEngine(SearchEngine& engine, bool enginePlaysAs = 1) {
	ChessPosition pos;
	pos.init();
	while (true) {
		std::cout << pos.to_string() << "\n";
		if (pos.checkmated()) {
			cout << "CHECKMATE - Game end. Player " << !pos.sideToMove() << " WINS\n";
			break;
		}
		ChessMove move;
		if (pos.sideToMove() == enginePlaysAs) {
			move = engine.search(pos, 3);
			std::cout << "Chosen move: " << move.to_string() << "\n";
		} else {
			int r0, r1, c0, c1;
			std::cin >> c0 >> r0 >> c1 >> r1;
			if (c0 == -1) {
				printMoves(pos, pos.sideToMove(), std::max(r0, 1));
				continue;
			}
			if (c0 == -2) {
				printMovesBySquare(pos, pos.sideToMove(), std::max(r0, 1), (r1<<3) | c1);
				continue;
			}
			move = ChessMove((r0<<3)|c0, (r1<<3)|c1, 0);
			if (!pos.isLegalMove(move)) {
				std::cout << "Invalid move\n";
				continue;
			}
			if (move.move & PROMOTION) {
				int xx;
				std::cout << "PROMOTE [0=N, 1=B, 2=R, 3=Q]: ";
				std::cin >> xx;
				std::cout << "\n";
				move.move |= xx<<12;
			}
		}
		if (pos.apply(move)) pos.setSTM(!pos.sideToMove());
		else {
			std::cout << "Error move\n";
			break;
		}
		continue;
	}
}

/*

A sequence of moves from a hypothetical very low rated chess game that creates a variety of scenarios admitting edge case testing:
4 1 4 3
4 6 4 4
6 0 5 2
3 6 3 4
4 3 3 4
5 7 1 3

Fool's mate
5 1 5 3
4 6 4 4
6 1 6 3
3 7 7 3

Scholar's mate
4 1 4 3
4 6 4 4
3 0 7 4
6 7 5 5
5 0 2 3
1 7 2 5
7 4 5 6 

Check
4 1 4 3
3 6 3 4
5 0 1 4

En passant
4 1 4 3
3 6 3 4
4 3 4 4
5 6 5 4
4 4 5 5

// Missile gambit
4 1 4 3
3 6 3 4
6 0 5 2
3 4 4 3
5 2 6 4
6 7 5 5
3 1 3 2
4 3 3 2
5 0 3 2
7 6 7 5
6 4 5 6
4 7 5 6
3 2 6 5
5 6 6 5

*/

#endif