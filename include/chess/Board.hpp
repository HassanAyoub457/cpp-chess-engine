#pragma once

#include <array>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include "chess/Move.hpp"

namespace chess {

class Board {
public:
    static Board startingPosition();
    static Board fromFEN(const std::string& fen);

    [[nodiscard]] std::string toFEN() const;
    [[nodiscard]] std::string pretty() const;
    [[nodiscard]] std::vector<Move> legalMoves() const;
    [[nodiscard]] std::optional<Move> parseLegalMove(const std::string& uci) const;
    [[nodiscard]] bool isInCheck(bool white) const;
    [[nodiscard]] bool isWhiteToMove() const { return whiteToMove_; }
    [[nodiscard]] char pieceAt(int square) const { return squares_.at(square); }
    [[nodiscard]] Board after(const Move& move) const;

    bool makeMove(const Move& move);

    static int squareFromString(const std::string& square);
    static std::string squareToString(int square);

private:
    std::array<char, 64> squares_{};
    bool whiteToMove_{true};
    bool whiteKingSide_{true};
    bool whiteQueenSide_{true};
    bool blackKingSide_{true};
    bool blackQueenSide_{true};
    int enPassantSquare_{-1};
    int halfmoveClock_{0};
    int fullmoveNumber_{1};

    [[nodiscard]] std::vector<Move> pseudoLegalMoves() const;
    [[nodiscard]] bool isSquareAttacked(int square, bool byWhite) const;
    void applyUnchecked(const Move& move);
};

std::ostream& operator<<(std::ostream& out, const Board& board);

}  // namespace chess
