#ifndef CHESS2_UTILS_H_NORMALEXISTING
#define CHESS2_UTILS_H_NORMALEXISTING

#include <random>

constexpr uint8_t WHITE = 1<<0;
constexpr uint8_t BLACK = 1<<1;
constexpr uint8_t PAWN = 1<<2;
constexpr uint8_t KNIGHT = 1<<3;
constexpr uint8_t BISHOP = 1<<4;
constexpr uint8_t ROOK = 1<<5;
constexpr uint8_t QUEEN = 1<<6;
constexpr uint8_t KING = 1<<7;
constexpr uint16_t PROMOTION = 1<<15;
constexpr uint16_t CAPTURE = 1<<14;
constexpr uint16_t FLAG0 = 1<<12;
constexpr uint16_t FLAG1 = 1<<13;

constexpr uint8_t WLCASTLE = 1<<1;
constexpr uint8_t WRCASTLE = 1<<2;
constexpr uint8_t BLCASTLE = 1<<3;
constexpr uint8_t BRCASTLE = 1<<4;

std::mt19937 CH2_PRNG(0);
std::uniform_real_distribution<float> rand01(0, 1);
std::uniform_int_distribution<int> randBool(0, 1);
std::uniform_int_distribution<int> randByte(0, 255);

void initPRNG(uint64_t x) {
	CH2_PRNG = std::mt19937(x);
}

// BEGIN GENERAL BIT THINGS

std::string u8tos(uint8_t v) {
	std::string res = "xxxxxxxx";
	for (int i = 8; i--; ) {
		res[i] = '0' + (v & 1);
		v>>=1;
	}
	return res;
}

std::string u16tos(uint16_t v) {
	std::string res = "xxxxxxxxxxxxxxxx";
	for (int i = 16; i--; ) {
		res[i] = '0' + (v & 1);
		v>>=1;
	}
	return res;
}

std::string u64togrid(uint64_t v) {
	std::string rows[8];
	for (int i = 0; i < 8; i++) rows[i] = "........";
	for (int i = 0; i < 64; i++) {
		rows[i>>3][i & 7] = '0' + (v & 1);
		v>>=1;
	}
	std::string res = "";
	for (int i = 8; i--; ) {
		res += rows[i];
		if (i) res.push_back('\n');
	} 
	return res;
}

// cpw
int8_t popCount(uint64_t x) {
    constexpr uint64_t k1 = uint64_t(0x5555555555555555);
    constexpr uint64_t k2 = uint64_t(0x3333333333333333);
    constexpr uint64_t k4 = uint64_t(0x0f0f0f0f0f0f0f0f);
    x =   x             - ((x >> 1)  & k1);
    x =  (x & k2)       + ((x >> 2)  & k2);
    x = ( x             +  (x >> 4)) & k4 ;
    x = (x * uint64_t(0x0101010101010101))>> 56;
    return (int8_t) x;
}

// END GENERAL BIT THINGS

std::string pieceName(uint8_t v) {
	std::string res = "..";
	if (!v) return res;
	res[0] = (v & 2) ? 'B' : 'W';
	if (v & PAWN) res[1] = 'P';
	if (v & KNIGHT) res[1] = 'N';
	if (v & BISHOP) res[1] = 'B';
	if (v & ROOK) res[1] = 'R';
	if (v & QUEEN) res[1] = 'Q';
	if (v & KING) res[1] = 'K';
	return res;
}

inline bool color(uint8_t x) { return x & 2; }
inline bool slider(uint8_t x) { return x & (PAWN | BISHOP | KNIGHT | ROOK | QUEEN | KING); }
inline int8_t getIndex(int8_t rank, int8_t file) { return (rank<<3) | file; }
inline int8_t getIndex(std::string s) {
	if (s.size() < 2) return 0;
	int8_t res;
	if (s[0] >= 'a' && s[0] <= 'h') res |= (s[0] - 'a');
	if (s[1] >= '1' && s[1] <= '8') res |= ((s[1] - '1')<<3);
	return res;
}
inline int8_t getRank(int8_t v) { return v>>3; }
inline int8_t getFile(int8_t v) { return v & 7; }
inline std::string coords(int8_t v) {
	std::string res = "xx";
	res[0] = 'a' + (v & 7);
	res[1] = '1' + (v>>3);
	return res;
}

