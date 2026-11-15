#include "MoveNotation.h"

string getMoveNotation(
    char piece,
    char target,
    const Move& move,
    const GameState& state,
    bool isEnPassant
) {
    // Castling notation
    if ((piece == 'K' || piece == 'k') &&
        move.fromCol == 4 &&
        move.fromRow == move.toRow) {

        if (move.toCol == 6)
            return "O-O";

        if (move.toCol == 2)
            return "O-O-O";
    }

    string notation;

    // Piece letter
    switch (piece) {
    case 'K':
    case 'k': notation += 'K'; break;
    case 'Q':
    case 'q': notation += 'Q'; break;
    case 'R':
    case 'r': notation += 'R'; break;
    case 'B':
    case 'b': notation += 'B'; break;
    case 'N':
    case 'n': notation += 'N'; break;
    }

    bool isCapture = target != '.' || isEnPassant;

    // Pawn captures include their starting file
    if ((piece == 'P' || piece == 'p') && isCapture)
        notation += char('a' + move.fromCol);

    // Capture symbol
    if (isCapture)
        notation += 'x';

    // Destination square
    notation += char('a' + move.toCol);
    notation += char('8' - move.toRow);

    // Check or checkmate
    if (state.isCheckmate)
        notation += '#';
    else if (state.isCheck)
        notation += '+';

    return notation;
}