#pragma once

#include <string>

namespace chess {

struct Move {
    int from{-1};
    int to{-1};
    char promotion{'\0'};
    bool enPassant{false};
    bool castling{false};

    [[nodiscard]] std::string uci() const;

    bool operator==(const Move& other) const {
        return from == other.from && to == other.to &&
               promotion == other.promotion;
    }
};

}  // namespace chess