// BEGIN BITBOARDS
// The nth bit (LSB begins the order, 0 indexed) is that square on the board. So A1 is bit 0, B1 is bit 1, A2 is bit 8, and so on.

// Doing this for portability
inline int8_t bitScanForward(uint64_t x) {
	int8_t res = 0;
	if (!x) return 64;
	if (!(x & 0xFFFFFFFF)) {
		res |= 32;
		x >>= 32;
	}
	if (!(x & 0xFFFF)) {
		res |= 16;
		x >>= 16;
	}
	if (!(x & 0xFF)) {
		res |= 8;
		x >>= 8;
	}
	if (!(x & 0xF)) {
		res |= 4;
		x >>= 4;
	}
	if (!(x & 3)) {
		res |= 2;
		x >>= 2;
	}
	res |= 1 - (x & 1);
	return res;
}

// from CPW
inline int8_t bitScanReverse(uint64_t x) {
const int index64[64] = {
    0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};
	const uint64_t debruijn64 = uint64_t(0x03f79d71b4cb0a89);
   	if (x == 0) return -1;
   	x |= x >> 1; 
   	x |= x >> 2;
   	x |= x >> 4;
   	x |= x >> 8;
   	x |= x >> 16;
   	x |= x >> 32;
   	return index64[(x * debruijn64) >> 58];
}

inline uint64_t onehot(int x) { return 1ULL<<x; }

uint64_t rankBBUtil(int x) {
	if (x < 0 || x > 7) return 0;
	return 255ULL<<(x<<3);
}

const uint64_t rankBB[8] = {rankBBUtil(0), rankBBUtil(1), rankBBUtil(2), rankBBUtil(3), rankBBUtil(4), rankBBUtil(5), rankBBUtil(6), rankBBUtil(7)}; 

uint64_t fileBBUtil(int x) {
	if (x < 0 || x > 7) return 0;
	uint64_t aFile = 1ULL | (1ULL<<8) | (1ULL<<16) | (1ULL<<24) | (1ULL<<32) | (1ULL<<40) | (1ULL<<48) | (1ULL<<56); 
	return aFile<<x;
}

const uint64_t fileBB[8] = {fileBBUtil(0), fileBBUtil(1), fileBBUtil(2), fileBBUtil(3), fileBBUtil(4), fileBBUtil(5), fileBBUtil(6), fileBBUtil(7)}; 

// along the line going up and to the right (y = x + x0, or y - x = x0)
// again, index = (y<<3) | x
uint64_t diagonalBBUtil(int x0) {
	if (x0 > 7 || x0 < -7) return 0;
	uint64_t res = 0;
	for (int x = 0; x < 8; x++) {
		int y = x + x0;
		if (x < 0 || y < 0 || y > 7) continue;
		int index = (y<<3) | x;
		res |= (1ULL<<index);
	}
	return res;
}

const uint64_t diagonalBBs[15] = {
	diagonalBBUtil(-7), diagonalBBUtil(-6), diagonalBBUtil(-5), 
	diagonalBBUtil(-4), diagonalBBUtil(-3), diagonalBBUtil(-2), 
	diagonalBBUtil(-1), diagonalBBUtil(0), diagonalBBUtil(1), 
	diagonalBBUtil(2), diagonalBBUtil(3), diagonalBBUtil(4), 
	diagonalBBUtil(5), diagonalBBUtil(6), diagonalBBUtil(7) 
};

inline uint64_t diagonalBB(int x0) {
	if (x0 > 7 || x0 < -7) return 0;
	return diagonalBBs[x0 + 7];
}

