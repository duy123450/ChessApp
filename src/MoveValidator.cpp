#include "MoveValidator.h"
#include <cstdlib>

// Main movement dispatcher
bool isValidMove(const Board& board, const Move& move) {
    int fromRow = move.fromRow;
    int fromCol = move.fromCol;
    int toRow = move.toRow;
    int toCol = move.toCol;

    // Check board boundaries
    if (
        fromRow < 0 || fromRow >= 8 ||
        fromCol < 0 || fromCol >= 8 ||
        toRow < 0 || toRow >= 8 ||
        toCol < 0 || toCol >= 8
        )
        return false;

    char piece = board.squares[fromRow][fromCol];
    char target = board.squares[toRow][toCol];

    // Cannot move an empty square
    if (piece == '.')
        return false;

    // Cannot move to the same square
    if (fromRow == toRow && fromCol == toCol)
        return false;

    bool isWhite = piece >= 'A' && piece <= 'Z';
    bool targetIsWhite = target >= 'A' && target <= 'Z';

    // Cannot capture your own piece
    if (target != '.' && isWhite == targetIsWhite)
        return false;

    // Cannot capture the opponent's King
    if (target == 'K' || target == 'k')
        return false;

    // Special case: Castling
    if ((piece == 'K' || piece == 'k') &&
        move.fromCol == 4 &&
        move.toRow == move.fromRow &&
        abs(move.toCol - move.fromCol) == 2) {

        return isCastlingMoveValid(board, move, isWhite);
    }

    // Check the piece's movement pattern
    bool movePatternValid = false;

    switch (piece) {
        case 'P':
        case 'p':
            movePatternValid = isPawnMoveValid(board, move, isWhite);
            break;

        case 'R':
        case 'r':
            movePatternValid = isRookMoveValid(board, move);
            break;

        case 'B':
        case 'b':
            movePatternValid = isBishopMoveValid(board, move);
            break;

        case 'Q':
        case 'q':
            movePatternValid = isQueenMoveValid(board, move);
            break;

        case 'N':
        case 'n':
            movePatternValid = isKnightMoveValid(board, move);
            break;

        case 'K':
        case 'k':
            movePatternValid = isKingMoveValid(board, move);
            break;

        default:
            return false;
    }

    if (!movePatternValid)
        return false;

    // Simulate the move
    Board simulatedBoard = board;

    // Check whether this is an en passant capture
    bool isEnPassant =
        (piece == 'P' || piece == 'p') &&
        fromCol != toCol &&
        target == '.' &&
        fromRow == board.enPassantRow &&
        toCol == board.enPassantCol;

    // Remove the captured Pawn during en passant
    if (isEnPassant)
        simulatedBoard.squares[fromRow][toCol] = '.';

    // Move the piece on the simulated board
    simulatedBoard.squares[toRow][toCol] = piece;
    simulatedBoard.squares[fromRow][fromCol] = '.';

    // Reject the move if it leaves our King in check
    if (isKingInCheck(simulatedBoard, isWhite))
        return false;

    return true;
}

// Check for legal move
bool hasAnyLegalMove(const Board& board, bool isWhite) {
    for (int fromRow = 0; fromRow < 8; fromRow++) {
        for (int fromCol = 0; fromCol < 8; fromCol++) {
            char piece = board.squares[fromRow][fromCol];

            if (piece == '.')
                continue;

            bool pieceIsWhite = piece >= 'A' && piece <= 'Z';

            if (pieceIsWhite != isWhite)
                continue;

            // Try every possible destination
            for (int toRow = 0; toRow < 8; toRow++) {
                for (int toCol = 0; toCol < 8; toCol++) {
                    Move move{
                        fromRow,
                        fromCol,
                        toRow,
                        toCol
                    };

                    if (isValidMove(board, move))
                        return true;
                }
            }
        }
    }

    return false;
}

