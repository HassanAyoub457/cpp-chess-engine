# Personal Chess Engine in C++

A compact, dependency-free chess engine written in modern C++17. It includes a
complete command-line game loop, legal move generation, check detection,
castling, en passant, promotion, FEN import/export, material evaluation, and a
negamax search with alpha-beta pruning.

## Features

- Legal move generation for every piece
- Castling, en passant, and all four promotion choices
- Check, checkmate, and stalemate detection
- FEN position loading and export
- Minimax-style negamax search with alpha-beta pruning
- Capture-first move ordering
- Interactive UCI-style coordinate input such as `e2e4` or `a7a8q`
- Unit tests including the standard depth-two perft count of 400 positions
- GitHub Actions workflow for automatic builds and tests

## Build

### CMake

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Run the engine:

```bash
./build/chess_engine
```

Run the tests:

```bash
ctest --test-dir build --output-on-failure
```

### Direct compiler command

```bash
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic \
  -Iinclude src/Board.cpp src/Engine.cpp src/main.cpp -o chess_engine
./chess_engine
```

## Commands

| Command | Description |
| --- | --- |
| `board` | Print the current board |
| `moves` | List all legal moves |
| `move e2e4` | Play a legal move |
| `e2e4` | Shorthand for `move e2e4` |
| `go 4` | Ask the engine to search four plies and play its move |
| `eval` | Show the static evaluation in centipawns |
| `fen <FEN>` | Load a position |
| `showfen` | Export the current position |
| `new` | Reset the game |
| `help` | Display available commands |
| `quit` | Exit |

Positive evaluation scores favor White; negative scores favor Black.

## Project structure

```text
include/chess/   Public engine headers
src/             Board, search, and command-line implementation
tests/           Dependency-free unit tests
.github/         Continuous integration workflow
```

## Possible next improvements

- Zobrist hashing and a transposition table
- Iterative deepening and time controls
- Quiescence search
- Piece-square tables and endgame-specific evaluation
- Standard UCI protocol support for chess GUIs

## License

MIT

## Push to GitHub

```bash
git init
git add .
git commit -m "Initial commit: C++ chess engine"
git branch -M main
git remote add origin https://github.com/YOUR_USERNAME/personal-chess-engine.git
git push -u origin main
```