// a bisector goes the other way, y = 7 + x0 - x;
uint64_t bisectorBBUtil(int x0) {
	if (x0 > 7 || x0 < -7) return 0;
	uint64_t res = 0;
	for (int x = 0; x < 8; x++) {
		int y = 7 + x0 - x;
		if (x < 0 || y < 0 || y > 7) continue;
		int index = (y<<3) | x;
		res |= 1ULL<<index;
	}
	return res;
}

const uint64_t bisectorBB[15] = {
	bisectorBBUtil(-7), bisectorBBUtil(-6), bisectorBBUtil(-5), 
	bisectorBBUtil(-4), bisectorBBUtil(-3), bisectorBBUtil(-2), 
	bisectorBBUtil(-1), bisectorBBUtil(0), bisectorBBUtil(1), 
	bisectorBBUtil(2), bisectorBBUtil(3), bisectorBBUtil(4), 
	bisectorBBUtil(5), bisectorBBUtil(6), bisectorBBUtil(7) 
};

// split the board in two around a square. The square is not included.
inline uint64_t positiveSide(int8_t src) { return uint64_t(-2)<<src; }
inline uint64_t negativeSide(int8_t src) { return (1ULL<<src) - 1; }

// knight attacks

const uint64_t knightBBUtil(int x0) {
	int r = x0>>3;
	int c = x0&7;
	uint64_t res = 0;
	if (c > 0) {
		if (r > 1) res |= 1ULL<<(x0 - 17); 
		if (r < 6) res |= 1ULL<<(x0 + 15);
		if (c > 1) {
			if (r > 0) res |= 1ULL<<(x0 - 10);
			if (r < 7) res |= 1ULL<<(x0 + 6);
		}
	}
	if (c < 7) {
		if (r > 1) res |= 1ULL<<(x0 - 15); 
		if (r < 6) res |= 1ULL<<(x0 + 17);
		if (c < 6) {
			if (r > 0) res |= 1ULL<<(x0 - 6);
			if (r < 7) res |= 1ULL<<(x0 + 10);
		}
	}
	return res;
}

const uint64_t knightBB[64] = {
knightBBUtil(0), knightBBUtil(1), knightBBUtil(2), knightBBUtil(3), knightBBUtil(4), knightBBUtil(5), knightBBUtil(6), knightBBUtil(7), 
knightBBUtil(8), knightBBUtil(9), knightBBUtil(10), knightBBUtil(11), knightBBUtil(12), knightBBUtil(13), knightBBUtil(14), knightBBUtil(15), 
knightBBUtil(16), knightBBUtil(17), knightBBUtil(18), knightBBUtil(19), knightBBUtil(20), knightBBUtil(21), knightBBUtil(22), knightBBUtil(23), 
knightBBUtil(24), knightBBUtil(25), knightBBUtil(26), knightBBUtil(27), knightBBUtil(28), knightBBUtil(29), knightBBUtil(30), knightBBUtil(31), 
knightBBUtil(32), knightBBUtil(33), knightBBUtil(34), knightBBUtil(35), knightBBUtil(36), knightBBUtil(37), knightBBUtil(38), knightBBUtil(39), 
knightBBUtil(40), knightBBUtil(41), knightBBUtil(42), knightBBUtil(43), knightBBUtil(44), knightBBUtil(45), knightBBUtil(46), knightBBUtil(47), 
knightBBUtil(48), knightBBUtil(49), knightBBUtil(50), knightBBUtil(51), knightBBUtil(52), knightBBUtil(53), knightBBUtil(54), knightBBUtil(55), 
knightBBUtil(56), knightBBUtil(57), knightBBUtil(58), knightBBUtil(59), knightBBUtil(60), knightBBUtil(61), knightBBUtil(62), knightBBUtil(63)
};

