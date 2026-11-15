#include "GameState.h"
#include "MoveValidator.h"

bool hasInsufficientMaterial(const Board& board) {
    int bishopCount = 0;
    int knightCount = 0;
    int otherPieceCount = 0;

    int bishopColor = -1;
    bool bishopsSameColor = true;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            char piece = board.squares[row][col];

            if (piece == '.' || piece == 'K' || piece == 'k')
                continue;

            if (piece == 'B' || piece == 'b') {
                bishopCount++;

                int currentColor = (row + col) % 2;

                if (bishopColor == -1)
                    bishopColor = currentColor;
                else if (bishopColor != currentColor)
                    bishopsSameColor = false;
            }
            else if (piece == 'N' || piece == 'n')
                knightCount++;
            else
                otherPieceCount++;
        }
    }

    int minorPieceCount = bishopCount + knightCount;

    // King vs King
    if (minorPieceCount == 0 && otherPieceCount == 0)
        return true;

    // King + Bishop/Knight vs King
    if (minorPieceCount == 1 && otherPieceCount == 0)
        return true;

    // King + Bishop vs King + Bishop (same-colored squares)
    if (bishopCount == 2 &&
        knightCount == 0 &&
        otherPieceCount == 0 &&
        bishopsSameColor) {
        return true;
    }

    return false;
}

GameState getGameState(const Board& board, bool isWhite) {
    GameState state;

    state.isCheck = isKingInCheck(board, isWhite);

    bool hasLegalMove = hasAnyLegalMove(board, isWhite);

    state.isCheckmate = state.isCheck && !hasLegalMove;
    state.isStalemate = !state.isCheck && !hasLegalMove;

    state.isDrawByInsufficientMaterial = hasInsufficientMaterial(board);
    state.canClaimDrawByFiftyMoveRule = board.halfMoveClock >= 100;
    state.isDrawBySeventyFiveMoveRule = board.halfMoveClock >= 150 && !state.isCheckmate;

    state.isGameOver = state.isCheckmate ||
        state.isStalemate ||
        state.isDrawByInsufficientMaterial ||
        state.isDrawBySeventyFiveMoveRule;

    return state;
}