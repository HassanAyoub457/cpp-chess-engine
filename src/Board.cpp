#include "chess/Board.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace chess {
namespace {

bool isWhitePiece(char piece) {
    return piece >= 'A' && piece <= 'Z';
}

bool isBlackPiece(char piece) {
    return piece >= 'a' && piece <= 'z';
}

bool isPieceForSide(char piece, bool white) {
    return white ? isWhitePiece(piece) : isBlackPiece(piece);
}

bool isOpponent(char piece, bool white) {
    return piece != '.' && isPieceForSide(piece, !white);
}

bool onBoard(int row, int col) {
    return row >= 0 && row < 8 && col >= 0 && col < 8;
}

char lower(char value) {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(value)));
}

void addPromotions(std::vector<Move>& moves, int from, int to,
                   bool enPassant = false) {
    for (const char piece : {'q', 'r', 'b', 'n'}) {
        moves.push_back(Move{from, to, piece, enPassant, false});
    }
}

}  // namespace

std::string Move::uci() const {
    if (from < 0 || to < 0) {
        return "0000";
    }
    std::string result = Board::squareToString(from) + Board::squareToString(to);
    if (promotion != '\0') {
        result.push_back(lower(promotion));
    }
    return result;
}

Board Board::startingPosition() {
    return fromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

Board Board::fromFEN(const std::string& fen) {
    std::istringstream input(fen);
    std::string placement;
    std::string activeColor;
    std::string castling;
    std::string enPassant;
    Board board;
    board.squares_.fill('.');

    if (!(input >> placement >> activeColor >> castling >> enPassant)) {
        throw std::invalid_argument("FEN must contain at least four fields");
    }
    input >> board.halfmoveClock_ >> board.fullmoveNumber_;
    if (board.fullmoveNumber_ <= 0) {
        board.fullmoveNumber_ = 1;
    }

    int square = 0;
    for (const char token : placement) {
        if (token == '/') {
            if (square % 8 != 0) {
                throw std::invalid_argument("Invalid FEN rank width");
            }
            continue;
        }
        if (std::isdigit(static_cast<unsigned char>(token))) {
            const int empty = token - '0';
            if (empty < 1 || empty > 8 || square + empty > 64) {
                throw std::invalid_argument("Invalid empty-square count in FEN");
            }
            square += empty;
            continue;
        }
        if (std::string("prnbqkPRNBQK").find(token) == std::string::npos ||
            square >= 64) {
            throw std::invalid_argument("Invalid piece placement in FEN");
        }
        board.squares_[square++] = token;
    }
    if (square != 64) {
        throw std::invalid_argument("FEN does not describe 64 squares");
    }

    if (activeColor != "w" && activeColor != "b") {
        throw std::invalid_argument("Invalid active color in FEN");
    }
    board.whiteToMove_ = activeColor == "w";
    board.whiteKingSide_ = castling.find('K') != std::string::npos;
    board.whiteQueenSide_ = castling.find('Q') != std::string::npos;
    board.blackKingSide_ = castling.find('k') != std::string::npos;
    board.blackQueenSide_ = castling.find('q') != std::string::npos;
    board.enPassantSquare_ = enPassant == "-" ? -1 : squareFromString(enPassant);
    return board;
}

std::string Board::toFEN() const {
    std::ostringstream output;
    for (int row = 0; row < 8; ++row) {
        int empty = 0;
        for (int col = 0; col < 8; ++col) {
            const char piece = squares_[row * 8 + col];
            if (piece == '.') {
                ++empty;
            } else {
                if (empty > 0) {
                    output << empty;
                    empty = 0;
                }
                output << piece;
            }
        }
        if (empty > 0) {
            output << empty;
        }
        if (row != 7) {
            output << '/';
        }
    }

    output << (whiteToMove_ ? " w " : " b ");
    std::string castling;
    if (whiteKingSide_) castling.push_back('K');
    if (whiteQueenSide_) castling.push_back('Q');
    if (blackKingSide_) castling.push_back('k');
    if (blackQueenSide_) castling.push_back('q');
    output << (castling.empty() ? "-" : castling) << ' ';
    output << (enPassantSquare_ < 0 ? "-" : squareToString(enPassantSquare_));
    output << ' ' << halfmoveClock_ << ' ' << fullmoveNumber_;
    return output.str();
}

std::string Board::pretty() const {
    std::ostringstream output;
    output << "\n    a   b   c   d   e   f   g   h\n";
    output << "  +---+---+---+---+---+---+---+---+\n";
    for (int row = 0; row < 8; ++row) {
        output << 8 - row << " |";
        for (int col = 0; col < 8; ++col) {
            output << ' ' << squares_[row * 8 + col] << " |";
        }
        output << ' ' << 8 - row << '\n';
        output << "  +---+---+---+---+---+---+---+---+\n";
    }
    output << "    a   b   c   d   e   f   g   h\n";
    output << "Side to move: " << (whiteToMove_ ? "White" : "Black");
    if (isInCheck(whiteToMove_)) {
        output << " (in check)";
    }
    output << '\n';
    return output.str();
}

int Board::squareFromString(const std::string& square) {
    if (square.size() != 2) {
        throw std::invalid_argument("Square must use algebraic notation, for example e4");
    }
    const char file = static_cast<char>(std::tolower(
        static_cast<unsigned char>(square[0])));
    const char rank = square[1];
    if (file < 'a' || file > 'h' || rank < '1' || rank > '8') {
        throw std::invalid_argument("Square is outside the chess board");
    }
    const int col = file - 'a';
    const int row = 8 - (rank - '0');
    return row * 8 + col;
}

std::string Board::squareToString(int square) {
    if (square < 0 || square >= 64) {
        throw std::out_of_range("Square index must be between 0 and 63");
    }
    const char file = static_cast<char>('a' + square % 8);
    const char rank = static_cast<char>('8' - square / 8);
    return std::string{file, rank};
}

std::vector<Move> Board::pseudoLegalMoves() const {
    std::vector<Move> moves;
    moves.reserve(64);
    const bool white = whiteToMove_;

    for (int from = 0; from < 64; ++from) {
        const char piece = squares_[from];
        if (!isPieceForSide(piece, white)) {
            continue;
        }

        const int row = from / 8;
        const int col = from % 8;
        const char type = lower(piece);

        if (type == 'p') {
            const int direction = white ? -1 : 1;
            const int startRow = white ? 6 : 1;
            const int promotionRow = white ? 0 : 7;
            const int oneRow = row + direction;

            if (onBoard(oneRow, col)) {
                const int one = oneRow * 8 + col;
                if (squares_[one] == '.') {
                    if (oneRow == promotionRow) {
                        addPromotions(moves, from, one);
                    } else {
                        moves.push_back(Move{from, one});
                    }
                    const int twoRow = row + 2 * direction;
                    const int two = twoRow * 8 + col;
                    if (row == startRow && squares_[two] == '.') {
                        moves.push_back(Move{from, two});
                    }
                }
            }

            for (const int deltaCol : {-1, 1}) {
                const int targetRow = row + direction;
                const int targetCol = col + deltaCol;
                if (!onBoard(targetRow, targetCol)) {
                    continue;
                }
                const int to = targetRow * 8 + targetCol;
                const bool capture = isOpponent(squares_[to], white) &&
                                     lower(squares_[to]) != 'k';
                const bool enPassant = to == enPassantSquare_;
                if (!capture && !enPassant) {
                    continue;
                }
                if (targetRow == promotionRow) {
                    addPromotions(moves, from, to, enPassant);
                } else {
                    moves.push_back(Move{from, to, '\0', enPassant, false});
                }
            }
            continue;
        }

        if (type == 'n') {
            constexpr int jumps[8][2] = {
                {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
                {1, -2}, {1, 2}, {2, -1}, {2, 1},
            };
            for (const auto& jump : jumps) {
                const int targetRow = row + jump[0];
                const int targetCol = col + jump[1];
                if (!onBoard(targetRow, targetCol)) continue;
                const int to = targetRow * 8 + targetCol;
                if (squares_[to] == '.' ||
                    (isOpponent(squares_[to], white) && lower(squares_[to]) != 'k')) {
                    moves.push_back(Move{from, to});
                }
            }
            continue;
        }

        if (type == 'b' || type == 'r' || type == 'q') {
            std::vector<std::pair<int, int>> directions;
            if (type == 'b' || type == 'q') {
                directions.insert(directions.end(), {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}});
            }
            if (type == 'r' || type == 'q') {
                directions.insert(directions.end(), {{-1, 0}, {1, 0}, {0, -1}, {0, 1}});
            }
            for (const auto& [deltaRow, deltaCol] : directions) {
                int targetRow = row + deltaRow;
                int targetCol = col + deltaCol;
                while (onBoard(targetRow, targetCol)) {
                    const int to = targetRow * 8 + targetCol;
                    if (squares_[to] == '.') {
                        moves.push_back(Move{from, to});
                    } else {
                        if (isOpponent(squares_[to], white) && lower(squares_[to]) != 'k') {
                            moves.push_back(Move{from, to});
                        }
                        break;
                    }
                    targetRow += deltaRow;
                    targetCol += deltaCol;
                }
            }
            continue;
        }

        if (type == 'k') {
            for (int deltaRow = -1; deltaRow <= 1; ++deltaRow) {
                for (int deltaCol = -1; deltaCol <= 1; ++deltaCol) {
                    if (deltaRow == 0 && deltaCol == 0) continue;
                    const int targetRow = row + deltaRow;
                    const int targetCol = col + deltaCol;
                    if (!onBoard(targetRow, targetCol)) continue;
                    const int to = targetRow * 8 + targetCol;
                    if (squares_[to] == '.' ||
                        (isOpponent(squares_[to], white) && lower(squares_[to]) != 'k')) {
                        moves.push_back(Move{from, to});
                    }
                }
            }

            if (white && from == 60 && piece == 'K' && !isInCheck(true)) {
                if (whiteKingSide_ && squares_[63] == 'R' &&
                    squares_[61] == '.' && squares_[62] == '.' &&
                    !isSquareAttacked(61, false) && !isSquareAttacked(62, false)) {
                    moves.push_back(Move{60, 62, '\0', false, true});
                }
                if (whiteQueenSide_ && squares_[56] == 'R' &&
                    squares_[57] == '.' && squares_[58] == '.' && squares_[59] == '.' &&
                    !isSquareAttacked(59, false) && !isSquareAttacked(58, false)) {
                    moves.push_back(Move{60, 58, '\0', false, true});
                }
            } else if (!white && from == 4 && piece == 'k' && !isInCheck(false)) {
                if (blackKingSide_ && squares_[7] == 'r' &&
                    squares_[5] == '.' && squares_[6] == '.' &&
                    !isSquareAttacked(5, true) && !isSquareAttacked(6, true)) {
                    moves.push_back(Move{4, 6, '\0', false, true});
                }
                if (blackQueenSide_ && squares_[0] == 'r' &&
                    squares_[1] == '.' && squares_[2] == '.' && squares_[3] == '.' &&
                    !isSquareAttacked(3, true) && !isSquareAttacked(2, true)) {
                    moves.push_back(Move{4, 2, '\0', false, true});
                }
            }
        }
    }
    return moves;
}

bool Board::isSquareAttacked(int square, bool byWhite) const {
    const int targetRow = square / 8;
    const int targetCol = square % 8;

    for (int from = 0; from < 64; ++from) {
        const char piece = squares_[from];
        if (!isPieceForSide(piece, byWhite)) continue;
        const int row = from / 8;
        const int col = from % 8;
        const char type = lower(piece);

        if (type == 'p') {
            const int direction = byWhite ? -1 : 1;
            if (targetRow == row + direction && std::abs(targetCol - col) == 1) {
                return true;
            }
            continue;
        }

        if (type == 'n') {
            const int deltaRow = std::abs(targetRow - row);
            const int deltaCol = std::abs(targetCol - col);
            if ((deltaRow == 2 && deltaCol == 1) ||
                (deltaRow == 1 && deltaCol == 2)) {
                return true;
            }
            continue;
        }

        if (type == 'k') {
            if (std::max(std::abs(targetRow - row), std::abs(targetCol - col)) == 1) {
                return true;
            }
            continue;
        }

        const int deltaRow = targetRow - row;
        const int deltaCol = targetCol - col;
        const bool diagonal = std::abs(deltaRow) == std::abs(deltaCol) && deltaRow != 0;
        const bool straight = (deltaRow == 0) != (deltaCol == 0);
        if ((type == 'b' && !diagonal) || (type == 'r' && !straight) ||
            (type == 'q' && !diagonal && !straight)) {
            continue;
        }
        if (type != 'b' && type != 'r' && type != 'q') {
            continue;
        }

        const int stepRow = (deltaRow > 0) - (deltaRow < 0);
        const int stepCol = (deltaCol > 0) - (deltaCol < 0);
        int scanRow = row + stepRow;
        int scanCol = col + stepCol;
        bool blocked = false;
        while (scanRow != targetRow || scanCol != targetCol) {
            if (squares_[scanRow * 8 + scanCol] != '.') {
                blocked = true;
                break;
            }
            scanRow += stepRow;
            scanCol += stepCol;
        }
        if (!blocked) {
            return true;
        }
    }
    return false;
}

bool Board::isInCheck(bool white) const {
    const char king = white ? 'K' : 'k';
    const auto found = std::find(squares_.begin(), squares_.end(), king);
    if (found == squares_.end()) {
        return true;
    }
    const int kingSquare = static_cast<int>(std::distance(squares_.begin(), found));
    return isSquareAttacked(kingSquare, !white);
}

std::vector<Move> Board::legalMoves() const {
    std::vector<Move> legal;
    const bool movingWhite = whiteToMove_;
    for (const Move& move : pseudoLegalMoves()) {
        Board next = *this;
        next.applyUnchecked(move);
        if (!next.isInCheck(movingWhite)) {
            legal.push_back(move);
        }
    }
    return legal;
}

std::optional<Move> Board::parseLegalMove(const std::string& uci) const {
    std::string normalized;
    normalized.reserve(5);
    for (const char token : uci) {
        if (!std::isspace(static_cast<unsigned char>(token))) {
            normalized.push_back(lower(token));
        }
    }
    for (const Move& move : legalMoves()) {
        if (move.uci() == normalized) {
            return move;
        }
    }
    return std::nullopt;
}

bool Board::makeMove(const Move& move) {
    for (const Move& legal : legalMoves()) {
        if (legal == move) {
            applyUnchecked(legal);
            return true;
        }
    }
    return false;
}

Board Board::after(const Move& move) const {
    Board next = *this;
    next.applyUnchecked(move);
    return next;
}

void Board::applyUnchecked(const Move& move) {
    const char piece = squares_[move.from];
    char captured = squares_[move.to];
    const bool movingWhite = isWhitePiece(piece);
    const bool pawnMove = lower(piece) == 'p';

    squares_[move.from] = '.';
    if (move.enPassant) {
        const int capturedSquare = movingWhite ? move.to + 8 : move.to - 8;
        captured = squares_[capturedSquare];
        squares_[capturedSquare] = '.';
    }

    squares_[move.to] = piece;
    if (move.promotion != '\0') {
        squares_[move.to] = movingWhite
            ? static_cast<char>(std::toupper(static_cast<unsigned char>(move.promotion)))
            : lower(move.promotion);
    }

    if (move.castling || (lower(piece) == 'k' && std::abs(move.to - move.from) == 2)) {
        if (move.to == 62) {
            squares_[61] = squares_[63];
            squares_[63] = '.';
        } else if (move.to == 58) {
            squares_[59] = squares_[56];
            squares_[56] = '.';
        } else if (move.to == 6) {
            squares_[5] = squares_[7];
            squares_[7] = '.';
        } else if (move.to == 2) {
            squares_[3] = squares_[0];
            squares_[0] = '.';
        }
    }

    if (piece == 'K') {
        whiteKingSide_ = false;
        whiteQueenSide_ = false;
    } else if (piece == 'k') {
        blackKingSide_ = false;
        blackQueenSide_ = false;
    } else if (piece == 'R') {
        if (move.from == 63) whiteKingSide_ = false;
        if (move.from == 56) whiteQueenSide_ = false;
    } else if (piece == 'r') {
        if (move.from == 7) blackKingSide_ = false;
        if (move.from == 0) blackQueenSide_ = false;
    }

    if (captured == 'R') {
        if (move.to == 63) whiteKingSide_ = false;
        if (move.to == 56) whiteQueenSide_ = false;
    } else if (captured == 'r') {
        if (move.to == 7) blackKingSide_ = false;
        if (move.to == 0) blackQueenSide_ = false;
    }

    enPassantSquare_ = -1;
    if (pawnMove && std::abs(move.to - move.from) == 16) {
        enPassantSquare_ = (move.from + move.to) / 2;
    }

    if (pawnMove || captured != '.') {
        halfmoveClock_ = 0;
    } else {
        ++halfmoveClock_;
    }
    if (!whiteToMove_) {
        ++fullmoveNumber_;
    }
    whiteToMove_ = !whiteToMove_;
}

std::ostream& operator<<(std::ostream& out, const Board& board) {
    return out << board.pretty();
}

}  // namespace chess
