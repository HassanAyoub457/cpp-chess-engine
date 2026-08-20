#include "chess/Engine.hpp"

#include <algorithm>
#include <cctype>
#include <limits>
#include <vector>

namespace chess {
namespace {

constexpr int kInfinity = 1'000'000;
constexpr int kMateScore = 100'000;

int pieceValue(char piece) {
    switch (static_cast<char>(std::tolower(static_cast<unsigned char>(piece)))) {
        case 'p': return 100;
        case 'n': return 320;
        case 'b': return 330;
        case 'r': return 500;
        case 'q': return 900;
        default: return 0;
    }
}

bool isWhitePiece(char piece) {
    return piece >= 'A' && piece <= 'Z';
}

}  // namespace

int Engine::evaluate(const Board& board) const {
    int score = 0;
    for (int square = 0; square < 64; ++square) {
        const char piece = board.pieceAt(square);
        if (piece == '.') continue;
        const int value = pieceValue(piece);
        score += isWhitePiece(piece) ? value : -value;

        // Small centralization bonus for minor pieces, expressed in centipawns.
        const char type = static_cast<char>(
            std::tolower(static_cast<unsigned char>(piece)));
        if (type == 'n' || type == 'b') {
            const int row = square / 8;
            const int col = square % 8;
            const int distance = std::abs(3 - row) + std::abs(3 - col);
            const int bonus = std::max(0, 18 - 3 * distance);
            score += isWhitePiece(piece) ? bonus : -bonus;
        }
    }
    return score;
}

int Engine::moveOrderingScore(const Board& board, const Move& move) const {
    int score = 0;
    const char attacker = board.pieceAt(move.from);
    const char victim = move.enPassant ? 'p' : board.pieceAt(move.to);
    if (victim != '.') {
        score += 10 * pieceValue(victim) - pieceValue(attacker);
    }
    if (move.promotion != '\0') {
        score += pieceValue(move.promotion) + 800;
    }
    if (move.castling) {
        score += 40;
    }
    return score;
}

int Engine::negamax(const Board& board, int depth, int alpha, int beta, int ply) {
    ++nodes_;
    std::vector<Move> moves = board.legalMoves();
    if (moves.empty()) {
        if (board.isInCheck(board.isWhiteToMove())) {
            return -kMateScore + ply;
        }
        return 0;
    }
    if (depth == 0) {
        const int whiteScore = evaluate(board);
        return board.isWhiteToMove() ? whiteScore : -whiteScore;
    }

    std::stable_sort(moves.begin(), moves.end(), [&](const Move& left, const Move& right) {
        return moveOrderingScore(board, left) > moveOrderingScore(board, right);
    });

    int best = -kInfinity;
    for (const Move& move : moves) {
        const Board next = board.after(move);
        const int score = -negamax(next, depth - 1, -beta, -alpha, ply + 1);
        best = std::max(best, score);
        alpha = std::max(alpha, score);
        if (alpha >= beta) {
            break;
        }
    }
    return best;
}

SearchResult Engine::findBestMove(const Board& board, int depth) {
    depth = std::max(1, depth);
    nodes_ = 0;
    std::vector<Move> moves = board.legalMoves();
    if (moves.empty()) {
        const int score = board.isInCheck(board.isWhiteToMove()) ? -kMateScore : 0;
        return SearchResult{std::nullopt, score, nodes_};
    }

    std::stable_sort(moves.begin(), moves.end(), [&](const Move& left, const Move& right) {
        return moveOrderingScore(board, left) > moveOrderingScore(board, right);
    });

    int alpha = -kInfinity;
    const int beta = kInfinity;
    int bestScore = -kInfinity;
    Move bestMove = moves.front();

    for (const Move& move : moves) {
        const Board next = board.after(move);
        const int score = -negamax(next, depth - 1, -beta, -alpha, 1);
        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
        alpha = std::max(alpha, score);
    }

    return SearchResult{bestMove, bestScore, nodes_};
}

}  // namespace chess
