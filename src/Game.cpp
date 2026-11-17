
#include "Game.h"

#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <cstdlib>

#include "Board.h"
#include "MoveValidator.h"
#include "MoveNotation.h"
#include "GameState.h"
#include "PositionKey.h"

using namespace std;

int getPieceValue(char piece)
{
    switch (tolower(static_cast<unsigned char>(piece)))
    {
    case 'p':
        return 1;
    case 'n':
        return 3;
    case 'b':
        return 3;
    case 'r':
        return 5;
    case 'q':
        return 9;
    default:
        return 0;
    }
}

void addMoveToHistory(
    vector<pair<string, string>> &moveHistory,
    char movedPiece,
    string notation)
{
    bool isWhite = movedPiece >= 'A' && movedPiece <= 'Z';

    if (isWhite)
    {
        moveHistory.push_back({notation, ""});
    }
    else
    {
        if (moveHistory.empty() || !moveHistory.back().second.empty())
            moveHistory.push_back({"", notation});
        else
            moveHistory.back().second = notation;
    }
}

string getGameOverMessage(const GameState& state, bool whiteWon)
{
    if (state.isCheckmate)
        return whiteWon ? "Checkmate! White wins." : "Checkmate! Black wins.";

    if (state.isStalemate)
        return "Draw by stalemate!";

    if (state.isDrawByInsufficientMaterial)
        return "Draw by insufficient material!";

    if (state.isDrawBySeventyFiveMoveRule)
        return "Draw by the 75-move rule!";

    return "";
}

Game::Game(GameData &gameData)
    : data(gameData)
{

    data.positionOccurrences[getPositionKey(data.board, data.whiteTurn)] = 1;
}

void Game::reset()
{
    data.board = Board();

    data.isPieceSelected = false;
    data.selectedRow = -1;
    data.selectedCol = -1;
    data.possibleMoves.clear();

    data.whiteCapturedPieces.clear();
    data.blackCapturedPieces.clear();

    data.moveHistory.clear();

    data.historyScrollOffset = 0;
    data.previousMoveCount = 0;
    data.isDraggingScrollbar = false;
    data.scrollbarDragOffset = 0.0f;

    data.isDraggingPiece = false;

    data.draggedRow = -1;
    data.draggedCol = -1;

    data.dragMouseX = 0.0f;
    data.dragMouseY = 0.0f;

    data.whiteMaterial = 0;
    data.blackMaterial = 0;

    data.gameOver = false;
    data.saveFileCleared = false;
    data.gameOverMessage.clear();
    data.whiteTurn = true;

    data.positionOccurrences.clear();
    data.positionOccurrences[getPositionKey(data.board, data.whiteTurn)] = 1;

    data.isPromotionPending = false;
    data.promotionIsWhite = false;
    data.promotionRow = -1;
    data.promotionCol = -1;

    data.pendingMove = Move{};
    data.pendingMovedPiece = '.';
    data.pendingCapturedPiece = '.';
    data.pendingIsEnPassant = false;

    cout << "New game started!" << '\n';
}


void Game::handlePromotion(char promotionChoice)
{
    if (!data.isPromotionPending)
        return;

    char promotedPiece = data.promotionIsWhite
        ? promotionChoice
        : static_cast<char>(promotionChoice + ('a' - 'A'));

    data.board.squares[data.promotionRow][data.promotionCol] =
        promotedPiece;

    GameState state = getGameState(
        data.board,
        !data.promotionIsWhite);

    string notation = getMoveNotation(
        data.pendingMovedPiece,
        data.pendingCapturedPiece,
        data.pendingMove,
        state,
        data.pendingIsEnPassant);

    size_t suffixPosition = notation.find_last_of("+#");

    if (suffixPosition == string::npos)
        notation += "=" + string(1, promotionChoice);
    else
        notation.insert(
            suffixPosition,
            "=" + string(1, promotionChoice));

    addMoveToHistory(
        data.moveHistory,
        data.pendingMovedPiece,
        notation);

    cout << notation << '\n';

    // Update the game-over state after promotion.
    if (state.isGameOver)
    {
        data.gameOver = true;

        bool whiteWon =
            data.pendingMovedPiece >= 'A' &&
            data.pendingMovedPiece <= 'Z';

        data.gameOverMessage = getGameOverMessage(
            state,
            whiteWon);
    }

    if (state.isCheckmate)
        cout << "Checkmate!\n";

    if (state.isStalemate)
        cout << "Stalemate!\n";

    if (state.isDrawByInsufficientMaterial)
        cout << "Draw by insufficient material!\n";

    data.isPromotionPending = false;
    data.promotionRow = -1;
    data.promotionCol = -1;

    data.whiteTurn = !data.whiteTurn;

    data.positionOccurrences[getPositionKey(data.board, data.whiteTurn)]++;
}


