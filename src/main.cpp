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
#include <fstream>
#include <iomanip>
#include <sstream>

#include "Board.h"
#include "PieceTextures.h"
#include "MoveValidator.h"
#include "MoveNotation.h"
#include "GameState.h"
#include "PositionKey.h"

using namespace sf;
using namespace std;

void drawCoordinates(
    RenderWindow& window,
    Font& font,
    float squareSize,
    float startX,
    float startY
) {
    for (int i = 0; i < 8; i++) {
        Color coordinateColor = (i % 2 == 0)
            ? Color(181, 136, 99)
            : Color(240, 217, 181);

        Text rank(font, to_string(8 - i), 14);

        rank.setFillColor(coordinateColor);
        rank.setPosition({
            startX + 4,
            startY + i * squareSize + 2
        });

        window.draw(rank);

        Text file(font, string(1, 'a' + i), 14);

        file.setFillColor(
            i % 2 == 0
            ? Color(240, 217, 181)
            : Color(181, 136, 99)
        );

        file.setPosition({
            startX + (i + 1) * squareSize - 18,
            startY + 8 * squareSize - 18
        });

        window.draw(file);
    }
}

void drawCapturedPieces(
    RenderWindow& window,
    vector<char>& capturedPieces,
    PieceTextures& textures,
    float startX,
    float startY
) {
    const float pieceSize = 30.0f;
    const float spacing = 36.0f;
    const int piecesPerRow = 9;

    for (int i = 0; i < capturedPieces.size(); i++) {
        Sprite pieceSprite(textures.whiteKing);

        pieceSprite.setTexture(getPieceTexture(capturedPieces[i], textures));

        int row = i / piecesPerRow;
        int col = i % piecesPerRow;

        pieceSprite.setPosition({
            startX + col * spacing,
            startY + row * 30.0f
        });

        float scale = pieceSize / 100.0f;
        pieceSprite.setScale({ scale, scale });

        window.draw(pieceSprite);
    }
}

void clearSavedGame() {
    ofstream file("saved_game.txt", ios::trunc);
    file.close();
}

bool expectLine(ifstream& file, const string& expected) {
    string line;

    while (getline(file, line)) {
        if (!line.empty())
            return line == expected;
    }

    return false;
}

