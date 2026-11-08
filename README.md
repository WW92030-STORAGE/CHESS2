# chess2

Chess engine using Bitboards instead of the mailbox (array of chess piece entities e.g. `uint8_t`) that I did previously. Various concepts are based on various articles on ChessProgramming wiki (CPW).

# WHAT IS A BITBOARD

A bitboard is a `uint64_t` which consists of 64 bits, same number of bits as a chessboard. In effect it represents a particular view of a chessboard. For instance, where the white pawns are, where the black kings are. But also theoretical measurements, such as ranks, files, diagonals, and lines of sight.

Most of the line-of-sight and other checks can be done very cleanly using bitboards and bitwise operations.

# BASIC USAGE

- To initialize a board or clone a position:
```
ChessPosition pos; // ChessPosition and ChessMove do not store pointers so copy assignment is ok.
ChessPosition pos2 = pos;
```
- To list all possible moves:
```
std::vector<ChessMove> moves = pos.possibleLegalMoves(0); // 1 for black
```
- To apply a move:
```
ChessMove move(12, 28); // Represents the initial move d2 > d4
if (pos2.isLegalMove(move)) // Checks if the move is legal, also sets flags if needed
	pos2.apply(move); // Apply the move. If you are promoting a pawn you will have to set the flag yourself, otherwise it defaults to knight.
```
- Functions to simulate playing game against another player or an engine, using the standard input, are provided in `Chess2GameUtils.h`. Each one also has options to display legal moves for your side, either globally or per square.

# EVALUATION AND ENGINES

- Evaluation class:
```
Evaluation eval;
std::cout << eval(pos); // operator() or the evaluate() method returns the evaluation as a float
```
- Engine:
```
SearchEngine mmSearch{&eval}; // Initializes an engine using a pointer to the evaluation. The evaluation function is not owned by the search engine.
std::cout << mmSearch.search(pos, 3).to_string(); // Search the position, with depth 3 (you max --> opp min --> you max --> eval)
```

Both `Evaluation` and `SearchEngine` have various derived classes which can be used the same way and/or customized.