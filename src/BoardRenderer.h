
#ifndef BOARD_RENDERER_H
#define BOARD_RENDERER_H

#include <SFML/Graphics.hpp>
#include <vector>

#include "Board.h"
#include "Move.h"
#include "PieceTextures.h"

class BoardRenderer
{
public:
    static void drawBoard(
        sf::RenderWindow &window,
        Board &board,
        PieceTextures &textures,
        const std::vector<Move> &possibleMoves,
        bool isPieceSelected,
        int selectedRow,
        int selectedCol,
        float squareSize,
        float startX,
        float startY,
        bool isDraggingPiece,
        int draggedRow,
        int draggedCol,
        float dragMouseX,
        float dragMouseY,
        bool hasLastMove,
        const Move& lastMove);

    static void drawCoordinates(
        sf::RenderWindow &window,
        sf::Font &font,
        float squareSize,
        float startX,
        float startY);

    static void drawCapturedPieces(
        sf::RenderWindow &window,
        const std::vector<char> &capturedPieces,
        PieceTextures &textures,
        float startX,
        float startY);
};

#endif
