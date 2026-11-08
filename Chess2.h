#ifndef CHESS2_H_NORMALEXISTING
#define CHESS2_H_NORMALEXISTING

#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>

#include "ChessUtils.h"

// ChessMove and ChessPosition do not contain any pointers so direct assignment and copy is valid here (in particular, the data contents of ChessMove is a single uint16_t)
// ChessPosition pos2 = pos;

// flags (4 bits), dest(6 bits), source(6 bits)
struct ChessMove {
	uint16_t move = 0;
	inline int8_t flags() { return move>>12; }
	inline int8_t source() { return move & 63; }
	inline int8_t dest() { return (move>>6) & 63; }
	inline bool capture() { return move & CAPTURE; }
	inline bool pawnPushed() { return flags() == 1; }
	inline bool enPassant() { return flags() == 5; }
	inline void setFlags(uint8_t x) { move = (move & ((1<<12) - 1)) | (x<<12); }
	inline bool castle() { return move == 2 || move == 3; }

	ChessMove() { }
	ChessMove(int8_t s, int8_t d, uint8_t flags) { move = (uint16_t)(s) | ((uint16_t)(d)<<6) | ((uint16_t)(flags)<<12); }

	bool operator<(const ChessMove& other) const {
		return move < other.move;
	}

	std::string flagStr() {
		return "[P: " + std::to_string(bool(move & PROMOTION))
		+ ", C: " + std::to_string(bool(move & CAPTURE))
		+ ", F: " + std::to_string(bool(move & FLAG1)) + std::to_string(bool(move & FLAG0)) + "]";
	}

	std::string to_string() {
		if (!move) return "NIL";
		return "ChessMove[" + coords(source()) + " > " + coords(dest()) + ": " + flagStr() + "]";
	}
};

struct ChessPosition {
	// pawns, knights, bishops, rooks, queens, kings
	uint64_t black[6] = {0, 0, 0, 0, 0, 0};
	uint64_t white[6] = {0, 0, 0, 0, 0, 0};
	ChessMove lastMove;
	ChessMove lastMove2;
	uint16_t hmc = 0;

	// x, x, x, black right castle, black left castle, white right castle, white left castle, side to move
	uint8_t data = BRCASTLE | BLCASTLE | WRCASTLE | WLCASTLE;
	uint8_t moveCounter = 0;

	inline bool sideToMove() { return data & 1; }
	inline void setSTM(bool value) { data = (data & 254) | value; }

	void clear() {
		for (int i = 0; i < 6; i++) black[i] = white[i] = 0;
		data = BRCASTLE | BLCASTLE | WRCASTLE | WLCASTLE;
		lastMove.move = 0;
		lastMove2.move = 0;
	}

	void init() {
		clear();
		white[0] = rankBB[1];
		black[0] = rankBB[6];
		white[1] = (1ULL<<1) | (1ULL<<6);
		black[1] = (1ULL<<57) | (1ULL<<62);
		white[2] = (1ULL<<2) | (1ULL<<5);
		black[2] = (1ULL<<58) | (1ULL<<61);
		white[3] = (1ULL<<0) | (1ULL<<7);
		black[3] = (1ULL<<56) | (1ULL<<63);
		white[4] = 1ULL<<3;
		black[4] = 1ULL<<59;
		white[5] = 1ULL<<4;
		black[5] = 1ULL<<60;
	}

	uint64_t getAllPieces(bool color) {
		if (color) return black[0] | black[1] | black[2] | black[3] | black[4] | black[5];
		return white[0] | white[1] | white[2] | white[3] | white[4] | white[5];
	}