// Check whether the path between two squares is clear
bool isPathClear(const Board& board, const Move& move) {
    int fromRow = move.fromRow;
    int fromCol = move.fromCol;
    int toRow = move.toRow;
    int toCol = move.toCol;

    // Determine movement direction
    int rowDirection = 0;
    int colDirection = 0;

    if (toRow > fromRow)
        rowDirection = 1;
    else if (toRow < fromRow)
        rowDirection = -1;

    if (toCol > fromCol)
        colDirection = 1;
    else if (toCol < fromCol)
        colDirection = -1;

    // Start checking from the square after the starting square
    int row = fromRow + rowDirection;
    int col = fromCol + colDirection;

    // Check every square before the destination
    while (row != toRow || col != toCol) {
        if (board.squares[row][col] != '.')
            return false;

        row += rowDirection;
        col += colDirection;
    }

    return true;
}

// Pawn movement
bool isPawnMoveValid(const Board& board, const Move& move, bool isWhite) {
    int direction = isWhite ? -1 : 1;
    int startingRow = isWhite ? 6 : 1;

    int fromRow = move.fromRow;
    int fromCol = move.fromCol;
    int toRow = move.toRow;
    int toCol = move.toCol;

    char target = board.squares[toRow][toCol];

    int rowDistance = toRow - fromRow;
    int colDistance = toCol - fromCol;

    // Move forward one square
    if (
        target == '.' &&
        rowDistance == direction &&
        colDistance == 0
        )
        return true;

    // Move forward two squares from starting position
    if (
        target == '.' &&
        fromRow == startingRow &&
        rowDistance == 2 * direction &&
        colDistance == 0 &&
        board.squares[fromRow + direction][fromCol] == '.'
        )
        return true;

    // En passant capture
    if (abs(move.toCol - move.fromCol) == 1 &&
        move.toRow == move.fromRow + direction &&
        board.squares[move.toRow][move.toCol] == '.' &&
        move.fromRow == board.enPassantRow &&
        move.toCol == board.enPassantCol) {

        char capturedPawn = isWhite ? 'p' : 'P';

        if (board.squares[board.enPassantRow][board.enPassantCol] == capturedPawn)
            return true;
    }

    // Capture diagonally
    if (
        target != '.' &&
        rowDistance == direction &&
        (colDistance == 1 || colDistance == -1)
        )
        return true;

    return false;
}

// Rook movement
bool isRookMoveValid(const Board& board, const Move& move) {
    // Rook can only move horizontally or vertically
    if (move.fromRow != move.toRow &&
        move.fromCol != move.toCol)
        return false;

    return isPathClear(board, move);
}

// Bishop movement
bool isBishopMoveValid(const Board& board, const Move& move) {
    int rowDistance = abs(move.toRow - move.fromRow);
    int colDistance = abs(move.toCol - move.fromCol);

    // Bishop can only move diagonally
    if (rowDistance != colDistance)
        return false;

    return isPathClear(board, move);
}

// Queen movement
bool isQueenMoveValid(const Board& board, const Move& move) {
    return isRookMoveValid(board, move) || isBishopMoveValid(board, move);
}

// Knight movement
bool isKnightMoveValid(const Board& board, const Move& move) {
    int rowDistance = abs(move.toRow - move.fromRow);
    int colDistance = abs(move.toCol - move.fromCol);

    // Knight moves in an L-shape
    return (rowDistance == 2 && colDistance == 1) || (rowDistance == 1 && colDistance == 2);
}

// King movement
//// Find where the King is
bool findKing(const Board& board, bool isWhite, int& kingRow, int& kingCol) {
    char king = isWhite ? 'K' : 'k';

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (board.squares[row][col] == king) {
                kingRow = row;
                kingCol = col;

                return true;
            }
        }
    }

    return false;
}

