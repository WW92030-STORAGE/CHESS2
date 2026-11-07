#ifndef CHESS2_BBBLSLSSL_NORMALEXISTING
#define CHESS2_BBBLSLSSL_NORMALEXISTING

#include <bits/stdc++.h>
using namespace std;
#include "Chess2.h"

void testenvironment() {
	ChessMove cm(getIndex("d2"), getIndex("d4"), 1);
	cout << cm.to_string() << endl;

	uint64_t aBitboard = bisectorBB[2];
	cout << u64togrid(aBitboard) << endl;

	ChessPosition pos;
	pos.clear();
	pos.white[2] |= 1ULL<<38;
	pos.black[3] |= 1ULL<<11;

	cout << pos.to_string() << "\n";
	printMoves(pos, 0);
	printMoves(pos, 1);

	ChessMove move(38, 20, 0);
	cout << pos.printMove(move);
}

void teststart() {
	ChessPosition pos;
	pos.init();
	cout << pos.to_string() << "\n";
	printMoves(pos, 0);
	printMoves(pos, 1);
}

void testpromotion() {
	ChessPosition pos;
	pos.clear();
	pos.white[0] |= 1ULL<<51;
	pos.black[1] |= 1ULL<<58;
	cout << pos.to_string() << "\n";
	printMoves(pos, 0);
	printMoves(pos, 1);
}

void testEnPassant() {
	ChessPosition pos;
	pos.clear();
	pos.white[0] |= 1ULL<<36;
	pos.black[0] |= 1ULL<<51;
	pos.setSTM(1);
	ChessMove move(51, 51 - 16, 0);
	cout << pos.isLegalMove(move) << endl;
	pos.apply(move);
	pos.setSTM(0);
	cout << pos.to_string() << "\n";
	printMoves(pos, 0);
	printMoves(pos, 1);
}

void testCastling() {
	ChessPosition pos;
	pos.clear();
	pos.white[5] |= 1ULL<<4;
	pos.black[5] |= 1ULL<<60;
	pos.white[3] |= 1ULL<<0 | 1ULL<<7;
	pos.black[3] |= 1ULL<<56 | 1ULL<<63;
	pos.setSTM(1);
	ChessMove move(63, 7, 0);
	cout << pos.isLegalMove(move) << endl;
	cout << move.to_string() << "\n";
	pos.apply(move);
	pos.setSTM(0);
	cout << pos.to_string() << "\n";
	printMoves(pos, 0);
	printMoves(pos, 1);
}

void testAttackedBy() {
	ChessPosition pos;
	pos.clear();
	pos.white[0] |= 1ULL<<27;
	pos.black[0] |= 1ULL<<43;
	pos.black[1] |= 1ULL<<26;
	pos.black[4] |= 1ULL<<63;
	pos.black[2] |= 1ULL<<57;
	pos.black[5] |= 1ULL<<28;


	cout << u64togrid(pos.isAttackedBy(36, 1));
}

void testCheck() {
	ChessPosition pos;
	pos.clear();
	pos.white[5] |= 1ULL<<4;
	pos.black[5] |= 1ULL<<60;
	pos.white[3] |= 1ULL<<0 | 1ULL<<7;
	pos.black[3] |= 1ULL<<56 | 1ULL<<63;
	pos.black[2] |= 1ULL<<23;
	pos.setSTM(0);
	cout << pos.to_string() << "\n";
	printMoves(pos, 0);
	printMoves(pos, 1);
}

void testActualPromotion() {
	ChessPosition pos;
	pos.clear();
	pos.white[0] |= 1ULL<<51;
	pos.white[0] |= 1ULL<<37;
	pos.black[1] |= 1ULL<<58;
	cout << pos.to_string() << "\n";
	printMoves(pos, 0);
	printMoves(pos, 1);

	ChessMove move(51, 58, 2); // make me a rook
	// move = ChessMove(37, 45, 0);
	pos.isLegalMove(move);
	cout << "MOVE: " << move.to_string() << "\n";
	pos.apply(move);
	cout << pos.to_string() << "\n";
}

#endif