	// what squares are x attacked by? (this does not include en passants)
	uint64_t isAttackedBy(int8_t s, bool sta) {
		uint64_t occ = getAllPieces(0) | getAllPieces(1);
		uint64_t res = 0;
		// pawns
		if (sta) { // black attacks from above
			if (s < 56) res |= (black[0] & (onehot(s + 7) | onehot(s + 9)));
		} else if (s > 7) res |= (white[0] & (onehot(s - 7) | onehot(s - 9)));
		// knights
		uint64_t knightField = knightBB[s];
		res |= (sta ? black : white)[1] & knightField;

		// bishops
		int8_t sum = (s>>3) + (s & 7);
		int8_t diff = (s>>3) - (s & 7);
		uint64_t diagonals = rayAttack(diagonalBB(diff), occ, s, 0) | rayAttack(diagonalBB(diff), occ, s, 1) | rayAttack(bisectorBB[sum], occ, s, 0) | rayAttack(bisectorBB[sum], occ, s, 1);
		res |= (sta ? black : white)[2] & diagonals;

		// rooks
		uint64_t cardinals = rayAttack(rankBB[s>>3], occ, s, 0) | rayAttack(rankBB[s>>3], occ, s, 1) | rayAttack(fileBB[s & 7], occ, s, 0) | rayAttack(fileBB[s & 7], occ, s, 1);
		res |= (sta ? black : white)[3] & cardinals;

		// queens
		res |= (sta ? black : white)[4] & (diagonals | cardinals);

		// kings
		res |= (sta ? black : white)[5] & kingBB(s);

		return res;
	}

	// is it being attacked? (this does not include en passants)
	bool isBeingAttacked(int8_t s, bool sta) {
		uint64_t occ = getAllPieces(0) | getAllPieces(1);
		// pawns
		if (sta) { // black attacks from above
			if (s < 56 && (black[0] & (onehot(s + 7) | onehot(s + 9)))) return true;
		} else if (s > 7 && (white[0] & (onehot(s - 7) | onehot(s - 9)))) return true;
		// knights
		uint64_t knightField = knightBB[s];
		if ((sta ? black : white)[1] & knightField) return true;

		// bishops
		int8_t sum = (s>>3) + (s & 7);
		int8_t diff = (s>>3) - (s & 7);
		uint64_t diagonals = rayAttack(diagonalBB(diff), occ, s, 0) | rayAttack(diagonalBB(diff), occ, s, 1) | rayAttack(bisectorBB[sum], occ, s, 0) | rayAttack(bisectorBB[sum], occ, s, 1);
		if ((sta ? black : white)[2] & diagonals) return true;

		// rooks
		uint64_t cardinals = rayAttack(rankBB[s>>3], occ, s, 0) | rayAttack(rankBB[s>>3], occ, s, 1) | rayAttack(fileBB[s & 7], occ, s, 0) | rayAttack(fileBB[s & 7], occ, s, 1);
		if ((sta ? black : white)[3] & cardinals) return true;

		// queens
		if ((sta ? black : white)[4] & (diagonals | cardinals)) return true;

		// kings
		if ((sta ? black : white)[5] & kingBB(s)) return true;

		return false;
	}

