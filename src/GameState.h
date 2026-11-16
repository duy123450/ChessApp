#pragma once

#include "Board.h"

struct GameState
{
    bool isCheck = false;
    bool isCheckmate = false;
    bool isStalemate = false;
    bool isDrawByInsufficientMaterial = false;
    bool canClaimDrawByFiftyMoveRule = false;
    bool isDrawBySeventyFiveMoveRule = false;
    bool isGameOver = false;
};

GameState getGameState(const Board &board, bool isWhite);

bool hasInsufficientMaterial(const Board &board);