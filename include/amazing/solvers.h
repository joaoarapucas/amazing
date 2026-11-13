#ifndef AMAZING_SOLVERS_H
#define AMAZING_SOLVERS_H

#include <optional>
#include <vector>

namespace amazing {

class Maze;

enum class SolveAlgo {
  LeftWallFollower,
  RightWallFollower
};

// A single step between neighboring cells. Values match Cell::Side, so a move
// can index Cell::walls directly: maze.cell(x, y).walls[static_cast<int>(move)].
enum class Move {
  Up,
  Right,
  Down,
  Left
};

using Path = std::vector<Move>;

// Every solver returns the moves actually walked from start to end, in order
// (including any backtracking out of dead ends), or std::nullopt when the
// solver cannot reach the end. Start == end yields an empty path.
std::optional<Path> solve_LeftWallFollower(const Maze& maze, int start_x, int start_y, int end_x, int end_y);
std::optional<Path> solve_RightWallFollower(const Maze& maze, int start_x, int start_y, int end_x, int end_y);

} // namespace amazing

#endif