void Game::handleDrawClaim(char key)
{
    if (data.isPromotionPending || data.gameOver)
        return;

    if (key == 'D')
    {
        if (data.board.halfMoveClock >= 100)
        {
            data.gameOver = true;
            data.gameOverMessage = "Draw by the 50-move rule!";

            cout << "Draw claimed by the 50-move rule!" << '\n';
        }
    }

    if (key == 'T')
    {
        string currentPosition = getPositionKey(
            data.board,
            data.whiteTurn);

        if (data.positionOccurrences[currentPosition] >= 3)
        {
            data.gameOver = true;
            data.gameOverMessage = "Draw by threefold repetition!";

            cout << "Draw claimed by threefold repetition!" << '\n';
        }
    }
}

void Game::handleBoardClick(int row, int col)
{
    Board &board = data.board;

    char piece = board.squares[row][col];

    if (!data.isPieceSelected)
    {
        bool pieceIsWhite = piece >= 'A' && piece <= 'Z';

        if (piece != '.' && pieceIsWhite == data.whiteTurn)
        {
            data.isPieceSelected = true;
            data.selectedRow = row;
            data.selectedCol = col;

            data.possibleMoves.clear();

            for (int r = 0; r < 8; r++)
            {
                for (int c = 0; c < 8; c++)
                {
                    Move move{
                        data.selectedRow,
                        data.selectedCol,
                        r,
                        c};

                    if (isValidMove(board, move))
                        data.possibleMoves.push_back(move);
                }
            }
        }
    }
    else
    {
        char selectedPiece =
            board.squares[data.selectedRow][data.selectedCol];

        bool selectedIsWhite =
            selectedPiece >= 'A' && selectedPiece <= 'Z';

        bool targetIsWhite =
            piece >= 'A' && piece <= 'Z';

        if (piece != '.' && selectedIsWhite == targetIsWhite)
        {
            if (row == data.selectedRow && col == data.selectedCol)
            {
                data.isPieceSelected = false;
                data.selectedRow = -1;
                data.selectedCol = -1;
                data.possibleMoves.clear();
            }
            else
            {
                data.selectedRow = row;
                data.selectedCol = col;

                data.possibleMoves.clear();

                for (int r = 0; r < 8; r++)
                {
                    for (int c = 0; c < 8; c++)
                    {
                        Move move{
                            data.selectedRow,
                            data.selectedCol,
                            r,
                            c};

                        if (isValidMove(board, move))
                            data.possibleMoves.push_back(move);
                    }
                }
            }
        }
        else
        {
            Move move{
                data.selectedRow,
                data.selectedCol,
                row,
                col};

            if (isValidMove(board, move))
            {
                bool opponentIsWhite =
                    selectedPiece >= 'a' && selectedPiece <= 'z';

                char movedPiece = selectedPiece;
                char capturedPiece = board.squares[row][col];

                if (selectedPiece == 'K')
                    board.whiteKingMoved = true;

                if (selectedPiece == 'k')
                    board.blackKingMoved = true;

                if (selectedPiece == 'R' && data.selectedRow == 7)
                {
                    if (data.selectedCol == 0)
                        board.whiteQueensideRookMoved = true;

                    if (data.selectedCol == 7)
                        board.whiteKingsideRookMoved = true;
                }

                if (selectedPiece == 'r' && data.selectedRow == 0)
                {
                    if (data.selectedCol == 0)
                        board.blackQueensideRookMoved = true;

                    if (data.selectedCol == 7)
                        board.blackKingsideRookMoved = true;
                }

                if (capturedPiece == 'R' && row == 7)
                {
                    if (col == 0)
                        board.whiteQueensideRookMoved = true;

                    if (col == 7)
                        board.whiteKingsideRookMoved = true;
                }

                if (capturedPiece == 'r' && row == 0)
                {
                    if (col == 0)
                        board.blackQueensideRookMoved = true;

                    if (col == 7)
                        board.blackKingsideRookMoved = true;
                }

                bool isEnPassant =
                    (movedPiece == 'P' || movedPiece == 'p') &&
                    data.selectedCol != col &&
                    board.squares[row][col] == '.' &&
                    data.selectedRow == board.enPassantRow &&
                    col == board.enPassantCol;

                char pieceCaptured = isEnPassant
                                         ? board.squares[data.selectedRow][col]
                                         : capturedPiece;

                if (pieceCaptured != '.')
                {
                    if (movedPiece >= 'A' && movedPiece <= 'Z')
                        data.whiteCapturedPieces.push_back(pieceCaptured);
                    else
                        data.blackCapturedPieces.push_back(pieceCaptured);
                }

                data.whiteMaterial = 0;
                data.blackMaterial = 0;

                for (char captured : data.whiteCapturedPieces)
                    data.whiteMaterial += getPieceValue(captured);

                for (char captured : data.blackCapturedPieces)
                    data.blackMaterial += getPieceValue(captured);

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
                    abs(row - data.selectedRow) == 2;

                board.enPassantRow = -1;
                board.enPassantCol = -1;

                if (isPawnDoubleStep)
                {
                    board.enPassantRow = row;
                    board.enPassantCol = col;
                }

                bool isCastling =
                    (movedPiece == 'K' || movedPiece == 'k') &&
                    data.selectedCol == 4 &&
                    row == data.selectedRow &&
                    abs(col - data.selectedCol) == 2;

                if (isCastling)
                {
                    int rookFromCol = (col == 6) ? 7 : 0;
                    int rookToCol = (col == 6) ? 5 : 3;

                    board.squares[row][col] = movedPiece;
                    board.squares[data.selectedRow][data.selectedCol] = '.';

                    board.squares[row][rookToCol] =
                        board.squares[row][rookFromCol];

                    board.squares[row][rookFromCol] = '.';

                    if (movedPiece == 'K')
                    {
                        if (col == 6)
                            board.whiteKingsideRookMoved = true;
                        else
                            board.whiteQueensideRookMoved = true;
                    }
                    else
                    {
                        if (col == 6)
                            board.blackKingsideRookMoved = true;
                        else
                            board.blackQueensideRookMoved = true;
                    }
                }
                else
                {
                    if (isEnPassant)
                        board.squares[data.selectedRow][col] = '.';

                    board.squares[row][col] = movedPiece;
                    board.squares[data.selectedRow][data.selectedCol] = '.';
                }

                bool isPawnPromotion =
                    (movedPiece == 'P' && row == 0) ||
                    (movedPiece == 'p' && row == 7);

                if (isPawnPromotion)
                {
                    data.isPromotionPending = true;
                    data.promotionIsWhite = (movedPiece == 'P');

                    data.promotionRow = row;
                    data.promotionCol = col;

                    data.pendingMove = move;
                    data.pendingMovedPiece = movedPiece;
                    data.pendingCapturedPiece = capturedPiece;
                    data.pendingIsEnPassant = isEnPassant;

                    cout << "Pawn promotion! Press Q, R, B, or N.\n";
                }

                if (!isPawnPromotion)
                {
                    GameState state = getGameState(
                        board,
                        opponentIsWhite);

                    if (state.isGameOver)
                    {
                        data.gameOver = true;

                        bool whiteWon = movedPiece >= 'A' && movedPiece <= 'Z';

                        data.gameOverMessage = getGameOverMessage(
                            state,
                            whiteWon);
                    }

                    string notation = getMoveNotation(
                        movedPiece,
                        capturedPiece,
                        move,
                        state,
                        isEnPassant);

                    addMoveToHistory(
                        data.moveHistory,
                        movedPiece,
                        notation);

                    cout << notation << '\n';

                    if (state.isStalemate)
                        cout << "Stalemate!" << '\n';

                    if (state.isDrawByInsufficientMaterial)
                        cout << "Draw by insufficient material!" << '\n';

                    if (state.canClaimDrawByFiftyMoveRule)
                        cout << "50-move rule: Draw can be claimed!" << '\n';

                    if (state.isDrawBySeventyFiveMoveRule)
                    {
                        data.gameOver = true;
                        data.gameOverMessage = "Draw by the 75-move rule!";

                        cout << "Draw by the 75-move rule!" << '\n';
                    }

                    data.whiteTurn = !data.whiteTurn;

                    data.positionOccurrences[getPositionKey(board, data.whiteTurn)]++;
                }

                data.isPieceSelected = false;
                data.selectedRow = -1;
                data.selectedCol = -1;

                data.possibleMoves.clear();
            }
            else
            {
                cout << "Invalid move!\n";
            }
        }
    }
}
