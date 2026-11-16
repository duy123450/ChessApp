#include <stdexcept> // For handling exceptions (errors)
#include "PieceTextures.h"

void loadPieceTextures(PieceTextures &textures)
{
    textures.whiteKing.loadFromFile("assets/pieces/whiteKing.png");
    textures.whiteQueen.loadFromFile("assets/pieces/whiteQueen.png");
    textures.whiteRook.loadFromFile("assets/pieces/whiteRook.png");
    textures.whiteBishop.loadFromFile("assets/pieces/whiteBishop.png");
    textures.whiteKnight.loadFromFile("assets/pieces/whiteKnight.png");
    textures.whitePawn.loadFromFile("assets/pieces/whitePawn.png");

    textures.blackKing.loadFromFile("assets/pieces/blackKing.png");
    textures.blackQueen.loadFromFile("assets/pieces/blackQueen.png");
    textures.blackRook.loadFromFile("assets/pieces/blackRook.png");
    textures.blackBishop.loadFromFile("assets/pieces/blackBishop.png");
    textures.blackKnight.loadFromFile("assets/pieces/blackKnight.png");
    textures.blackPawn.loadFromFile("assets/pieces/blackPawn.png");
}

Texture &getPieceTexture(char piece, PieceTextures &textures)
{
    switch (piece)
    {
    case 'K':
        return textures.whiteKing;
    case 'Q':
        return textures.whiteQueen;
    case 'R':
        return textures.whiteRook;
    case 'B':
        return textures.whiteBishop;
    case 'N':
        return textures.whiteKnight;
    case 'P':
        return textures.whitePawn;

    case 'k':
        return textures.blackKing;
    case 'q':
        return textures.blackQueen;
    case 'r':
        return textures.blackRook;
    case 'b':
        return textures.blackBishop;
    case 'n':
        return textures.blackKnight;
    case 'p':
        return textures.blackPawn;
    }

    throw runtime_error("Invalid chess piece");
}

void drawPieces(
    RenderWindow &window,
    Board &board,
    PieceTextures &textures,
    float squareSize,
    float startX,
    float startY,
    int skipRow,
    int skipCol)
{
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {

            // Skip the piece currently being dragged
            if (row == skipRow && col == skipCol)
                continue;

            // Get the piece from the matrix
            char piece = board.squares[row][col];

            // Skip empty squares
            if (piece == '.')
                continue;

            // Get the corresponding texture
            Texture &texture = getPieceTexture(piece, textures);

            // Create a sprite
            Sprite sprite(texture);

            // Scale the piece to fit the board square
            Vector2u textureSize = texture.getSize();

            sprite.setScale({
                squareSize / textureSize.x,
                squareSize / textureSize.y
                });

            // Position the sprite
            float x = startX + col * squareSize;
            float y = startY + row * squareSize;

            sprite.setPosition({ x, y });

            // Draw the sprite
            window.draw(sprite);
        }
    }
}