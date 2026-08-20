#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>

#include "chess/Board.hpp"
#include "chess/Engine.hpp"

namespace {

std::string trim(const std::string& text) {
    const auto first = std::find_if_not(text.begin(), text.end(), [](unsigned char value) {
        return std::isspace(value);
    });
    const auto last = std::find_if_not(text.rbegin(), text.rend(), [](unsigned char value) {
        return std::isspace(value);
    }).base();
    return first < last ? std::string(first, last) : std::string{};
}

void printHelp() {
    std::cout
        << "Commands:\n"
        << "  board            Print the board\n"
        << "  moves            List legal moves\n"
        << "  move e2e4        Play a move (bare e2e4 also works)\n"
        << "  go [depth]       Search and play an engine move (default: 4)\n"
        << "  eval             Print the static evaluation\n"
        << "  fen <FEN>        Load a FEN position\n"
        << "  showfen          Print the current FEN\n"
        << "  new              Reset to the starting position\n"
        << "  help             Show this help\n"
        << "  quit             Exit\n";
}

void printGameState(const chess::Board& board) {
    const auto legal = board.legalMoves();
    if (!legal.empty()) return;
    if (board.isInCheck(board.isWhiteToMove())) {
        std::cout << "Checkmate. "
                  << (board.isWhiteToMove() ? "Black" : "White")
                  << " wins.\n";
    } else {
        std::cout << "Stalemate.\n";
    }
}

}  // namespace

int main() {
    chess::Board board = chess::Board::startingPosition();
    chess::Engine engine;

    std::cout << "Personal Chess Engine (C++17)\n";
    std::cout << "Type 'help' for commands. Moves use UCI coordinates such as e2e4.\n";
    std::cout << board;

    std::string line;
    while (std::cout << "> " && std::getline(std::cin, line)) {
        line = trim(line);
        if (line.empty()) continue;

        std::istringstream commandStream(line);
        std::string command;
        commandStream >> command;
        std::transform(command.begin(), command.end(), command.begin(), [](unsigned char value) {
            return static_cast<char>(std::tolower(value));
        });

        if (command == "quit" || command == "exit") {
            break;
        }
        if (command == "help") {
            printHelp();
            continue;
        }
        if (command == "board") {
            std::cout << board;
            continue;
        }
        if (command == "new") {
            board = chess::Board::startingPosition();
            std::cout << board;
            continue;
        }
        if (command == "showfen") {
            std::cout << board.toFEN() << '\n';
            continue;
        }
        if (command == "moves") {
            const auto moves = board.legalMoves();
            for (std::size_t index = 0; index < moves.size(); ++index) {
                std::cout << moves[index].uci()
                          << ((index + 1) % 12 == 0 ? "\n" : " ");
            }
            if (moves.size() % 12 != 0) std::cout << '\n';
            std::cout << moves.size() << " legal move(s).\n";
            continue;
        }
        if (command == "eval") {
            std::cout << "Evaluation: " << engine.evaluate(board)
                      << " centipawns (positive favors White).\n";
            continue;
        }
        if (command == "fen") {
            std::string fen;
            std::getline(commandStream, fen);
            try {
                board = chess::Board::fromFEN(trim(fen));
                std::cout << board;
            } catch (const std::exception& error) {
                std::cout << "Invalid FEN: " << error.what() << '\n';
            }
            continue;
        }
        if (command == "go") {
            int depth = 4;
            commandStream >> depth;
            depth = std::clamp(depth, 1, 7);
            std::cout << "Searching depth " << depth << "...\n";
            const chess::SearchResult result = engine.findBestMove(board, depth);
            if (!result.bestMove) {
                printGameState(board);
                continue;
            }
            std::cout << "Engine move: " << result.bestMove->uci()
                      << " | score: " << result.score
                      << " | nodes: " << result.nodes << '\n';
            board.makeMove(*result.bestMove);
            std::cout << board;
            printGameState(board);
            continue;
        }

        std::string moveText;
        if (command == "move") {
            commandStream >> moveText;
        } else if (command.size() == 4 || command.size() == 5) {
            moveText = command;
        }
        if (!moveText.empty()) {
            const auto move = board.parseLegalMove(moveText);
            if (!move) {
                std::cout << "Illegal move. Use 'moves' to list legal moves.\n";
                continue;
            }
            board.makeMove(*move);
            std::cout << board;
            printGameState(board);
            continue;
        }

        std::cout << "Unknown command. Type 'help'.\n";
    }

    std::cout << "Good game.\n";
    return 0;
}
