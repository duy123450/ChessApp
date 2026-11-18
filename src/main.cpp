
#include <SFML/Graphics.hpp>

#include <algorithm>
#include <iostream>
#include <optional>

#include "PieceTextures.h"
#include "GameData.h"
#include "SaveManager.h"
#include "BoardRenderer.h"
#include "UI.h"
#include "Game.h"

using namespace sf;
using namespace std;

// Board position
struct BoardPosition
{
    int row;
    int col;
};

// Convert mouse coordinates into board coordinates
optional<BoardPosition> getBoardPosition(
    float mouseX,
    float mouseY,
    float startX,
    float startY,
    float squareSize)
{
    constexpr int boardSize = 8;

    if (
        mouseX < startX ||
        mouseX >= startX + boardSize * squareSize ||
        mouseY < startY ||
        mouseY >= startY + boardSize * squareSize)
    {
        return nullopt;
    }

    int col = static_cast<int>((mouseX - startX) / squareSize);
    int row = static_cast<int>((mouseY - startY) / squareSize);

    return BoardPosition{ row, col };
}

int main()
{
    // Initialize window
    RenderWindow window(
        VideoMode({ 1010, 800 }),
        "Chess App");

    // Board settings
    const int boardSize = 8;
    const float squareSize = 75.0f;

    const float startX = 10.0f;
    const float startY = 100.0f;

    // History settings
    const int visibleRows = 20;

    const float historyContentTop = 100.0f;
    const float historyContentHeight = 490.0f;

    const float scrollbarX = 980.0f;
    const float scrollbarWidth = 10.0f;

    // Initialize game
    GameData game;
    Game gameController(game);

    bool showLoadConfirmation = false;

    // Load resources
    PieceTextures textures;
    loadPieceTextures(textures);

    Font font;

    if (!font.openFromFile("assets/fonts/ARIAL.TTF"))
        return -1;

    UI ui(font);

    // Main loop
    while (window.isOpen())
    {
        // Handle events
        while (const optional event = window.pollEvent())
        {
            // Close window
            if (event->is<Event::Closed>())
            {
                window.close();
            }

            // Mouse wheel scrolling
            if (const auto* wheel = event->getIf<Event::MouseWheelScrolled>())
            {
                int mouseX = wheel->position.x;
                int mouseY = wheel->position.y;

                if (
                    mouseX >= 620 && mouseX <= 1000 &&
                    mouseY >= historyContentTop &&
                    mouseY < historyContentTop + historyContentHeight)
                {
                    int maxScroll = max(
                        0,
                        static_cast<int>(game.moveHistory.size()) - visibleRows);

                    game.historyScrollOffset -=
                        static_cast<int>(wheel->delta * 3.0f);

                    game.historyScrollOffset = max(
                        0,
                        min(game.historyScrollOffset, maxScroll));
                }
            }

            // Mouse button pressed
            if (const auto* mousePressed = event->getIf<Event::MouseButtonPressed>())
            {
                if (mousePressed->button == Mouse::Button::Left)
                {
                    float mouseX = static_cast<float>(mousePressed->position.x);
                    float mouseY = static_cast<float>(mousePressed->position.y);

                    // Load confirmation dialog
                    if (showLoadConfirmation)
                    {
                        if (
                            mouseX >= 390.0f && mouseX <= 490.0f &&
                            mouseY >= 425.0f && mouseY <= 465.0f)
                        {
                            showLoadConfirmation = false;

                            if (!SaveManager::loadGame(game))
                                cout << "Failed to load game!" << '\n';
                        }
                        else if (
                            mouseX >= 520.0f && mouseX <= 620.0f &&
                            mouseY >= 425.0f && mouseY <= 465.0f)
                        {
                            showLoadConfirmation = false;
                            cout << "Load cancelled!" << '\n';
                        }
                    }

                    // Pawn promotion popup
                    else if (game.isPromotionPending)
                    {
                        char promotionChoice = ui.handlePromotionClick(
                            mouseX,
                            mouseY);

                        if (promotionChoice != '.')
                        {
                            gameController.handlePromotion(promotionChoice);
                        }
                    }

                    // Normal game interactions
                    else
                    {
                        // New Game button
                        if (
                            mouseX >= 620.0f && mouseX <= 750.0f &&
                            mouseY >= 620.0f && mouseY <= 660.0f)
                        {
                            gameController.reset();
                        }

                        // Save Game button
                        if (
                            mouseX >= 760.0f && mouseX <= 870.0f &&
                            mouseY >= 620.0f && mouseY <= 660.0f)
                        {
                            SaveManager::saveGame(game);
                        }

                        // Load Game button
                        if (
                            mouseX >= 760.0f && mouseX <= 870.0f &&
                            mouseY >= 675.0f && mouseY <= 715.0f)
                        {
                            showLoadConfirmation = true;
                        }

                        // Claim Draw button
                        if (ui.isClaimDrawButtonClicked(mouseX, mouseY))
                        {
                            gameController.handleDrawClaim('D');
                            gameController.handleDrawClaim('T');
                        }

                        // Scrollbar click and drag
                        int totalRows = static_cast<int>(game.moveHistory.size());

                        if (
                            totalRows > visibleRows &&
                            mouseX >= scrollbarX &&
                            mouseX <= scrollbarX + scrollbarWidth &&
                            mouseY >= historyContentTop &&
                            mouseY <= historyContentTop + historyContentHeight)
                        {
                            int maxScroll = totalRows - visibleRows;

                            float thumbHeight = max(
                                30.0f,
                                historyContentHeight * visibleRows / totalRows);

                            float thumbTop =
                                historyContentTop +
                                (static_cast<float>(game.historyScrollOffset) / maxScroll) *
                                (historyContentHeight - thumbHeight);

                            if (
                                mouseY >= thumbTop &&
                                mouseY <= thumbTop + thumbHeight)
                            {
                                game.isDraggingScrollbar = true;
                                game.scrollbarDragOffset = mouseY - thumbTop;
                            }
                            else
                            {
                                if (mouseY < thumbTop)
                                    game.historyScrollOffset -= visibleRows;
                                else
                                    game.historyScrollOffset += visibleRows;

                                game.historyScrollOffset = max(
                                    0,
                                    min(game.historyScrollOffset, maxScroll));
                            }
                        }

                        // Board click
                        auto boardPosition = getBoardPosition(
                            mouseX,
                            mouseY,
                            startX,
                            startY,
                            squareSize);

                        if (
                            boardPosition &&
                            !showLoadConfirmation &&
                            !game.isPromotionPending &&
                            !game.gameOver)
                        {
                            int row = boardPosition->row;
                            int col = boardPosition->col;

                            char piece = game.board.squares[row][col];

                            bool pieceIsWhite = piece >= 'A' && piece <= 'Z';

                            // Start dragging a piece belonging to the current player
                            if (
                                piece != '.' &&
                                pieceIsWhite == game.whiteTurn)
                            {
                                // Select the piece if it isn't already selected
                                if (
                                    !game.isPieceSelected ||
                                    game.selectedRow != row ||
                                    game.selectedCol != col)
                                {
                                    gameController.handleBoardClick(row, col);
                                }

                                if (
                                    game.isPieceSelected &&
                                    game.selectedRow == row &&
                                    game.selectedCol == col)
                                {
                                    game.isDraggingPiece = true;
                                    game.draggedRow = row;
                                    game.draggedCol = col;
                                    game.dragMouseX = mouseX;
                                    game.dragMouseY = mouseY;
                                }
                            }
                            else
                            {
                                gameController.handleBoardClick(row, col);
                            }
                        }
                    }
                }
            }

            // Mouse button released
            if (const auto* mouseReleased = event->getIf<Event::MouseButtonReleased>())
            {
                if (mouseReleased->button == Mouse::Button::Left)
                {
                    game.isDraggingScrollbar = false;

                    if (game.isDraggingPiece)
                    {
                        float mouseX = static_cast<float>(mouseReleased->position.x);
                        float mouseY = static_cast<float>(mouseReleased->position.y);

                        // Check whether the piece was released on the board
                        auto boardPosition = getBoardPosition(
                            mouseX,
                            mouseY,
                            startX,
                            startY,
                            squareSize);

                        if (boardPosition)
                        {
                            int row = boardPosition->row;
                            int col = boardPosition->col;

                            if (row != game.draggedRow || col != game.draggedCol)
                            {
                                gameController.handleBoardClick(row, col);
                            }
                        }

                        // Reset drag state
                        game.isDraggingPiece = false;
                        game.draggedRow = -1;
                        game.draggedCol = -1;
                    }
                }
            }

            // Mouse moved
            if (const auto* mouseMoved = event->getIf<Event::MouseMoved>())
            {
                if (game.isDraggingPiece)
                {
                    game.dragMouseX = static_cast<float>(mouseMoved->position.x);
                    game.dragMouseY = static_cast<float>(mouseMoved->position.y);
                }

                if (game.isDraggingScrollbar)
                {
                    int totalRows = static_cast<int>(game.moveHistory.size());
                    int maxScroll = totalRows - visibleRows;

                    if (maxScroll > 0)
                    {
                        float thumbHeight = max(
                            30.0f,
                            historyContentHeight * visibleRows / totalRows);

                        float newThumbTop =
                            static_cast<float>(mouseMoved->position.y) -
                            game.scrollbarDragOffset;

                        float minThumbTop = historyContentTop;

                        float maxThumbTop =
                            historyContentTop +
                            historyContentHeight -
                            thumbHeight;

                        newThumbTop = max(
                            minThumbTop,
                            min(newThumbTop, maxThumbTop));

                        float scrollRatio =
                            (newThumbTop - historyContentTop) /
                            (historyContentHeight - thumbHeight);

                        game.historyScrollOffset =
                            static_cast<int>(scrollRatio * maxScroll + 0.5f);
                    }
                }
            }

            // Keyboard input
            if (const auto* keyPressed = event->getIf<Event::KeyPressed>())
            {
                // Pawn promotion
                if (game.isPromotionPending)
                {
                    char promotionChoice = '.';

                    switch (keyPressed->code)
                    {
                    case Keyboard::Key::Q:
                        promotionChoice = 'Q';
                        break;
                    case Keyboard::Key::R:
                        promotionChoice = 'R';
                        break;
                    case Keyboard::Key::B:
                        promotionChoice = 'B';
                        break;
                    case Keyboard::Key::N:
                        promotionChoice = 'N';
                        break;
                    }

                    if (promotionChoice != '.')
                    {
                        gameController.handlePromotion(promotionChoice);
                    }
                }

                // Draw claims
                if (keyPressed->code == Keyboard::Key::D)
                {
                    gameController.handleDrawClaim('D');
                }

                if (keyPressed->code == Keyboard::Key::T)
                {
                    gameController.handleDrawClaim('T');
                }
            }
        }

        // Clear saved game after game over
        if (game.gameOver && !game.saveFileCleared)
        {
            SaveManager::clearSavedGame();
            game.saveFileCleared = true;

            cout << "Saved game cleared!" << '\n';
        }

        // Update move history scrolling
        int currentMoveCount = static_cast<int>(game.moveHistory.size());

        int previousMaxScroll = max(
            0,
            game.previousMoveCount - visibleRows);

        if (
            currentMoveCount > game.previousMoveCount &&
            game.historyScrollOffset >= previousMaxScroll)
        {
            game.historyScrollOffset = max(
                0,
                currentMoveCount - visibleRows);
        }

        game.previousMoveCount = currentMoveCount;

        int maxScroll = max(
            0,
            currentMoveCount - visibleRows);

        game.historyScrollOffset = max(
            0,
            min(game.historyScrollOffset, maxScroll));

        // Render
        window.clear(Color(40, 40, 40));

        ui.drawPanelsAndHistory(
            window,
            game,
            visibleRows);

        BoardRenderer::drawBoard(
            window,
            game.board,
            textures,
            game.possibleMoves,
            game.isPieceSelected,
            game.selectedRow,
            game.selectedCol,
            squareSize,
            startX,
            startY,
            game.isDraggingPiece,
            game.draggedRow,
            game.draggedCol,
            game.dragMouseX,
            game.dragMouseY,
            game.hasLastMove,
            game.lastMove);

        ui.drawPlayerInfo(window, game);
        ui.drawClaimDrawButton(window);

        BoardRenderer::drawCapturedPieces(
            window,
            game.whiteCapturedPieces,
            textures,
            15.0f,
            755.0f);

        BoardRenderer::drawCapturedPieces(
            window,
            game.blackCapturedPieces,
            textures,
            15.0f,
            55.0f);

        BoardRenderer::drawCoordinates(
            window,
            font,
            squareSize,
            startX,
            startY);

        ui.drawLoadConfirmation(
            window,
            showLoadConfirmation);

        // Draw pawn promotion popup
        if (game.isPromotionPending)
        {
            ui.drawPromotionPopup(window);
        }

        // Draw game-over popup
        ui.drawGameOverPopup(window, game);

        window.display();
    }

    return 0;
}