// king field generator
uint64_t kingBB(int8_t src) {
	uint64_t square = 1ULL<<src;
	uint64_t res = 1ULL<<src;
	int8_t rank = src>>3;
	int8_t file = src & 7;
	if (rank > 0) {
		res |= square>>8;
		if (file > 0) res |= square>>9;
		if (file < 7) res |= square>>7;
	}
	if (rank < 7) {
		res |= square<<8;
		if (file > 0) res |= square<<7;
		if (file < 7) res |= square<<9;
	}
	if (file > 0) res |= square>>1;
	if (file < 7) res |= square<<1;
	return res;
}

// These transformation functions are from CPW

uint64_t flipVertical(uint64_t x) {
	constexpr uint64_t k1 = uint64_t(0x00FF00FF00FF00FF);
   	constexpr uint64_t k2 = uint64_t(0x0000FFFF0000FFFF);
   	x = ((x >>  8) & k1) | ((x & k1) <<  8);
   	x = ((x >> 16) & k2) | ((x & k2) << 16);
   	x = ( x >> 32)       | ( x       << 32);
   	return x;
}

uint64_t flipHorizontal(uint64_t x) {
   	constexpr uint64_t k1 = uint64_t(0x5555555555555555);
   	constexpr uint64_t k2 = uint64_t(0x3333333333333333);
   	constexpr uint64_t k4 = uint64_t(0x0f0f0f0f0f0f0f0f);
   	x = ((x >> 1) & k1) | ((x & k1) << 1);
   	x = ((x >> 2) & k2) | ((x & k2) << 2);
   	x = ((x >> 4) & k4) | ((x & k4) << 4);
   	return x;
}

uint64_t flipDiagonal(uint64_t x) {
   	uint64_t t;
   	constexpr uint64_t k1 = uint64_t(0x5500550055005500);
   	constexpr uint64_t k2 = uint64_t(0x3333000033330000);
   	constexpr uint64_t k4 = uint64_t(0x0f0f0f0f00000000);
   	t  = k4 & (x ^ (x << 28));
   	x ^=       t ^ (t >> 28) ;
   	t  = k2 & (x ^ (x << 14));
   	x ^=       t ^ (t >> 14) ;
   	t  = k1 & (x ^ (x <<  7));
   	x ^=       t ^ (t >>  7) ;
   	return x;
}

uint64_t flipBisector(uint64_t x) {
   	uint64_t t;
   	constexpr uint64_t k1 = uint64_t(0xaa00aa00aa00aa00);
   	constexpr uint64_t k2 = uint64_t(0xcccc0000cccc0000);
   	constexpr uint64_t k4 = uint64_t(0xf0f0f0f00f0f0f0f);
   	t  =       x ^ (x << 36) ;
   	x ^= k4 & (t ^ (x >> 36));
   	t  = k2 & (x ^ (x << 18));
   	x ^=       t ^ (t >> 18) ;
   	t  = k1 & (x ^ (x <<  9));
   	x ^=       t ^ (t >>  9) ;
   	return x;
}

uint64_t rotateLeft(uint64_t x) { return flipHorizontal(flipDiagonal(x)); }
uint64_t rotateRight(uint64_t x) { return flipVertical(flipDiagonal(x)); }

// https://chessprogramming.org/Classical_Approach. Using this you go from a for loop to a single value
// This method determines, given a line and a side, a ray spanning from the line to the first blocker on the ray.
// In case of possible captures, said blocker is included. (Landing on a piece of the same color will be filtered out in the actual legality check way before this happens.)
uint64_t rayAttack(uint64_t line, uint64_t occupied, int8_t src, bool side) {
	if (side) {
		uint64_t ray = line & positiveSide(src);
		uint64_t blocker = ray & occupied;
		if (blocker) {
			int8_t sq = bitScanForward(blocker);
			ray ^= (line & positiveSide(sq));
		}
		return ray;
	}
	uint64_t ray = line & negativeSide(src);
	uint64_t blocker = ray & occupied;
	if (blocker) {
		int8_t sq = bitScanReverse(blocker);
		ray ^= (line & negativeSide(sq));
	}
	return ray;
}

// END BITBOARDS

#endif