#pragma once

#include <cstdint>
#include <optional>

#include "chess/Board.hpp"

namespace chess {

struct SearchResult {
    std::optional<Move> bestMove;
    int score{0};
    std::uint64_t nodes{0};
};

class Engine {
public:
    [[nodiscard]] SearchResult findBestMove(const Board& board, int depth = 4);
    [[nodiscard]] int evaluate(const Board& board) const;

private:
    std::uint64_t nodes_{0};

    int negamax(const Board& board, int depth, int alpha, int beta, int ply);
    [[nodiscard]] int moveOrderingScore(const Board& board, const Move& move) const;
};

}  // namespace chess
