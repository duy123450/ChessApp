#pragma once

struct Board
{
    char squares[8][8] = {
        // Black pieces
        {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
        {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},

        // Empty squares
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},

        // White pieces
        {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
        {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}};

    // Track King movement
    bool whiteKingMoved = false;
    bool blackKingMoved = false;

    // Track Rook movement
    bool whiteKingsideRookMoved = false;
    bool whiteQueensideRookMoved = false;

    bool blackKingsideRookMoved = false;
    bool blackQueensideRookMoved = false;

    // En passant tracking
    int enPassantRow = -1;
    int enPassantCol = -1;

    // 50-move rule tracking
    int halfMoveClock = 0;
};