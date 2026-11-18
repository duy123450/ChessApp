
#ifndef GAME_DATA_H
#define GAME_DATA_H

#include <vector>
#include <string>
#include <utility>
#include <unordered_map>

#include "Board.h"
#include "Move.h"


struct SavedGameData
{
    Board board;

    bool whiteTurn = true;

    bool hasLastMove = false;
    Move lastMove{};

    std::vector<char> whiteCapturedPieces;
    std::vector<char> blackCapturedPieces;

    std::vector<std::pair<std::string, std::string>> moveHistory;

    std::unordered_map<std::string, int> positionOccurrences;

    bool isPromotionPending = false;
    bool promotionIsWhite = false;

    int promotionRow = -1;
    int promotionCol = -1;

    Move pendingMove{};

    char pendingMovedPiece = '.';
    char pendingCapturedPiece = '.';

    bool pendingIsEnPassant = false;
};

struct GameData
{
    Board board;

    std::unordered_map<std::string, int> positionOccurrences;

    bool isPieceSelected = false;
    int selectedRow = -1;
    int selectedCol = -1;

    std::vector<Move> possibleMoves;

    bool hasLastMove = false;
    Move lastMove{};

    std::vector<char> whiteCapturedPieces;
    std::vector<char> blackCapturedPieces;

    std::vector<std::pair<std::string, std::string>> moveHistory;

    int historyScrollOffset = 0;
    int previousMoveCount = 0;

    bool isDraggingScrollbar = false;
    float scrollbarDragOffset = 0.0f;

    bool isDraggingPiece = false;

    int draggedRow = -1;
    int draggedCol = -1;

    float dragMouseX = 0.0f;
    float dragMouseY = 0.0f;

    int whiteMaterial = 0;
    int blackMaterial = 0;

    bool gameOver = false;
    bool saveFileCleared = false;
    std::string gameOverMessage;
    bool whiteTurn = true;

    bool isPromotionPending = false;
    bool promotionIsWhite = false;

    int promotionRow = -1;
    int promotionCol = -1;

    Move pendingMove{};

    char pendingMovedPiece = '.';
    char pendingCapturedPiece = '.';

    bool pendingIsEnPassant = false;
};

#endif
