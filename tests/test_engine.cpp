#include <cstdlib>
#include <iostream>
#include <string>

#include "chess/Board.hpp"
#include "chess/Engine.hpp"

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

std::uint64_t perft(const chess::Board& board, int depth) {
    if (depth == 0) return 1;
    std::uint64_t nodes = 0;
    for (const chess::Move& move : board.legalMoves()) {
        nodes += perft(board.after(move), depth - 1);
    }
    return nodes;
}

void play(chess::Board& board, const std::string& uci) {
    const auto move = board.parseLegalMove(uci);
    expect(move.has_value(), "Expected legal move " + uci);
    if (move) {
        expect(board.makeMove(*move), "Move should be applied: " + uci);
    }
}

}  // namespace

int main() {
    chess::Board start = chess::Board::startingPosition();
    expect(start.legalMoves().size() == 20, "Starting position has 20 legal moves");
    expect(perft(start, 2) == 400, "Starting position perft(2) equals 400");
    expect(start.toFEN() ==
               "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
           "Starting FEN round-trips");

    play(start, "e2e4");
    expect(!start.isWhiteToMove(), "Black moves after e2e4");
    expect(start.legalMoves().size() == 20, "Black has 20 replies to e2e4");
    expect(start.toFEN().find(" e3 ") != std::string::npos,
           "Double pawn move sets the en passant target");

    chess::Board mate = chess::Board::startingPosition();
    play(mate, "e2e4");
    play(mate, "e7e5");
    play(mate, "f1c4");
    play(mate, "b8c6");
    play(mate, "d1h5");
    play(mate, "g8f6");
    play(mate, "h5f7");
    expect(mate.isInCheck(false), "Black is in check in Scholar's Mate");
    expect(mate.legalMoves().empty(), "Scholar's Mate has no legal replies");

    chess::Board castling = chess::Board::fromFEN(
        "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    expect(castling.parseLegalMove("e1g1").has_value(), "White can castle king-side");
    expect(castling.parseLegalMove("e1c1").has_value(), "White can castle queen-side");

    chess::Board promotion = chess::Board::fromFEN(
        "4k3/P7/8/8/8/8/8/4K3 w - - 0 1");
    expect(promotion.parseLegalMove("a7a8q").has_value(), "Queen promotion is legal");
    expect(promotion.parseLegalMove("a7a8n").has_value(), "Knight promotion is legal");

    chess::Engine engine;
    const chess::SearchResult search = engine.findBestMove(
        chess::Board::startingPosition(), 2);
    expect(search.bestMove.has_value(), "Engine finds a move");
    expect(search.nodes > 0, "Engine searches at least one node");

    if (failures == 0) {
        std::cout << "All chess-engine tests passed.\n";
        return EXIT_SUCCESS;
    }
    std::cerr << failures << " test(s) failed.\n";
    return EXIT_FAILURE;
}
