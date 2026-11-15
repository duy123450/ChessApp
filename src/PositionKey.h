
#pragma once

#include <string>
#include "MoveValidator.h"

// Check whether the current player has a legal en passant capture
inline bool hasLegalEnPassantCapture(const Board& board, bool whiteToMove)
{
    if (board.enPassantRow < 0 || board.enPassantRow >= 8 ||
        board.enPassantCol < 0 || board.enPassantCol >= 8)
    {
        return false;
    }

    int fromRow = board.enPassantRow;
    int toRow = whiteToMove ? fromRow - 1 : fromRow + 1;

    if (toRow < 0 || toRow >= 8)
        return false;

    char pawn = whiteToMove ? 'P' : 'p';

    for (int offset : { -1, 1 })
    {
        int fromCol = board.enPassantCol + offset;

        if (fromCol < 0 || fromCol >= 8)
            continue;

        if (board.squares[fromRow][fromCol] != pawn)
            continue;

        Move move{ fromRow, fromCol, toRow, board.enPassantCol };

        if (isValidMove(board, move))
            return true;
    }

    return false;
}

// Generate a string representing the current chess position
inline std::string getPositionKey(const Board& board, bool whiteToMove)
{
    std::string key;
    key.reserve(80);

    // 1. Record all 64 squares
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            key += board.squares[row][col];
        }
    }

    // 2. Record whose turn it is
    key += whiteToMove ? 'w' : 'b';

    // 3. Record castling rights
    key += (!board.whiteKingMoved &&
        board.squares[7][4] == 'K' &&
        !board.whiteKingsideRookMoved &&
        board.squares[7][7] == 'R') ? '1' : '0';

    key += (!board.whiteKingMoved &&
        board.squares[7][4] == 'K' &&
        !board.whiteQueensideRookMoved &&
        board.squares[7][0] == 'R') ? '1' : '0';

    key += (!board.blackKingMoved &&
        board.squares[0][4] == 'k' &&
        !board.blackKingsideRookMoved &&
        board.squares[0][7] == 'r') ? '1' : '0';

    key += (!board.blackKingMoved &&
        board.squares[0][4] == 'k' &&
        !board.blackQueensideRookMoved &&
        board.squares[0][0] == 'r') ? '1' : '0';

    // 4. Record whether a legal en passant capture is available
    key += hasLegalEnPassantCapture(board, whiteToMove) ? 'e' : '-';

    return key;
}
