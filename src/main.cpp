#include <SFML/Graphics.hpp>
#include <optional>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <cctype>
#include <string>
#include <utility>
#include <unordered_map>

#include "Board.h"
#include "PieceTextures.h"
#include "MoveValidator.h"
#include "MoveNotation.h"
#include "GameState.h"
#include "PositionKey.h"
#include "GameData.h"
#include "SaveManager.h"
#include "BoardRenderer.h"
#include "UI.h"
#include "Game.h"

using namespace sf;
using namespace std;

int main() {
    RenderWindow window(
        VideoMode({ 1010, 800 }),
        "Chess App"
    );

    const int boardSize = 8;
    const float squareSize = 75.0f;

    const float startX = 10.0f;
    const float startY = 100.0f;

    // Game data and references
    GameData game;
    Game gameController(game);

    Board& board = game.board;

    unordered_map<string, int>& positionOccurrences =
        game.positionOccurrences;

    bool& isPieceSelected = game.isPieceSelected;
    int& selectedRow = game.selectedRow;
    int& selectedCol = game.selectedCol;

    vector<Move>& possibleMoves = game.possibleMoves;

    vector<char>& whiteCapturedPieces = game.whiteCapturedPieces;
    vector<char>& blackCapturedPieces = game.blackCapturedPieces;

    vector<pair<string, string>>& moveHistory = game.moveHistory;

    int& historyScrollOffset = game.historyScrollOffset;
    int& previousMoveCount = game.previousMoveCount;

    bool& isDraggingScrollbar = game.isDraggingScrollbar;
    float& scrollbarDragOffset = game.scrollbarDragOffset;

    int& whiteMaterial = game.whiteMaterial;
    int& blackMaterial = game.blackMaterial;

    bool& gameOver = game.gameOver;
    bool& saveFileCleared = game.saveFileCleared;
    bool& whiteTurn = game.whiteTurn;

    bool& isPromotionPending = game.isPromotionPending;
    bool& promotionIsWhite = game.promotionIsWhite;

    int& promotionRow = game.promotionRow;
    int& promotionCol = game.promotionCol;

    Move& pendingMove = game.pendingMove;

    char& pendingMovedPiece = game.pendingMovedPiece;
    char& pendingCapturedPiece = game.pendingCapturedPiece;

    bool& pendingIsEnPassant = game.pendingIsEnPassant;

    const int visibleRows = 20;

    const float historyContentTop = 100.0f;
    const float historyContentHeight = 490.0f;

    const float scrollbarX = 980.0f;
    const float scrollbarWidth = 10.0f;

    positionOccurrences[getPositionKey(board, whiteTurn)] = 1;

    bool showLoadConfirmation = false;

    PieceTextures textures;
    loadPieceTextures(textures);

    Font font;

    if (!font.openFromFile("assets/fonts/ARIAL.TTF"))
        return -1;

    UI ui(font);

    while (window.isOpen()) {
        while (const optional event = window.pollEvent()) {
            if (event->is<Event::Closed>())
                window.close();

            if (const auto* wheel = event->getIf<Event::MouseWheelScrolled>()) {
                int mouseX = wheel->position.x;
                int mouseY = wheel->position.y;

                if (
                    mouseX >= 620 && mouseX <= 1000 &&
                    mouseY >= historyContentTop &&
                    mouseY < historyContentTop + historyContentHeight
                ) {
                    int maxScroll = max(
                        0,
                        static_cast<int>(moveHistory.size()) - visibleRows
                    );

                    historyScrollOffset -= static_cast<int>(wheel->delta * 3.0f);

                    historyScrollOffset = max(
                        0,
                        min(historyScrollOffset, maxScroll)
                    );
                }
            }

            if (const auto* mousePressed = event->getIf<Event::MouseButtonPressed>()) {
                if (mousePressed->button == Mouse::Button::Left) {
                    float mouseX = static_cast<float>(mousePressed->position.x);
                    float mouseY = static_cast<float>(mousePressed->position.y);

                    if (showLoadConfirmation) {
                        if (
                            mouseX >= 390.0f && mouseX <= 490.0f &&
                            mouseY >= 425.0f && mouseY <= 465.0f
                        ) {
                            showLoadConfirmation = false;

                            if (!SaveManager::loadGame(game))
                                cout << "Failed to load game!" << '\n';
                        }
                        else if (
                            mouseX >= 520.0f && mouseX <= 620.0f &&
                            mouseY >= 425.0f && mouseY <= 465.0f
                        ) {
                            showLoadConfirmation = false;
                            cout << "Load cancelled!" << '\n';
                        }
                    }
                    else {
                        if (
                            mouseX >= 620.0f && mouseX <= 750.0f &&
                            mouseY >= 620.0f && mouseY <= 660.0f
                        ) {
                            gameController.reset();
                        }

                        if (
                            mouseX >= 760.0f && mouseX <= 870.0f &&
                            mouseY >= 620.0f && mouseY <= 660.0f
                        ) {
                            SaveManager::saveGame(game);
                        }

                        if (
                            mouseX >= 880.0f && mouseX <= 990.0f &&
                            mouseY >= 620.0f && mouseY <= 660.0f
                        ) {
                            showLoadConfirmation = true;
                        }

                        // Scrollbar click and drag handling (one copy only)
                        int totalRows = static_cast<int>(moveHistory.size());

                        if (
                            totalRows > visibleRows &&
                            mouseX >= scrollbarX &&
                            mouseX <= scrollbarX + scrollbarWidth &&
                            mouseY >= historyContentTop &&
                            mouseY <= historyContentTop + historyContentHeight
                        ) {
                            int maxScroll = totalRows - visibleRows;

                            float thumbHeight = max(
                                30.0f,
                                historyContentHeight * visibleRows / totalRows
                            );

                            float thumbTop =
                                historyContentTop +
                                (static_cast<float>(historyScrollOffset) / maxScroll) *
                                (historyContentHeight - thumbHeight);

                            if (mouseY >= thumbTop && mouseY <= thumbTop + thumbHeight) {
                                isDraggingScrollbar = true;
                                scrollbarDragOffset = mouseY - thumbTop;
                            }
                            else {
                                if (mouseY < thumbTop)
                                    historyScrollOffset -= visibleRows;
                                else
                                    historyScrollOffset += visibleRows;

                                historyScrollOffset = max(
                                    0,
                                    min(historyScrollOffset, maxScroll)
                                );
                            }
                        }
                    }
                }
            }

            if (const auto* mouseReleased = event->getIf<Event::MouseButtonReleased>()) {
                if (mouseReleased->button == Mouse::Button::Left)
                    isDraggingScrollbar = false;
            }

            if (const auto* mouseMoved = event->getIf<Event::MouseMoved>()) {
                if (isDraggingScrollbar) {
                    int totalRows = static_cast<int>(moveHistory.size());
                    int maxScroll = totalRows - visibleRows;

                    if (maxScroll > 0) {
                        float thumbHeight = max(
                            30.0f,
                            historyContentHeight * visibleRows / totalRows
                        );

                        float newThumbTop =
                            static_cast<float>(mouseMoved->position.y) -
                            scrollbarDragOffset;

                        float minThumbTop = historyContentTop;
                        float maxThumbTop =
                            historyContentTop + historyContentHeight - thumbHeight;

                        newThumbTop = max(
                            minThumbTop,
                            min(newThumbTop, maxThumbTop)
                        );

                        float scrollRatio =
                            (newThumbTop - historyContentTop) /
                            (historyContentHeight - thumbHeight);

                        historyScrollOffset =
                            static_cast<int>(scrollRatio * maxScroll + 0.5f);
                    }
                }
            }

            if (const auto* keyPressed = event->getIf<Event::KeyPressed>()) {
                if (isPromotionPending) {
                    char promotionChoice = '.';

                    switch (keyPressed->code) {
                    case Keyboard::Key::Q: promotionChoice = 'Q'; break;
                    case Keyboard::Key::R: promotionChoice = 'R'; break;
                    case Keyboard::Key::B: promotionChoice = 'B'; break;
                    case Keyboard::Key::N: promotionChoice = 'N'; break;
                    }

                    if (promotionChoice != '.') {
                        gameController.handlePromotion(promotionChoice);
                    }
                }

                if (keyPressed->code == Keyboard::Key::D) {
                    gameController.handleDrawClaim('D');
                }

                if (keyPressed->code == Keyboard::Key::T) {
                    gameController.handleDrawClaim('T');
                }
            }

            if (const auto* mousePressed = event->getIf<Event::MouseButtonPressed>()) {
                if (
                    !showLoadConfirmation &&
                    !isPromotionPending &&
                    !gameOver &&
                    mousePressed->button == Mouse::Button::Left
                ) {
                    int mouseX = mousePressed->position.x;
                    int mouseY = mousePressed->position.y;

                    if (
                        mouseX >= startX &&
                        mouseX < startX + boardSize * squareSize &&
                        mouseY >= startY &&
                        mouseY < startY + boardSize * squareSize
                    ) {
                        int col = static_cast<int>((mouseX - startX) / squareSize);
                        int row = static_cast<int>((mouseY - startY) / squareSize);

                        gameController.handleBoardClick(row, col);
                    }
                }
            }
        }

        if (gameOver && !saveFileCleared) {
            SaveManager::clearSavedGame();
            saveFileCleared = true;

            cout << "Saved game cleared!" << '\n';
        }

        int currentMoveCount = static_cast<int>(moveHistory.size());

        int previousMaxScroll = max(
            0,
            previousMoveCount - visibleRows
        );

        if (
            currentMoveCount > previousMoveCount &&
            historyScrollOffset >= previousMaxScroll
        ) {
            historyScrollOffset = max(
                0,
                currentMoveCount - visibleRows
            );
        }

        previousMoveCount = currentMoveCount;

        int maxScroll = max(
            0,
            currentMoveCount - visibleRows
        );

        historyScrollOffset = max(
            0,
            min(historyScrollOffset, maxScroll)
        );

        window.clear(Color(40, 40, 40));

        ui.drawPanelsAndHistory(
            window,
            game,
            visibleRows
        );

        BoardRenderer::drawBoard(
            window,
            board,
            textures,
            possibleMoves,
            isPieceSelected,
            selectedRow,
            selectedCol,
            squareSize,
            startX,
            startY
        );

        ui.drawPlayerInfo(window, game);

        BoardRenderer::drawCapturedPieces(
            window,
            whiteCapturedPieces,
            textures,
            220.0f,
            725.0f
        );

        BoardRenderer::drawCapturedPieces(
            window,
            blackCapturedPieces,
            textures,
            220.0f,
            25.0f
        );

        BoardRenderer::drawCoordinates(
            window,
            font,
            squareSize,
            startX,
            startY
        );

        ui.drawLoadConfirmation(window, showLoadConfirmation);

        window.display();
    }

    return 0;
}