//// Validate if the King is in Check
bool isKingInCheck(const Board& board, bool isWhite) {
    int kingRow;
    int kingCol;

    // Find the King
    if (!findKing(board, isWhite, kingRow, kingCol))
        return false;

    // Check every square for an opponent's piece
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            char piece = board.squares[row][col];

            if (piece == '.')
                continue;

            bool attackerIsWhite = piece >= 'A' && piece <= 'Z';

            // Ignore pieces belonging to the King we're checking
            if (attackerIsWhite == isWhite)
                continue;

            Move move{
                row,
                col,
                kingRow,
                kingCol
            };

            switch (piece) {
                // Pawn attacks diagonally
                case 'P':
                case 'p':
                    if (
                        abs(kingCol - col) == 1 &&
                        ((attackerIsWhite && kingRow == row - 1) ||
                            (!attackerIsWhite && kingRow == row + 1))
                        )
                        return true;

                    break;

                case 'R':
                case 'r':
                    if (isRookMoveValid(board, move))
                        return true;
                    break;

                case 'B':
                case 'b':
                    if (isBishopMoveValid(board, move))
                        return true;
                    break;

                case 'Q':
                case 'q':
                    if (isQueenMoveValid(board, move))
                        return true;
                    break;

                case 'N':
                case 'n':
                    if (isKnightMoveValid(board, move))
                        return true;
                    break;

                case 'K':
                case 'k':
                    if (isKingMoveValid(board, move))
                        return true;
                    break;
            }
        }
    }

    return false;
}

bool isCastlingMoveValid(const Board& board, const Move& move, bool isWhite) {
    int row = isWhite ? 7 : 0;

    char king = isWhite ? 'K' : 'k';
    char rook = isWhite ? 'R' : 'r';

    // King must start from its original square
    if (move.fromRow != row || move.fromCol != 4)
        return false;

    // King must move to the same row
    if (move.toRow != row)
        return false;

    bool isKingside = move.toCol == 6;
    bool isQueenside = move.toCol == 2;

    if (!isKingside && !isQueenside)
        return false;

    // King must not have moved before
    if (isWhite ? board.whiteKingMoved : board.blackKingMoved)
        return false;

    // Check the correct Rook
    int rookCol = isKingside ? 7 : 0;

    if (board.squares[row][rookCol] != rook)
        return false;

    // Rook must not have moved before
    if (isWhite) {
        if (isKingside && board.whiteKingsideRookMoved)
            return false;

        if (isQueenside && board.whiteQueensideRookMoved)
            return false;
    }
    else {
        if (isKingside && board.blackKingsideRookMoved)
            return false;

        if (isQueenside && board.blackQueensideRookMoved)
            return false;
    }

    // Check whether the squares between King and Rook are empty
    int startCol = isKingside ? 5 : 1;
    int endCol = isKingside ? 6 : 3;

    for (int col = startCol; col <= endCol; col++) {
        if (board.squares[row][col] != '.')
            return false;
    }

    // King cannot castle while in check
    if (isKingInCheck(board, isWhite))
        return false;

    // Check the square the King passes through
    int middleCol = isKingside ? 5 : 3;

    Board middleBoard = board;

    middleBoard.squares[row][4] = '.';
    middleBoard.squares[row][middleCol] = king;

    if (isKingInCheck(middleBoard, isWhite))
        return false;

    // Check the final position after castling
    Board finalBoard = board;

    int rookDestination = isKingside ? 5 : 3;

    finalBoard.squares[row][4] = '.';
    finalBoard.squares[row][move.toCol] = king;

    finalBoard.squares[row][rookCol] = '.';
    finalBoard.squares[row][rookDestination] = rook;

    if (isKingInCheck(finalBoard, isWhite))
        return false;

    return true;
}

bool isKingMoveValid(const Board& board, const Move& move) {
    int rowDistance = abs(move.toRow - move.fromRow);
    int colDistance = abs(move.toCol - move.fromCol);

    // King can move one square in any direction
    return (rowDistance <= 1 && colDistance <= 1) && (rowDistance != 0 || colDistance != 0);
}