	// Returns if the move is pseudolegal, and tries to correct the flags. (Be aware that an illegal move might still have flags changed.)
	bool isPseudoLegalMove(ChessMove& move) {
		bool stm = sideToMove();
		uint64_t blacks = getAllPieces(1);
		uint64_t whites = getAllPieces(0);
		uint64_t allPieces = blacks | whites;

		uint64_t player = (stm ? blacks : whites);
		uint64_t opponent = (stm ? whites : blacks);

		int8_t s = move.source();
		int8_t d = move.dest();
		uint64_t sSquare = onehot(s);
		uint64_t dSquare = onehot(d);
		// 1. are source valid?
		if (!(sSquare & player)) return false;

		// are dest valid?
		if (dSquare & player || dSquare & (white[5] | black[5])) return false; // do not move into your own side or a king
		bool capture = dSquare & opponent;
		if (capture) move.move |= CAPTURE;

		if (sSquare & (stm ? black : white)[0]) { // pawns
			if (stm) {
				if (d < 8) move.move |= PROMOTION;
				if (sSquare & rankBB[0]) return false;
				bool intermediateActive = allPieces & onehot(s - 8);
				if (d == s - 8 && !intermediateActive) return true;
				if ((sSquare & rankBB[6]) && d == s - 16 && !intermediateActive && !(allPieces & onehot(s - 16))) {
					move.move |= FLAG0;
					return true;
				}
				if ((!(sSquare & fileBB[0]) && d == s - 9) || (!(sSquare & fileBB[7]) && d == s - 7)) {
					if (capture) return true;
					// en passant
					if (lastMove.pawnPushed()) {
						int8_t ld = lastMove.dest();
						if ((ld == s + 1 && d == s - 7) || (ld == s - 1 && d == s - 9)) {
							move.move |= (CAPTURE | FLAG0);
							return true;
						}
					}
				}
			} else {
				if (d > 56) move.move |= PROMOTION;
				if (sSquare & rankBB[7]) return false;
				bool intermediateActive = allPieces & onehot(s + 8);
				if (d == s + 8 && !intermediateActive) return true;
				if ((sSquare & rankBB[1]) && d == s + 16 && !intermediateActive && !(allPieces & onehot(s + 16))) {
					move.move |= FLAG0;
					return true;
				}
				if ((!(sSquare & fileBB[0]) && d == s + 7) || (!(sSquare & fileBB[7]) && d == s + 9))  {
					if (capture) return true;
					// en passant
					if (lastMove.pawnPushed()) {
						int8_t ld = lastMove.dest();
						if ((ld == s + 1 && d == s + 9) || (ld == s - 1 && d == s + 7)) {
							move.move |= (CAPTURE | FLAG0);
							return true;
						}
					}
				}
			}
		} if (sSquare & (stm ? black : white)[1]) { // knights
			if (dSquare & knightBB[s]) return true;
		} if (sSquare & (stm ? black : white)[2] || sSquare & (stm ? black : white)[4]) { // bishops/queens
			int sum = (s>>3) + (s & 7);
			int diff = (s>>3) - (s & 7);

			// time for raycasting!
			if (dSquare & diagonalBB(diff)) {
				uint64_t line = rayAttack(diagonalBB(diff), allPieces, s, d > s);
				if (line & dSquare) return true;
			} else if (dSquare & bisectorBB[sum]) {
				uint64_t line = rayAttack(bisectorBB[sum], allPieces, s, d > s);
				if (line & dSquare) return true;
			}
		} if (sSquare & (stm ? black : white)[3] || sSquare & (stm ? black : white)[4]) { // rooks/queens
			if ((s>>3) == (d>>3)) {
				uint64_t line = rayAttack(rankBB[s>>3], allPieces, s, d > s);
				if (line & dSquare) return true;
			} else if ((s&7) == (d&7)) {
				uint64_t line = rayAttack(fileBB[s&7], allPieces, s, d > s);
				if (line & dSquare) return true;
			}
		} if (sSquare & (stm ? black : white)[5]) {
			// castling rules
			if (d == s + 2 && data & (stm ? BRCASTLE : WRCASTLE)) {
				if (!(allPieces & (onehot(s + 1) | onehot(s + 2)))) {
					move.move |= 2<<12;
					return true;
				}
			} else if (d == s - 2 && data & (stm ? BLCASTLE : WLCASTLE)) {
				if (!(allPieces & (onehot(s - 1) | onehot(s - 2) | onehot(s - 3)))) {
					move.move |= 3<<12;
					return true;
				}
			}
			bool nL = !(sSquare & fileBB[0]);
			bool nR = !(sSquare & fileBB[7]);
			bool nU = !(sSquare & rankBB[7]);
			bool nD = !(sSquare & rankBB[0]);
			if (
				(nL && (d == s - 1 || (nU && d == s + 7) || (nD && d == s - 9))) ||
				(nR && (d == s + 1 || (nU && d == s + 9) || (nD && d == s - 7))) || 
				(nU && d == s + 8) || (nD && d == s - 8)
			) return true;

		}
		return false;
	}

	bool isInCheck() {
		return (isBeingAttacked(bitScanForward((sideToMove() ? black : white)[5]), !sideToMove()));
	}

