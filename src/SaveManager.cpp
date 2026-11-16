
#include "SaveManager.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <cctype>

using namespace std;

void SaveManager::clearSavedGame()
{
    ofstream file("saved_game.txt", ios::trunc);
    file.close();
}

bool expectLine(ifstream &file, const string &expected)
{
    string line;

    while (getline(file, line))
    {
        if (!line.empty())
            return line == expected;
    }

    return false;
}

template <typename T>
bool readLabeledValue(
    ifstream &file,
    const string &expectedLabel,
    T &value)
{
    string line;

    if (!getline(file, line))
        return false;

    size_t separator = line.find(':');

    if (separator == string::npos)
        return false;

    string label = line.substr(0, separator);

    if (label != expectedLabel)
        return false;

    istringstream stream(line.substr(separator + 1));

    stream >> value;

    if (stream.fail())
        return false;

    stream >> ws;

    return stream.eof();
}

bool readLabeledString(
    ifstream &file,
    const string &expectedLabel,
    string &value)
{
    string line;

    if (!getline(file, line))
        return false;

    size_t separator = line.find(':');

    if (separator == string::npos)
        return false;

    if (line.substr(0, separator) != expectedLabel)
        return false;

    value = line.substr(separator + 1);

    if (!value.empty() && value[0] == ' ')
        value.erase(0, 1);

    return true;
}

int getSavedPieceValue(char piece)
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

void SaveManager::saveGame(const GameData &game)
{
    if (game.gameOver)
    {
        cout << "Cannot save: the game has already ended!" << '\n';
        return;
    }

    ofstream file("saved_game.txt", ios::trunc);

    if (!file.is_open())
    {
        cout << "Failed to open save file!" << '\n';
        return;
    }

    file << "========== CHESS SAVE FILE ==========\n";
    file << "Version: 1\n\n";

    file << "========== BOARD ==========\n";

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
            file << game.board.squares[row][col];

        file << '\n';
    }

    file << "\n========== CASTLING RIGHTS ==========\n";
    file << "White King Moved: " << game.board.whiteKingMoved << '\n';
    file << "Black King Moved: " << game.board.blackKingMoved << '\n';

    file << "White Kingside Rook Moved: "
         << game.board.whiteKingsideRookMoved << '\n';

    file << "White Queenside Rook Moved: "
         << game.board.whiteQueensideRookMoved << '\n';

    file << "Black Kingside Rook Moved: "
         << game.board.blackKingsideRookMoved << '\n';

    file << "Black Queenside Rook Moved: "
         << game.board.blackQueensideRookMoved << '\n';

    file << "\n========== SPECIAL MOVE DATA ==========\n";
    file << "En Passant Row: " << game.board.enPassantRow << '\n';
    file << "En Passant Column: " << game.board.enPassantCol << '\n';
    file << "Half Move Clock: " << game.board.halfMoveClock << '\n';

    file << "\n========== GAME TURN ==========\n";
    file << "White Turn: " << game.whiteTurn << '\n';

    file << "\n========== CAPTURED PIECES ==========\n";

    file << "White Captured Count: "
         << game.whiteCapturedPieces.size() << '\n';

    file << "White Captured: ";
    for (char piece : game.whiteCapturedPieces)
        file << piece;
    file << '\n';

    file << "Black Captured Count: "
         << game.blackCapturedPieces.size() << '\n';

    file << "Black Captured: ";
    for (char piece : game.blackCapturedPieces)
        file << piece;
    file << '\n';

    file << "\n========== MOVE HISTORY ==========\n";
    file << "Move Count: " << game.moveHistory.size() << '\n';

    for (const auto &move : game.moveHistory)
    {
        file << quoted(move.first) << ' '
             << quoted(move.second) << '\n';
    }

    file << "\n========== POSITION REPETITIONS ==========\n";
    file << "Position Count: " << game.positionOccurrences.size() << '\n';

    for (const auto &position : game.positionOccurrences)
    {
        file << quoted(position.first) << ' '
             << position.second << '\n';
    }

    file << "\n========== PAWN PROMOTION ==========\n";
    file << "Promotion Pending: " << game.isPromotionPending << '\n';
    file << "Promotion Is White: " << game.promotionIsWhite << '\n';

    file << "Promotion Row: " << game.promotionRow << '\n';
    file << "Promotion Column: " << game.promotionCol << '\n';

    file << "Pending Move From Row: " << game.pendingMove.fromRow << '\n';
    file << "Pending Move From Column: " << game.pendingMove.fromCol << '\n';
    file << "Pending Move To Row: " << game.pendingMove.toRow << '\n';
    file << "Pending Move To Column: " << game.pendingMove.toCol << '\n';

    file << "Pending Moved Piece: " << game.pendingMovedPiece << '\n';
    file << "Pending Captured Piece: " << game.pendingCapturedPiece << '\n';
    file << "Pending Is En Passant: " << game.pendingIsEnPassant << '\n';

    file.flush();

    if (!file)
    {
        cout << "Failed to save game!" << '\n';
        return;
    }

    file.close();

    cout << "Game saved successfully!" << '\n';
}

