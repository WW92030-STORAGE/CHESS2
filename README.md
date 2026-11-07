# chess2

Chess engine using Bitboards instead of the mailbox (array of chess piece entities e.g. `uint8_t`) that I did previously. Most of this comes from various articles on ChessProgramming wiki (CPW).

# WHAT IS A BITBOARD

A bitboard is a `uint64_t` which consists of 64 bits, same number of bits as a chessboard. In effect it represents a particular view of a chessboard. For instance, where the white pawns are, where the black kings are. But also theoretical measurements, such as ranks, files, diagonals, and lines of sight.

Most of the line-of-sight and other checks can be done very cleanly using bitboards and bitwise operations.