	bool noLegalMoves() {
		return possibleLegalMoves(sideToMove()).size() == 0;
	}

	bool checkmated() {
		return isInCheck() && noLegalMoves();
	}

	bool forcedStalemate() {
		return hmc >= 150;
	}

	bool canCastle() {
		return data & (sideToMove() ? (BRCASTLE | BLCASTLE) : (WRCASTLE | WLCASTLE));
	}

	// a stalemate happens in !isInCheck() && noLegalMoves()

	bool isLegalMove(ChessMove& move) {
		if (!isPseudoLegalMove(move)) return false;
		ChessPosition next = *this;
		next.apply(move);
		if (next.isInCheck()) return false;

		int8_t f = move.flags();
		if (f == 2) {
			if (isInCheck()) return false;
			ChessPosition next2 = *this;
			ChessMove move2(move.source(), move.source() + 1, 0);
			next2.apply(move2);
			if (next2.isInCheck()) return false;
		}
		else if (f == 3) {
			if (isInCheck()) return false; 
			ChessPosition next2 = *this;
			ChessMove move2(move.source(), move.source() - 1, 0);
			next2.apply(move2);
			if (next2.isInCheck()) return false;
		}
		return true;
	}

	std::vector<ChessMove> possiblePseudoLegals(bool stm) {
		bool originalSTM = sideToMove();
		setSTM(stm);

		uint64_t blacks = getAllPieces(1);
		uint64_t whites = getAllPieces(0);
		std::vector<ChessMove> moves;
		for (int8_t i = 0; i < 64; i++) {
			for (int8_t j = 0; j < 64; j++) {
				if (i == j) continue;
				if (!(stm ? blacks : whites) & onehot(i)) continue;
				ChessMove move(i, j, 0);
				if (isPseudoLegalMove(move)) {
					moves.push_back(move);
					if (move.move & PROMOTION) {
						for (int i = 0; i < 3; i++) {
							move.move += FLAG0;
							moves.push_back(move);
						}
					}
				}
			}
		}
		setSTM(originalSTM);

		return moves;
	}

	std::vector<ChessMove> possibleLegalMoves(bool stm) {
		bool originalSTM = sideToMove();
		setSTM(stm);

		uint64_t blacks = getAllPieces(1);
		uint64_t whites = getAllPieces(0);
		std::vector<ChessMove> moves;
		for (int8_t i = 0; i < 64; i++) {
			if (!(stm ? blacks : whites) & onehot(i)) continue;
			for (int8_t j = 0; j < 64; j++) {
				if (i == j) continue;
				ChessMove move(i, j, 0);
				if (isLegalMove(move)) {
					moves.push_back(move);
					if (move.move & PROMOTION) {
						for (int i = 0; i < 3; i++) {
							move.move += FLAG0;
							moves.push_back(move);
						}
					}
				}
			}
		}
		setSTM(originalSTM);

		return moves;
	}

	std::vector<ChessMove> possibleLegalMovesPerSquare(bool stm, int8_t i) {
		bool originalSTM = sideToMove();
		std::vector<ChessMove> moves;
	
		uint64_t blacks = getAllPieces(1);
		uint64_t whites = getAllPieces(0);
		if (!(stm ? blacks : whites) & onehot(i)) return moves;

		setSTM(stm);

		for (int8_t j = 0; j < 64; j++) {
				if (i == j) continue;
				ChessMove move(i, j, 0);
				if (isLegalMove(move)) {
					moves.push_back(move);
					if (move.move & PROMOTION) {
						for (int i = 0; i < 3; i++) {
							move.move += FLAG0;
							moves.push_back(move);
						}
					}
				}
		}
		setSTM(originalSTM);

		return moves;
	}