template <typename T>
bool readLabeledValue(
    ifstream& file,
    const string& expectedLabel,
    T& value
) {
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
    ifstream& file,
    const string& expectedLabel,
    string& value
) {
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

struct SavedGameData {
    Board board;

    bool whiteTurn = true;

    vector<char> whiteCapturedPieces;
    vector<char> blackCapturedPieces;

    vector<pair<string, string>> moveHistory;

    unordered_map<string, int> positionOccurrences;

    bool isPromotionPending = false;
    bool promotionIsWhite = false;

    int promotionRow = -1;
    int promotionCol = -1;

    Move pendingMove{};

    char pendingMovedPiece = '.';
    char pendingCapturedPiece = '.';

    bool pendingIsEnPassant = false;
};

struct GameData {
    Board board;

    unordered_map<string, int> positionOccurrences;

    bool isPieceSelected = false;
    int selectedRow = -1;
    int selectedCol = -1;

    vector<Move> possibleMoves;

    vector<char> whiteCapturedPieces;
    vector<char> blackCapturedPieces;

    vector<pair<string, string>> moveHistory;

    int historyScrollOffset = 0;
    int previousMoveCount = 0;

    bool isDraggingScrollbar = false;
    float scrollbarDragOffset = 0.0f;

    int whiteMaterial = 0;
    int blackMaterial = 0;

    bool gameOver = false;
    bool saveFileCleared = false;
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

void saveGame(const GameData& game) {
    if (game.gameOver) {
        cout << "Cannot save: the game has already ended!" << '\n';
        return;
    }

    ofstream file("saved_game.txt", ios::trunc);

    if (!file.is_open()) {
        cout << "Failed to open save file!" << '\n';
        return;
    }

    file << "========== CHESS SAVE FILE ==========\n";
    file << "Version: 1\n\n";

    file << "========== BOARD ==========\n";

    for (int row = 0; row < 8; row++) {
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

    for (const auto& move : game.moveHistory) {
        file << quoted(move.first) << ' '
            << quoted(move.second) << '\n';
    }

    file << "\n========== POSITION REPETITIONS ==========\n";
    file << "Position Count: " << game.positionOccurrences.size() << '\n';

    for (const auto& position : game.positionOccurrences) {
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

    if (!file) {
        cout << "Failed to save game!" << '\n';
        return;
    }

    file.close();

    cout << "Game saved successfully!" << '\n';
}

bool loadGame(GameData& game) {
    ifstream file("saved_game.txt");

    if (!file.is_open()) {
        cout << "Failed to open save file!" << '\n';
        return false;
    }

    SavedGameData data;

    if (!expectLine(file, "========== CHESS SAVE FILE ==========")) {
        cout << "Invalid save file header!" << '\n';
        return false;
    }

    int version;

    if (!readLabeledValue(file, "Version", version) || version != 1) {
        cout << "Unsupported save file version!" << '\n';
        return false;
    }

    if (!expectLine(file, "========== BOARD ==========")) {
        cout << "Missing board data!" << '\n';
        return false;
    }

    const string validPieces = "prnbqkPRNBQK.";

    for (int row = 0; row < 8; row++) {
        string line;

        if (!getline(file, line) || line.size() != 8) {
            cout << "Invalid board data!" << '\n';
            return false;
        }

        for (int col = 0; col < 8; col++) {
            if (validPieces.find(line[col]) == string::npos) {
                cout << "Invalid chess piece!" << '\n';
                return false;
            }

            data.board.squares[row][col] = line[col];
        }
    }

    if (!expectLine(file, "========== CASTLING RIGHTS ==========")) {
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
        !readLabeledValue(file, "Black Queenside Rook Moved", blackQueensideRookMoved)
    ) {
        cout << "Invalid castling data!" << '\n';
        return false;
    }

    if (
        (whiteKingMoved != 0 && whiteKingMoved != 1) ||
        (blackKingMoved != 0 && blackKingMoved != 1) ||
        (whiteKingsideRookMoved != 0 && whiteKingsideRookMoved != 1) ||
        (whiteQueensideRookMoved != 0 && whiteQueensideRookMoved != 1) ||
        (blackKingsideRookMoved != 0 && blackKingsideRookMoved != 1) ||
        (blackQueensideRookMoved != 0 && blackQueensideRookMoved != 1)
    ) {
        cout << "Invalid castling values!" << '\n';
        return false;
    }

    data.board.whiteKingMoved = whiteKingMoved;
    data.board.blackKingMoved = blackKingMoved;

    data.board.whiteKingsideRookMoved = whiteKingsideRookMoved;
    data.board.whiteQueensideRookMoved = whiteQueensideRookMoved;

    data.board.blackKingsideRookMoved = blackKingsideRookMoved;
    data.board.blackQueensideRookMoved = blackQueensideRookMoved;

    if (!expectLine(file, "========== SPECIAL MOVE DATA ==========")) {
        cout << "Missing special move data!" << '\n';
        return false;
    }

    if (
        !readLabeledValue(file, "En Passant Row", data.board.enPassantRow) ||
        !readLabeledValue(file, "En Passant Column", data.board.enPassantCol) ||
        !readLabeledValue(file, "Half Move Clock", data.board.halfMoveClock)
    ) {
        cout << "Invalid special move data!" << '\n';
        return false;
    }

    if (
        data.board.enPassantRow < -1 || data.board.enPassantRow > 7 ||
        data.board.enPassantCol < -1 || data.board.enPassantCol > 7 ||
        (data.board.enPassantRow == -1) != (data.board.enPassantCol == -1) ||
        data.board.halfMoveClock < 0
    ) {
        cout << "Special move data is out of range!" << '\n';
        return false;
    }

    if (!expectLine(file, "========== GAME TURN ==========")) {
        cout << "Missing turn data!" << '\n';
        return false;
    }

    int savedWhiteTurn;

    if (
        !readLabeledValue(file, "White Turn", savedWhiteTurn) ||
        (savedWhiteTurn != 0 && savedWhiteTurn != 1)
    ) {
        cout << "Invalid turn data!" << '\n';
        return false;
    }

    data.whiteTurn = savedWhiteTurn;

    if (!expectLine(file, "========== CAPTURED PIECES ==========")) {
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
        !readLabeledString(file, "Black Captured", blackCaptured)
    ) {
        cout << "Invalid captured pieces data!" << '\n';
        return false;
    }

    if (
        whiteCapturedCount < 0 || whiteCapturedCount > 64 ||
        blackCapturedCount < 0 || blackCapturedCount > 64 ||
        whiteCaptured.size() != static_cast<size_t>(whiteCapturedCount) ||
        blackCaptured.size() != static_cast<size_t>(blackCapturedCount)
    ) {
        cout << "Invalid captured pieces count!" << '\n';
        return false;
    }

    const string validCapturedPieces = "prnbqPRNBQ";

    for (char piece : whiteCaptured) {
        if (validCapturedPieces.find(piece) == string::npos) {
            cout << "Invalid captured piece!" << '\n';
            return false;
        }
    }

    for (char piece : blackCaptured) {
        if (validCapturedPieces.find(piece) == string::npos) {
            cout << "Invalid captured piece!" << '\n';
            return false;
        }
    }

    data.whiteCapturedPieces.assign(whiteCaptured.begin(), whiteCaptured.end());
    data.blackCapturedPieces.assign(blackCaptured.begin(), blackCaptured.end());

    if (!expectLine(file, "========== MOVE HISTORY ==========")) {
        cout << "Missing move history!" << '\n';
        return false;
    }

    int moveCount;

    if (
        !readLabeledValue(file, "Move Count", moveCount) ||
        moveCount < 0 || moveCount > 10000
    ) {
        cout << "Invalid move count!" << '\n';
        return false;
    }

    for (int i = 0; i < moveCount; i++) {
        string whiteMove;
        string blackMove;

        if (!(file >> quoted(whiteMove) >> quoted(blackMove))) {
            cout << "Invalid move history data!" << '\n';
            return false;
        }

        data.moveHistory.push_back({ whiteMove, blackMove });
    }

    if (!expectLine(file, "========== POSITION REPETITIONS ==========")) {
        cout << "Missing position repetition data!" << '\n';
        return false;
    }

    int positionCount;

    if (
        !readLabeledValue(file, "Position Count", positionCount) ||
        positionCount < 1 || positionCount > 10000
    ) {
        cout << "Invalid position count!" << '\n';
        return false;
    }

    for (int i = 0; i < positionCount; i++) {
        string positionKey;
        int occurrenceCount;

        if (!(file >> quoted(positionKey) >> occurrenceCount)) {
            cout << "Invalid position repetition data!" << '\n';
            return false;
        }

        if (positionKey.empty() || occurrenceCount < 1) {
            cout << "Invalid position repetition values!" << '\n';
            return false;
        }

        if (data.positionOccurrences.find(positionKey) != data.positionOccurrences.end()) {
            cout << "Duplicate position key!" << '\n';
            return false;
        }

        data.positionOccurrences[positionKey] = occurrenceCount;
    }

    if (!expectLine(file, "========== PAWN PROMOTION ==========")) {
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
        !readLabeledValue(file, "Pending Move To Column", data.pendingMove.toCol)
        ) {
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
        capturedPiece.size() != 1
        ) {
        cout << "Invalid pending piece data!" << '\n';
        return false;
    }

    if (
        (promotionPending != 0 && promotionPending != 1) ||
        (promotionIsWhite != 0 && promotionIsWhite != 1) ||
        (pendingIsEnPassant != 0 && pendingIsEnPassant != 1)
    ) {
        cout << "Invalid promotion boolean values!" << '\n';
        return false;
    }

    data.isPromotionPending = promotionPending;
    data.promotionIsWhite = promotionIsWhite;
    data.pendingIsEnPassant = pendingIsEnPassant;

    data.pendingMovedPiece = movedPiece[0];
    data.pendingCapturedPiece = capturedPiece[0];

    if (data.isPromotionPending) {
        if (
            data.promotionRow < 0 || data.promotionRow > 7 ||
            data.promotionCol < 0 || data.promotionCol > 7 ||
            data.pendingMove.fromRow < 0 || data.pendingMove.fromRow > 7 ||
            data.pendingMove.fromCol < 0 || data.pendingMove.fromCol > 7 ||
            data.pendingMove.toRow < 0 || data.pendingMove.toRow > 7 ||
            data.pendingMove.toCol < 0 || data.pendingMove.toCol > 7 ||
            data.promotionRow != data.pendingMove.toRow ||
            data.promotionCol != data.pendingMove.toCol
        ) {
            cout << "Invalid pending promotion coordinates!" << '\n';
            return false;
        }

        if (
            (data.promotionIsWhite && data.pendingMovedPiece != 'P') ||
            (!data.promotionIsWhite && data.pendingMovedPiece != 'p')
        ) {
            cout << "Invalid pending promotion piece!" << '\n';
            return false;
        }

        char boardPiece = data.board.squares[data.promotionRow][data.promotionCol];

        if (boardPiece != data.pendingMovedPiece) {
            cout << "Pending promotion piece does not match the board!" << '\n';
            return false;
        }
    }

    file >> ws;

    if (!file.eof()) {
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
        game.whiteMaterial += getPieceValue(piece);

    for (char piece : game.blackCapturedPieces)
        game.blackMaterial += getPieceValue(piece);

    game.gameOver = false;
    game.saveFileCleared = false;

    file.close();

    cout << "Game loaded successfully!" << '\n';

    return true;
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

    RectangleShape square(Vector2f(squareSize, squareSize));

    RectangleShape highlight(Vector2f(squareSize, squareSize));
    highlight.setFillColor(Color(255, 255, 0, 100));

    CircleShape moveDot(8.0f);
    moveDot.setFillColor(Color(100, 100, 100, 150));

    CircleShape captureRing(squareSize / 2 - 5);
    captureRing.setFillColor(Color::Transparent);
    captureRing.setOutlineThickness(5);
    captureRing.setOutlineColor(Color(100, 100, 100, 150));

    PieceTextures textures;
    loadPieceTextures(textures);

    Font font;

    if (!font.openFromFile("assets/fonts/ARIAL.TTF"))
        return -1;

    Text blackPlayerText(font);
    blackPlayerText.setString("Player 2 - Black");
    blackPlayerText.setCharacterSize(24);
    blackPlayerText.setStyle(Text::Bold);
    blackPlayerText.setFillColor(Color::White);
    blackPlayerText.setPosition({ 20.0f, 20.0f });

    Text whitePlayerText(font);
    whitePlayerText.setString("Player 1 - White");
    whitePlayerText.setCharacterSize(24);
    whitePlayerText.setStyle(Text::Bold);
    whitePlayerText.setFillColor(Color::White);
    whitePlayerText.setPosition({ 20.0f, 720.0f });

    Text blackAdvantageText(font);
    blackAdvantageText.setCharacterSize(22);
    blackAdvantageText.setStyle(Text::Bold);
    blackAdvantageText.setFillColor(Color::White);
    blackAdvantageText.setPosition({ 570.0f, 35.0f });

    Text whiteAdvantageText(font);
    whiteAdvantageText.setCharacterSize(22);
    whiteAdvantageText.setStyle(Text::Bold);
    whiteAdvantageText.setFillColor(Color::White);
    whiteAdvantageText.setPosition({ 570.0f, 725.0f });

    RectangleShape blackPanel({ 600.0f, 80.0f });
    blackPanel.setPosition({ 10.0f, 10.0f });
    blackPanel.setFillColor(Color(181, 139, 105));
    blackPanel.setOutlineColor(Color(100, 100, 100));
    blackPanel.setOutlineThickness(1.0f);

    RectangleShape whitePanel({ 600.0f, 80.0f });
    whitePanel.setPosition({ 10.0f, 710.0f });
    whitePanel.setFillColor(Color(181, 139, 105));
    whitePanel.setOutlineColor(Color(100, 100, 100));
    whitePanel.setOutlineThickness(1.0f);

    RectangleShape moveHistoryPanel({ 380.0f, 600.0f });
    moveHistoryPanel.setPosition({ 620.0f, 10.0f });
    moveHistoryPanel.setFillColor(Color(92, 64, 51));
    moveHistoryPanel.setOutlineColor(Color(100, 100, 100));
    moveHistoryPanel.setOutlineThickness(1.0f);

    Text moveHistoryTitle(font, "Move History", 28);
    moveHistoryTitle.setStyle(Text::Bold);
    moveHistoryTitle.setFillColor(Color::White);
    moveHistoryTitle.setPosition({ 640.0f, 15.0f });

    RectangleShape newGameButton({ 130.0f, 40.0f });
    newGameButton.setPosition({ 620.0f, 620.0f });
    newGameButton.setFillColor(Color(181, 139, 105));
    newGameButton.setOutlineColor(Color(220, 220, 220));
    newGameButton.setOutlineThickness(1.0f);

    Text newGameButtonText(font, "New Game", 18);
    newGameButtonText.setStyle(Text::Bold);
    newGameButtonText.setFillColor(Color::White);
    newGameButtonText.setPosition({ 635.0f, 630.0f });

    RectangleShape saveGameButton({ 110.0f, 40.0f });
    saveGameButton.setPosition({ 760.0f, 620.0f });
    saveGameButton.setFillColor(Color(70, 120, 170));
    saveGameButton.setOutlineColor(Color(220, 220, 220));
    saveGameButton.setOutlineThickness(1.0f);

    Text saveGameButtonText(font, "Save Game", 17);
    saveGameButtonText.setStyle(Text::Bold);
    saveGameButtonText.setFillColor(Color::White);
    saveGameButtonText.setPosition({ 770.0f, 630.0f });

    RectangleShape loadGameButton({ 110.0f, 40.0f });
    loadGameButton.setPosition({ 880.0f, 620.0f });
    loadGameButton.setFillColor(Color(70, 120, 170));
    loadGameButton.setOutlineColor(Color(220, 220, 220));
    loadGameButton.setOutlineThickness(1.0f);

    Text loadGameButtonText(font, "Load Game", 17);
    loadGameButtonText.setStyle(Text::Bold);
    loadGameButtonText.setFillColor(Color::White);
    loadGameButtonText.setPosition({ 890.0f, 630.0f });

    RectangleShape confirmationOverlay({ 1010.0f, 800.0f });
    confirmationOverlay.setFillColor(Color(0, 0, 0, 160));

    RectangleShape confirmationPanel({ 450.0f, 200.0f });
    confirmationPanel.setPosition({ 280.0f, 300.0f });
    confirmationPanel.setFillColor(Color(70, 55, 45));
    confirmationPanel.setOutlineColor(Color(220, 217, 181));
    confirmationPanel.setOutlineThickness(2.0f);

    Text confirmationTitle(font, "Load Saved Game?", 26);
    confirmationTitle.setStyle(Text::Bold);
    confirmationTitle.setFillColor(Color::White);
    confirmationTitle.setPosition({ 315.0f, 325.0f });

    Text confirmationMessage(
        font,
        "Your current game will be replaced.",
        18
    );
    confirmationMessage.setFillColor(Color(220, 220, 220));
    confirmationMessage.setPosition({ 315.0f, 375.0f });

    RectangleShape confirmLoadButton({ 100.0f, 40.0f });
    confirmLoadButton.setPosition({ 390.0f, 425.0f });
    confirmLoadButton.setFillColor(Color(70, 130, 80));

    Text confirmLoadText(font, "Confirm", 18);
    confirmLoadText.setStyle(Text::Bold);
    confirmLoadText.setFillColor(Color::White);
    confirmLoadText.setPosition({ 405.0f, 433.0f });

    RectangleShape cancelLoadButton({ 100.0f, 40.0f });
    cancelLoadButton.setPosition({ 520.0f, 425.0f });
    cancelLoadButton.setFillColor(Color(150, 65, 65));

    Text cancelLoadText(font, "Cancel", 18);
    cancelLoadText.setStyle(Text::Bold);
    cancelLoadText.setFillColor(Color::White);
    cancelLoadText.setPosition({ 540.0f, 433.0f });

    Text moveNumberHeader(font, "No.", 18);
    moveNumberHeader.setStyle(Text::Bold);
    moveNumberHeader.setFillColor(Color(220, 220, 220));
    moveNumberHeader.setPosition({ 640.0f, 65.0f });

    Text whiteMoveHeader(font, "White", 18);
    whiteMoveHeader.setStyle(Text::Bold);
    whiteMoveHeader.setFillColor(Color::White);
    whiteMoveHeader.setPosition({ 700.0f, 65.0f });

    Text blackMoveHeader(font, "Black", 18);
    blackMoveHeader.setStyle(Text::Bold);
    blackMoveHeader.setFillColor(Color::White);
    blackMoveHeader.setPosition({ 830.0f, 65.0f });

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

                            if (!loadGame(game))
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
                            saveGame(game);
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
            clearSavedGame();
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

        window.draw(blackPanel);
        window.draw(whitePanel);
        window.draw(moveHistoryPanel);

        window.draw(newGameButton);
        window.draw(newGameButtonText);

        window.draw(saveGameButton);
        window.draw(saveGameButtonText);

        window.draw(loadGameButton);
        window.draw(loadGameButtonText);

        window.draw(moveHistoryTitle);
        window.draw(moveNumberHeader);
        window.draw(whiteMoveHeader);
        window.draw(blackMoveHeader);

        int endIndex = min(
            static_cast<int>(moveHistory.size()),
            historyScrollOffset + visibleRows
        );

        for (int i = historyScrollOffset; i < endIndex; i++) {
            int row = i - historyScrollOffset;

            float y = historyContentTop + row * 24.0f;

            RectangleShape rowBackground({ 340.0f, 24.0f });
            rowBackground.setPosition({ 630.0f, y });

            if (i == static_cast<int>(moveHistory.size()) - 1) {
                rowBackground.setFillColor(Color(140, 105, 45));
            }
            else if (row % 2 == 0) {
                rowBackground.setFillColor(Color(110, 78, 62));
            }
            else {
                rowBackground.setFillColor(Color(92, 64, 51));
            }

            window.draw(rowBackground);

            Text moveNumber(font, to_string(i + 1) + ".", 17);
            moveNumber.setFillColor(Color(220, 220, 220));
            moveNumber.setPosition({ 640.0f, y });

            Text whiteMove(font, moveHistory[i].first, 17);
            whiteMove.setFillColor(Color::White);
            whiteMove.setPosition({ 700.0f, y });

            Text blackMove(font, moveHistory[i].second, 17);
            blackMove.setFillColor(Color::White);
            blackMove.setPosition({ 830.0f, y });

            window.draw(moveNumber);
            window.draw(whiteMove);
            window.draw(blackMove);
        }

        int totalRows = static_cast<int>(moveHistory.size());

        if (totalRows > visibleRows) {
            int maxScroll = totalRows - visibleRows;

            RectangleShape scrollbarTrack({
                scrollbarWidth,
                historyContentHeight
            });

            scrollbarTrack.setPosition({
                scrollbarX,
                historyContentTop
            });

            scrollbarTrack.setFillColor(Color(55, 40, 32));

            window.draw(scrollbarTrack);

            float thumbHeight = max(
                30.0f,
                historyContentHeight * visibleRows / totalRows
            );

            float thumbTop =
                historyContentTop +
                (static_cast<float>(historyScrollOffset) / maxScroll) *
                (historyContentHeight - thumbHeight);

            RectangleShape scrollbarThumb({
                scrollbarWidth,
                thumbHeight
            });

            scrollbarThumb.setPosition({
                scrollbarX,
                thumbTop
            });

            scrollbarThumb.setFillColor(Color(200, 170, 130));

            window.draw(scrollbarThumb);
        }

        for (int row = 0; row < boardSize; row++) {
            for (int col = 0; col < boardSize; col++) {
                float x = startX + col * squareSize;
                float y = startY + row * squareSize;

                square.setPosition({ x, y });

                if ((row + col) % 2 == 0)
                    square.setFillColor(Color(240, 217, 181));
                else
                    square.setFillColor(Color(181, 136, 99));

                window.draw(square);
            }
        }

        if (isPieceSelected) {
            float x = startX + selectedCol * squareSize;
            float y = startY + selectedRow * squareSize;

            highlight.setPosition({ x, y });
            window.draw(highlight);
        }

        for (const Move& move : possibleMoves) {
            float x = startX + move.toCol * squareSize;
            float y = startY + move.toRow * squareSize;

            if (board.squares[move.toRow][move.toCol] == '.') {
                moveDot.setPosition({
                    x + squareSize / 2 - 8,
                    y + squareSize / 2 - 8
                });

                window.draw(moveDot);
            }
            else {
                captureRing.setPosition({ x + 5, y + 5 });
                window.draw(captureRing);
            }
        }

        drawPieces(window, board, textures, squareSize, startX, startY);

        int materialDifference = whiteMaterial - blackMaterial;

        if (materialDifference > 0) {
            whiteAdvantageText.setString("+" + to_string(materialDifference));
            blackAdvantageText.setString("");
        }
        else if (materialDifference < 0) {
            blackAdvantageText.setString("+" + to_string(-materialDifference));
            whiteAdvantageText.setString("");
        }
        else {
            whiteAdvantageText.setString("");
            blackAdvantageText.setString("");
        }

        window.draw(blackPlayerText);
        window.draw(whitePlayerText);

        window.draw(blackAdvantageText);
        window.draw(whiteAdvantageText);

        drawCapturedPieces(
            window,
            whiteCapturedPieces,
            textures,
            220.0f,
            725.0f
        );

        drawCapturedPieces(
            window,
            blackCapturedPieces,
            textures,
            220.0f,
            25.0f
        );

        drawCoordinates(window, font, squareSize, startX, startY);

        if (showLoadConfirmation) {
            window.draw(confirmationOverlay);
            window.draw(confirmationPanel);

            window.draw(confirmationTitle);
            window.draw(confirmationMessage);

            window.draw(confirmLoadButton);
            window.draw(confirmLoadText);

            window.draw(cancelLoadButton);
            window.draw(cancelLoadText);
        }

        window.display();
    }

    return 0;
}
