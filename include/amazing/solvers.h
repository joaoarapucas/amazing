#ifndef AMAZING_SOLVERS_H
#define AMAZING_SOLVERS_H

#include <optional>
#include <vector>

namespace amazing {

class Maze;

enum class SolveAlgo {
  LeftWallFollower,
  RightWallFollower,
  Greedy,
  Dijkstra
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

// Solvers model a robot inside the maze: they only sense the walls of the cell
// they are standing on. Besides that they may know the maze dimensions and the
// coordinates of the end, but never walls of cells they have not visited.
//
// Every solver returns the moves actually walked from start to end, in order
// (including any backtracking out of dead ends), or std::nullopt when the
// solver cannot reach the end. Start == end yields an empty path.

// Keeps the left (or right) hand on the wall. Fails when the end is not on the
// wall the hand touches, which can only happen in mazes with loops.
std::optional<Path> solve_LeftWallFollower(const Maze& maze, int start_x, int start_y, int end_x, int end_y);
std::optional<Path> solve_RightWallFollower(const Maze& maze, int start_x, int start_y, int end_x, int end_y);

// Moves to the open neighbor closest to the end by Manhattan distance, with no
// memory. Fails as soon as it falls into a cycle, e.g. at the bottom of a
// dead end that points toward the end.
std::optional<Path> solve_Greedy(const Maze& maze, int start_x, int start_y, int end_x, int end_y);

// Online Dijkstra (micromouse flood fill): assumes unvisited passages are open,
// follows the shortest path over the walls known so far and replans whenever it
// senses a new wall. Always reaches the end when it is reachable.
std::optional<Path> solve_Dijkstra(const Maze& maze, int start_x, int start_y, int end_x, int end_y);

} // namespace amazing

#endif
