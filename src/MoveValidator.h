#pragma once

#include "Board.h"
#include "Move.h"

bool isValidMove(const Board& board, const Move& move);
bool hasAnyLegalMove(const Board& board, bool isWhite);

bool isPawnMoveValid(const Board& board, const Move& move, bool isWhite);
bool isRookMoveValid(const Board& board, const Move& move);
bool isBishopMoveValid(const Board& board, const Move& move);
bool isQueenMoveValid(const Board& board, const Move& move);
bool isKnightMoveValid(const Board& board, const Move& move);

bool findKing(const Board& board, bool isWhite, int& kingRow, int& kingCol);
bool isKingInCheck(const Board& board, bool isWhite);
bool isKingMoveValid(const Board& board, const Move& move);

// Special rules
bool isCastlingMoveValid(const Board& board, const Move& move, bool isWhite);

// Shared path-checking function
bool isPathClear(const Board& board, const Move& move);