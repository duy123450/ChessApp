
#include "BoardRenderer.h"

#include <string>

void BoardRenderer::drawBoard(
    sf::RenderWindow &window,
    Board &board,
    PieceTextures &textures,
    const std::vector<Move> &possibleMoves,
    bool isPieceSelected,
    int selectedRow,
    int selectedCol,
    float squareSize,
    float startX,
    float startY)
{
    sf::RectangleShape square(
        sf::Vector2f(squareSize, squareSize));

    // Draw board squares
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            float x = startX + col * squareSize;
            float y = startY + row * squareSize;

            square.setPosition({x, y});

            if ((row + col) % 2 == 0)
                square.setFillColor(sf::Color(240, 217, 181));
            else
                square.setFillColor(sf::Color(181, 136, 99));

            window.draw(square);
        }
    }

    // Highlight selected square
    if (isPieceSelected)
    {
        sf::RectangleShape highlight(
            sf::Vector2f(squareSize, squareSize));

        highlight.setFillColor(sf::Color(255, 255, 0, 100));

        float x = startX + selectedCol * squareSize;
        float y = startY + selectedRow * squareSize;

        highlight.setPosition({x, y});
        window.draw(highlight);
    }

    // Draw possible move indicators
    sf::CircleShape moveDot(8.0f);
    moveDot.setFillColor(sf::Color(100, 100, 100, 150));

    sf::CircleShape captureRing(squareSize / 2 - 5);
    captureRing.setFillColor(sf::Color::Transparent);
    captureRing.setOutlineThickness(5);
    captureRing.setOutlineColor(sf::Color(100, 100, 100, 150));

    for (const Move &move : possibleMoves)
    {
        float x = startX + move.toCol * squareSize;
        float y = startY + move.toRow * squareSize;

        if (board.squares[move.toRow][move.toCol] == '.')
        {
            moveDot.setPosition({x + squareSize / 2 - 8,
                                 y + squareSize / 2 - 8});

            window.draw(moveDot);
        }
        else
        {
            captureRing.setPosition({x + 5, y + 5});
            window.draw(captureRing);
        }
    }

    // Draw chess pieces
    drawPieces(
        window,
        board,
        textures,
        squareSize,
        startX,
        startY);
}

void BoardRenderer::drawCoordinates(
    sf::RenderWindow &window,
    sf::Font &font,
    float squareSize,
    float startX,
    float startY)
{
    for (int i = 0; i < 8; i++)
    {
        sf::Color coordinateColor = (i % 2 == 0)
                                        ? sf::Color(181, 136, 99)
                                        : sf::Color(240, 217, 181);

        sf::Text rank(font, std::to_string(8 - i), 14);

        rank.setFillColor(coordinateColor);
        rank.setPosition({startX + 4,
                          startY + i * squareSize + 2});

        window.draw(rank);

        sf::Text file(font, std::string(1, 'a' + i), 14);

        file.setFillColor(
            i % 2 == 0
                ? sf::Color(240, 217, 181)
                : sf::Color(181, 136, 99));

        file.setPosition({startX + (i + 1) * squareSize - 18,
                          startY + 8 * squareSize - 18});

        window.draw(file);
    }
}

void BoardRenderer::drawCapturedPieces(
    sf::RenderWindow &window,
    const std::vector<char> &capturedPieces,
    PieceTextures &textures,
    float startX,
    float startY)
{
    const float pieceSize = 30.0f;
    const float spacing = 36.0f;
    const int piecesPerRow = 9;

    for (int i = 0; i < static_cast<int>(capturedPieces.size()); i++)
    {
        sf::Sprite pieceSprite(textures.whiteKing);

        pieceSprite.setTexture(
            getPieceTexture(capturedPieces[i], textures));

        int row = i / piecesPerRow;
        int col = i % piecesPerRow;

        pieceSprite.setPosition({startX + col * spacing,
                                 startY + row * 30.0f});

        float scale = pieceSize / 100.0f;
        pieceSprite.setScale({scale, scale});

        window.draw(pieceSprite);
    }
}
