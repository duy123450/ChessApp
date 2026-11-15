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

using namespace sf;
using namespace std;

int getPieceValue(char piece) {
    switch (tolower(static_cast<unsigned char>(piece))) {
        case 'p': return 1;
        case 'n': return 3;
        case 'b': return 3;
        case 'r': return 5;
        case 'q': return 9;
        default: return 0;
    }
}

void addMoveToHistory(
    vector<pair<string, string>>& moveHistory,
    char movedPiece,
    string notation
) {
    bool isWhite = movedPiece >= 'A' && movedPiece <= 'Z';

    if (isWhite) {
        moveHistory.push_back({ notation, "" });
    }
    else {
        if (moveHistory.empty() || !moveHistory.back().second.empty())
            moveHistory.push_back({ "", notation });
        else
            moveHistory.back().second = notation;
    }
}

void resetGame(GameData& game) {
    game.board = Board();

    game.isPieceSelected = false;
    game.selectedRow = -1;
    game.selectedCol = -1;
    game.possibleMoves.clear();

    game.whiteCapturedPieces.clear();
    game.blackCapturedPieces.clear();

    game.moveHistory.clear();

    game.historyScrollOffset = 0;
    game.previousMoveCount = 0;
    game.isDraggingScrollbar = false;
    game.scrollbarDragOffset = 0.0f;

    game.whiteMaterial = 0;
    game.blackMaterial = 0;

    game.gameOver = false;
    game.saveFileCleared = false;
    game.whiteTurn = true;

    game.positionOccurrences.clear();
    game.positionOccurrences[getPositionKey(game.board, game.whiteTurn)] = 1;

    game.isPromotionPending = false;
    game.promotionIsWhite = false;
    game.promotionRow = -1;
    game.promotionCol = -1;

    game.pendingMove = Move{};
    game.pendingMovedPiece = '.';
    game.pendingCapturedPiece = '.';
    game.pendingIsEnPassant = false;

    cout << "New game started!" << '\n';
}

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
                            resetGame(game);
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
                        char promotedPiece = promotionIsWhite
                            ? promotionChoice
                            : static_cast<char>(promotionChoice + ('a' - 'A'));

                        board.squares[promotionRow][promotionCol] = promotedPiece;

                        GameState state = getGameState(board, !promotionIsWhite);

                        string notation = getMoveNotation(
                            pendingMovedPiece,
                            pendingCapturedPiece,
                            pendingMove,
                            state,
                            pendingIsEnPassant
                        );

                        size_t suffixPosition = notation.find_last_of("+#");

                        if (suffixPosition == string::npos)
                            notation += "=" + string(1, promotionChoice);
                        else
                            notation.insert(suffixPosition, "=" + string(1, promotionChoice));

                        addMoveToHistory(moveHistory, pendingMovedPiece, notation);

                        cout << notation << '\n';

                        if (state.isStalemate)
                            cout << "Stalemate!\n";

                        isPromotionPending = false;
                        promotionRow = -1;
                        promotionCol = -1;

                        whiteTurn = !whiteTurn;

                        positionOccurrences[getPositionKey(board, whiteTurn)]++;
                    }
                }

                if (
                    keyPressed->code == Keyboard::Key::D &&
                    !isPromotionPending && !gameOver
                ) {
                    if (board.halfMoveClock >= 100) {
                        gameOver = true;
                        cout << "Draw claimed by the 50-move rule!" << '\n';
                    }
                }

                if (
                    keyPressed->code == Keyboard::Key::T &&
                    !isPromotionPending && !gameOver
                ) {
                    string currentPosition = getPositionKey(board, whiteTurn);

                    if (positionOccurrences[currentPosition] >= 3) {
                        gameOver = true;
                        cout << "Draw claimed by threefold repetition!" << '\n';
                    }
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

                        char piece = board.squares[row][col];

                        if (!isPieceSelected) {
                            bool pieceIsWhite = piece >= 'A' && piece <= 'Z';

                            if (piece != '.' && pieceIsWhite == whiteTurn) {
                                isPieceSelected = true;
                                selectedRow = row;
                                selectedCol = col;

                                possibleMoves.clear();

                                for (int r = 0; r < 8; r++) {
                                    for (int c = 0; c < 8; c++) {
                                        Move move{
                                            selectedRow,
                                            selectedCol,
                                            r,
                                            c
                                        };

                                        if (isValidMove(board, move))
                                            possibleMoves.push_back(move);
                                    }
                                }
                            }
                        }
                        else {
                            char selectedPiece = board.squares[selectedRow][selectedCol];

                            bool selectedIsWhite =
                                selectedPiece >= 'A' && selectedPiece <= 'Z';

                            bool targetIsWhite =
                                piece >= 'A' && piece <= 'Z';

                            if (piece != '.' && selectedIsWhite == targetIsWhite) {
                                if (row == selectedRow && col == selectedCol) {
                                    isPieceSelected = false;
                                    selectedRow = -1;
                                    selectedCol = -1;
                                    possibleMoves.clear();
                                }
                                else {
                                    selectedRow = row;
                                    selectedCol = col;

                                    possibleMoves.clear();

                                    for (int r = 0; r < 8; r++) {
                                        for (int c = 0; c < 8; c++) {
                                            Move move{
                                                selectedRow,
                                                selectedCol,
                                                r,
                                                c
                                            };

                                            if (isValidMove(board, move))
                                                possibleMoves.push_back(move);
                                        }
                                    }
                                }
                            }
                            else {
                                Move move{
                                    selectedRow,
                                    selectedCol,
                                    row,
                                    col
                                };

                                if (isValidMove(board, move)) {
                                    bool opponentIsWhite =
                                        selectedPiece >= 'a' && selectedPiece <= 'z';

                                    char movedPiece = selectedPiece;
                                    char capturedPiece = board.squares[row][col];

                                    if (selectedPiece == 'K')
                                        board.whiteKingMoved = true;

                                    if (selectedPiece == 'k')
                                        board.blackKingMoved = true;

                                    if (selectedPiece == 'R' && selectedRow == 7) {
                                        if (selectedCol == 0)
                                            board.whiteQueensideRookMoved = true;

                                        if (selectedCol == 7)
                                            board.whiteKingsideRookMoved = true;
                                    }

                                    if (selectedPiece == 'r' && selectedRow == 0) {
                                        if (selectedCol == 0)
                                            board.blackQueensideRookMoved = true;

                                        if (selectedCol == 7)
                                            board.blackKingsideRookMoved = true;
                                    }

                                    if (capturedPiece == 'R' && row == 7) {
                                        if (col == 0)
                                            board.whiteQueensideRookMoved = true;

                                        if (col == 7)
                                            board.whiteKingsideRookMoved = true;
                                    }

                                    if (capturedPiece == 'r' && row == 0) {
                                        if (col == 0)
                                            board.blackQueensideRookMoved = true;

                                        if (col == 7)
                                            board.blackKingsideRookMoved = true;
                                    }

                                    bool isEnPassant =
                                        (movedPiece == 'P' || movedPiece == 'p') &&
                                        selectedCol != col &&
                                        board.squares[row][col] == '.' &&
                                        selectedRow == board.enPassantRow &&
                                        col == board.enPassantCol;

                                    char pieceCaptured = isEnPassant
                                        ? board.squares[selectedRow][col]
                                        : capturedPiece;

                                    if (pieceCaptured != '.') {
                                        if (movedPiece >= 'A' && movedPiece <= 'Z')
                                            whiteCapturedPieces.push_back(pieceCaptured);
                                        else
                                            blackCapturedPieces.push_back(pieceCaptured);
                                    }

                                    whiteMaterial = 0;
                                    blackMaterial = 0;

                                    for (char captured : whiteCapturedPieces)
                                        whiteMaterial += getPieceValue(captured);

                                    for (char captured : blackCapturedPieces)
                                        blackMaterial += getPieceValue(captured);

                                    bool isPawnMove =
                                        movedPiece == 'P' || movedPiece == 'p';

                                    bool isCapture =
                                        capturedPiece != '.' || isEnPassant;

                                    if (isPawnMove || isCapture)
                                        board.halfMoveClock = 0;
                                    else
                                        board.halfMoveClock++;

                                    bool isPawnDoubleStep =
                                        (movedPiece == 'P' || movedPiece == 'p') &&
                                        abs(row - selectedRow) == 2;

                                    board.enPassantRow = -1;
                                    board.enPassantCol = -1;

                                    if (isPawnDoubleStep) {
                                        board.enPassantRow = row;
                                        board.enPassantCol = col;
                                    }

                                    bool isCastling =
                                        (movedPiece == 'K' || movedPiece == 'k') &&
                                        selectedCol == 4 &&
                                        row == selectedRow &&
                                        abs(col - selectedCol) == 2;

                                    if (isCastling) {
                                        int rookFromCol = (col == 6) ? 7 : 0;
                                        int rookToCol = (col == 6) ? 5 : 3;

                                        board.squares[row][col] = movedPiece;
                                        board.squares[selectedRow][selectedCol] = '.';

                                        board.squares[row][rookToCol] =
                                            board.squares[row][rookFromCol];

                                        board.squares[row][rookFromCol] = '.';

                                        if (movedPiece == 'K') {
                                            if (col == 6)
                                                board.whiteKingsideRookMoved = true;
                                            else
                                                board.whiteQueensideRookMoved = true;
                                        }
                                        else {
                                            if (col == 6)
                                                board.blackKingsideRookMoved = true;
                                            else
                                                board.blackQueensideRookMoved = true;
                                        }
                                    }
                                    else {
                                        if (isEnPassant)
                                            board.squares[selectedRow][col] = '.';

                                        board.squares[row][col] = movedPiece;
                                        board.squares[selectedRow][selectedCol] = '.';
                                    }

                                    bool isPawnPromotion =
                                        (movedPiece == 'P' && row == 0) ||
                                        (movedPiece == 'p' && row == 7);

                                    if (isPawnPromotion) {
                                        isPromotionPending = true;
                                        promotionIsWhite = (movedPiece == 'P');

                                        promotionRow = row;
                                        promotionCol = col;

                                        pendingMove = move;
                                        pendingMovedPiece = movedPiece;
                                        pendingCapturedPiece = capturedPiece;
                                        pendingIsEnPassant = isEnPassant;

                                        cout << "Pawn promotion! Press Q, R, B, or N.\n";
                                    }

                                    if (!isPawnPromotion) {
                                        GameState state = getGameState(board, opponentIsWhite);

                                        if (state.isGameOver)
                                            gameOver = true;

                                        string notation = getMoveNotation(
                                            movedPiece,
                                            capturedPiece,
                                            move,
                                            state,
                                            isEnPassant
                                        );

                                        addMoveToHistory(moveHistory, movedPiece, notation);

                                        cout << notation << '\n';

                                        if (state.isStalemate)
                                            cout << "Stalemate!" << '\n';

                                        if (state.isDrawByInsufficientMaterial)
                                            cout << "Draw by insufficient material!" << '\n';

                                        if (state.canClaimDrawByFiftyMoveRule)
                                            cout << "50-move rule: Draw can be claimed!" << '\n';

                                        if (state.isDrawBySeventyFiveMoveRule) {
                                            gameOver = true;
                                            cout << "Draw by the 75-move rule!" << '\n';
                                        }

                                        whiteTurn = !whiteTurn;

                                        positionOccurrences[getPositionKey(board, whiteTurn)]++;
                                    }

                                    isPieceSelected = false;
                                    selectedRow = -1;
                                    selectedCol = -1;

                                    possibleMoves.clear();
                                }
                                else {
                                    cout << "Invalid move!\n";
                                }
                            }
                        }
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