bool SaveManager::loadGame(GameData &game)
{
    ifstream file("saved_game.txt");

    if (!file.is_open())
    {
        cout << "Failed to open save file!" << '\n';
        return false;
    }

    SavedGameData data;

    if (!expectLine(file, "========== CHESS SAVE FILE =========="))
    {
        cout << "Invalid save file header!" << '\n';
        return false;
    }

    int version;

    if (!readLabeledValue(file, "Version", version) || version != 1)
    {
        cout << "Unsupported save file version!" << '\n';
        return false;
    }

    if (!expectLine(file, "========== BOARD =========="))
    {
        cout << "Missing board data!" << '\n';
        return false;
    }

    const string validPieces = "prnbqkPRNBQK.";

    for (int row = 0; row < 8; row++)
    {
        string line;

        if (!getline(file, line) || line.size() != 8)
        {
            cout << "Invalid board data!" << '\n';
            return false;
        }

        for (int col = 0; col < 8; col++)
        {
            if (validPieces.find(line[col]) == string::npos)
            {
                cout << "Invalid chess piece!" << '\n';
                return false;
            }

            data.board.squares[row][col] = line[col];
        }
    }

    if (!expectLine(file, "========== CASTLING RIGHTS =========="))
    {
        cout << "Missing castling data!" << '\n';
        return false;
    }

    int whiteKingMoved;
    int blackKingMoved;
    int whiteKingsideRookMoved;
    int whiteQueensideRookMoved;
    int blackKingsideRookMoved;
    int blackQueensideRookMoved;

    if (
        !readLabeledValue(file, "White King Moved", whiteKingMoved) ||
        !readLabeledValue(file, "Black King Moved", blackKingMoved) ||
        !readLabeledValue(file, "White Kingside Rook Moved", whiteKingsideRookMoved) ||
        !readLabeledValue(file, "White Queenside Rook Moved", whiteQueensideRookMoved) ||
        !readLabeledValue(file, "Black Kingside Rook Moved", blackKingsideRookMoved) ||
        !readLabeledValue(file, "Black Queenside Rook Moved", blackQueensideRookMoved))
    {
        cout << "Invalid castling data!" << '\n';
        return false;
    }

    if (
        (whiteKingMoved != 0 && whiteKingMoved != 1) ||
        (blackKingMoved != 0 && blackKingMoved != 1) ||
        (whiteKingsideRookMoved != 0 && whiteKingsideRookMoved != 1) ||
        (whiteQueensideRookMoved != 0 && whiteQueensideRookMoved != 1) ||
        (blackKingsideRookMoved != 0 && blackKingsideRookMoved != 1) ||
        (blackQueensideRookMoved != 0 && blackQueensideRookMoved != 1))
    {
        cout << "Invalid castling values!" << '\n';
        return false;
    }

    data.board.whiteKingMoved = whiteKingMoved;
    data.board.blackKingMoved = blackKingMoved;

    data.board.whiteKingsideRookMoved = whiteKingsideRookMoved;
    data.board.whiteQueensideRookMoved = whiteQueensideRookMoved;

    data.board.blackKingsideRookMoved = blackKingsideRookMoved;
    data.board.blackQueensideRookMoved = blackQueensideRookMoved;

    if (!expectLine(file, "========== SPECIAL MOVE DATA =========="))
    {
        cout << "Missing special move data!" << '\n';
        return false;
    }

    if (
        !readLabeledValue(file, "En Passant Row", data.board.enPassantRow) ||
        !readLabeledValue(file, "En Passant Column", data.board.enPassantCol) ||
        !readLabeledValue(file, "Half Move Clock", data.board.halfMoveClock))
    {
        cout << "Invalid special move data!" << '\n';
        return false;
    }

    if (
        data.board.enPassantRow < -1 || data.board.enPassantRow > 7 ||
        data.board.enPassantCol < -1 || data.board.enPassantCol > 7 ||
        (data.board.enPassantRow == -1) != (data.board.enPassantCol == -1) ||
        data.board.halfMoveClock < 0)
    {
        cout << "Special move data is out of range!" << '\n';
        return false;
    }

    if (!expectLine(file, "========== GAME TURN =========="))
    {
        cout << "Missing turn data!" << '\n';
        return false;
    }

    int savedWhiteTurn;

    if (
        !readLabeledValue(file, "White Turn", savedWhiteTurn) ||
        (savedWhiteTurn != 0 && savedWhiteTurn != 1))
    {
        cout << "Invalid turn data!" << '\n';
        return false;
    }

    data.whiteTurn = savedWhiteTurn;

    if (!expectLine(file, "========== CAPTURED PIECES =========="))
    {
        cout << "Missing captured pieces data!" << '\n';
        return false;
    }

    int whiteCapturedCount;
    int blackCapturedCount;

    string whiteCaptured;
    string blackCaptured;

    if (
        !readLabeledValue(file, "White Captured Count", whiteCapturedCount) ||
        !readLabeledString(file, "White Captured", whiteCaptured) ||
        !readLabeledValue(file, "Black Captured Count", blackCapturedCount) ||
        !readLabeledString(file, "Black Captured", blackCaptured))
    {
        cout << "Invalid captured pieces data!" << '\n';
        return false;
    }

    if (
        whiteCapturedCount < 0 || whiteCapturedCount > 64 ||
        blackCapturedCount < 0 || blackCapturedCount > 64 ||
        whiteCaptured.size() != static_cast<size_t>(whiteCapturedCount) ||
        blackCaptured.size() != static_cast<size_t>(blackCapturedCount))
    {
        cout << "Invalid captured pieces count!" << '\n';
        return false;
    }

    const string validCapturedPieces = "prnbqPRNBQ";

    for (char piece : whiteCaptured)
    {
        if (validCapturedPieces.find(piece) == string::npos)
        {
            cout << "Invalid captured piece!" << '\n';
            return false;
        }
    }

    for (char piece : blackCaptured)
    {
        if (validCapturedPieces.find(piece) == string::npos)
        {
            cout << "Invalid captured piece!" << '\n';
            return false;
        }
    }

    data.whiteCapturedPieces.assign(whiteCaptured.begin(), whiteCaptured.end());
    data.blackCapturedPieces.assign(blackCaptured.begin(), blackCaptured.end());

    if (!expectLine(file, "========== MOVE HISTORY =========="))
    {
        cout << "Missing move history!" << '\n';
        return false;
    }

    int moveCount;

    if (
        !readLabeledValue(file, "Move Count", moveCount) ||
        moveCount < 0 || moveCount > 10000)
    {
        cout << "Invalid move count!" << '\n';
        return false;
    }

    for (int i = 0; i < moveCount; i++)
    {
        string whiteMove;
        string blackMove;

        if (!(file >> quoted(whiteMove) >> quoted(blackMove)))
        {
            cout << "Invalid move history data!" << '\n';
            return false;
        }

        data.moveHistory.push_back({whiteMove, blackMove});
    }

    if (!expectLine(file, "========== POSITION REPETITIONS =========="))
    {
        cout << "Missing position repetition data!" << '\n';
        return false;
    }

    int positionCount;

    if (
        !readLabeledValue(file, "Position Count", positionCount) ||
        positionCount < 1 || positionCount > 10000)
    {
        cout << "Invalid position count!" << '\n';
        return false;
    }

    for (int i = 0; i < positionCount; i++)
    {
        string positionKey;
        int occurrenceCount;

        if (!(file >> quoted(positionKey) >> occurrenceCount))
        {
            cout << "Invalid position repetition data!" << '\n';
            return false;
        }

        if (positionKey.empty() || occurrenceCount < 1)
        {
            cout << "Invalid position repetition values!" << '\n';
            return false;
        }

        if (data.positionOccurrences.find(positionKey) != data.positionOccurrences.end())
        {
            cout << "Duplicate position key!" << '\n';
            return false;
        }

        data.positionOccurrences[positionKey] = occurrenceCount;
    }

    if (!expectLine(file, "========== PAWN PROMOTION =========="))
    {
        cout << "Missing pawn promotion data!" << '\n';
        return false;
    }

    int promotionPending;
    int promotionIsWhite;
    int pendingIsEnPassant;

    if (
        !readLabeledValue(file, "Promotion Pending", promotionPending) ||
        !readLabeledValue(file, "Promotion Is White", promotionIsWhite) ||
        !readLabeledValue(file, "Promotion Row", data.promotionRow) ||
        !readLabeledValue(file, "Promotion Column", data.promotionCol) ||
        !readLabeledValue(file, "Pending Move From Row", data.pendingMove.fromRow) ||
        !readLabeledValue(file, "Pending Move From Column", data.pendingMove.fromCol) ||
        !readLabeledValue(file, "Pending Move To Row", data.pendingMove.toRow) ||
        !readLabeledValue(file, "Pending Move To Column", data.pendingMove.toCol))
    {
        cout << "Invalid pawn promotion data!" << '\n';
        return false;
    }

    string movedPiece;
    string capturedPiece;

    if (
        !readLabeledString(file, "Pending Moved Piece", movedPiece) ||
        !readLabeledString(file, "Pending Captured Piece", capturedPiece) ||
        !readLabeledValue(file, "Pending Is En Passant", pendingIsEnPassant) ||
        movedPiece.size() != 1 ||
        capturedPiece.size() != 1)
    {
        cout << "Invalid pending piece data!" << '\n';
        return false;
    }

    if (
        (promotionPending != 0 && promotionPending != 1) ||
        (promotionIsWhite != 0 && promotionIsWhite != 1) ||
        (pendingIsEnPassant != 0 && pendingIsEnPassant != 1))
    {
        cout << "Invalid promotion boolean values!" << '\n';
        return false;
    }

    data.isPromotionPending = promotionPending;
    data.promotionIsWhite = promotionIsWhite;
    data.pendingIsEnPassant = pendingIsEnPassant;

    data.pendingMovedPiece = movedPiece[0];
    data.pendingCapturedPiece = capturedPiece[0];

    if (data.isPromotionPending)
    {
        if (
            data.promotionRow < 0 || data.promotionRow > 7 ||
            data.promotionCol < 0 || data.promotionCol > 7 ||
            data.pendingMove.fromRow < 0 || data.pendingMove.fromRow > 7 ||
            data.pendingMove.fromCol < 0 || data.pendingMove.fromCol > 7 ||
            data.pendingMove.toRow < 0 || data.pendingMove.toRow > 7 ||
            data.pendingMove.toCol < 0 || data.pendingMove.toCol > 7 ||
            data.promotionRow != data.pendingMove.toRow ||
            data.promotionCol != data.pendingMove.toCol)
        {
            cout << "Invalid pending promotion coordinates!" << '\n';
            return false;
        }

        if (
            (data.promotionIsWhite && data.pendingMovedPiece != 'P') ||
            (!data.promotionIsWhite && data.pendingMovedPiece != 'p'))
        {
            cout << "Invalid pending promotion piece!" << '\n';
            return false;
        }

        char boardPiece = data.board.squares[data.promotionRow][data.promotionCol];

        if (boardPiece != data.pendingMovedPiece)
        {
            cout << "Pending promotion piece does not match the board!" << '\n';
            return false;
        }
    }

    file >> ws;

    if (!file.eof())
    {
        cout << "Unexpected data at the end of the save file!" << '\n';
        return false;
    }

    // Apply loaded data only after the complete file has been read.
    game.board = data.board;
    game.whiteTurn = data.whiteTurn;

    game.whiteCapturedPieces = data.whiteCapturedPieces;
    game.blackCapturedPieces = data.blackCapturedPieces;

    game.moveHistory = data.moveHistory;
    game.positionOccurrences = data.positionOccurrences;

    game.isPromotionPending = data.isPromotionPending;
    game.promotionIsWhite = data.promotionIsWhite;

    game.promotionRow = data.promotionRow;
    game.promotionCol = data.promotionCol;

    game.pendingMove = data.pendingMove;
    game.pendingMovedPiece = data.pendingMovedPiece;
    game.pendingCapturedPiece = data.pendingCapturedPiece;
    game.pendingIsEnPassant = data.pendingIsEnPassant;

    game.isPieceSelected = false;
    game.selectedRow = -1;
    game.selectedCol = -1;
    game.possibleMoves.clear();

    game.historyScrollOffset = 0;
    game.previousMoveCount = static_cast<int>(game.moveHistory.size());
    game.isDraggingScrollbar = false;
    game.scrollbarDragOffset = 0.0f;

    game.whiteMaterial = 0;
    game.blackMaterial = 0;

    for (char piece : game.whiteCapturedPieces)
        game.whiteMaterial += getSavedPieceValue(piece);

    for (char piece : game.blackCapturedPieces)
        game.blackMaterial += getSavedPieceValue(piece);

    game.gameOver = false;
    game.saveFileCleared = false;

    file.close();

    cout << "Game loaded successfully!" << '\n';

    return true;
}