	bool apply(ChessMove& move) {
		bool stm = sideToMove();
		int8_t s = move.source();
		int8_t d = move.dest();
		uint64_t sSquare = onehot(s);
		uint64_t dSquare = onehot(d);
		int8_t sPiece = -1;
		int8_t dPiece = -1;
		bool pawnMoved = false;
		for (int i = 0; i < 6; i++) {
			uint64_t blob = (stm ? black : white)[i];
			if (blob & sSquare) {
				if (i == 0) pawnMoved = true;
				sPiece = i;
				break;
			}
		}
		if (sPiece == -1) return 0;
		// erase the original
		(stm ? black : white)[sPiece] &= ~sSquare;
		// erase if captured
		if (move.capture()) {
			for (int i = 0; i < 6; i++) {
				uint64_t blob = (stm ? white : black)[i];
				if (blob & dSquare) {
					dPiece = i;
					break;
				}
			}
			if (dPiece >= 0) (stm ? white : black)[dPiece] &= ~dSquare;
		}
		// promotion
		if (move.move & PROMOTION) {
			(stm ? black : white)[1 + (move.flags() & 3)] |= dSquare;
		}
		// happy path (no promotion)
		else (stm ? black : white)[sPiece] |= dSquare;

		// castling rules
		int8_t flags = move.flags();
		if (flags == 5) {
			if (d == s + 9) (stm ? white : black)[0] &= ~onehot(s + stm ? 1 : -1);
			if (d == s + 7) (stm ? white : black)[0] &= ~onehot(s + stm ? -1 : 1);
		}
		if (flags == 2) {
			(stm ? black : white)[3] &= ~onehot(s + 3);
			(stm ? black : white)[3] |= onehot(s + 1);
		} else if (flags == 3) {
			(stm ? black : white)[3] &= ~onehot(s - 4);
			(stm ? black : white)[3] |= onehot(s - 1);
		}

		// set flags
		lastMove2 = lastMove;
		lastMove = move;
		if (s == 0 || d == 0 || s == 4 || d == 4) data &= ~(WLCASTLE);
		if (s == 7 || d == 7 || s == 4 || d == 4) data &= ~(WRCASTLE);
		if (s == 56 || d == 56 || s == 60 || d == 60) data &= ~(BLCASTLE);
		if (s == 63 || d == 63 || s == 60 || d == 60) data &= ~(BRCASTLE);

		// update counter
		if (pawnMoved || move.capture()) hmc = 0;
		else hmc++;

		return 1;
	}

	std::string to_string() {
		std::string res = "ChessPosition[" + u8tos(data) + ", " + lastMove.to_string() + ", hmc=" + std::to_string(hmc) + "]";
		for (int r = 8; r--; ) {
			res.push_back('\n');
			for (int c = 0; c < 8; c++) {
				int8_t index = (r<<3) | c;
				uint64_t square = onehot(index);
				std::string piece = "..";
				for (int i = 0; i < 6; i++) {
					if (white[i] & square) {
						piece = pieceName(WHITE | (1<<(i + 2)));
						break;
					} else if (black[i] & square) {
						piece = pieceName(BLACK | (1<<(i + 2)));
						break;
					}
				}
				res += piece;
				res.push_back(' ');
			}
		}
		return res;
	}

	std::string printMove(ChessMove& move) {
		if (!isLegalMove(move)) return "";
		std::string res = "ChessPosition[" + u8tos(data) + ", " + lastMove.to_string() + "]" + "\nMove: " + move.to_string();
		for (int r = 8; r--; ) {
			res.push_back('\n');
			for (int c = 0; c < 8; c++) {
				int8_t index = (r<<3) | c;
				std::string piece = "..";
				if (index == move.source()) piece = "[]";
				else {
					if (index == move.dest()) index = move.source();
					uint64_t square = onehot(index);
				
					for (int i = 0; i < 6; i++) {
						if (white[i] & square) {
							piece = pieceName(WHITE | (1<<(i + 2)));
							break;
						} else if (black[i] & square) {
							piece = pieceName(BLACK | (1<<(i + 2)));
							break;
						}
					}
				}
				res += piece;
				res.push_back(' ');
			}
		}
		return res;
	}
};

#endif
