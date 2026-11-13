#ifndef AMAZING_MAZE_H
#define AMAZING_MAZE_H

#include <optional>
#include <vector>
#include "generators.h"
#include "solvers.h"

namespace amazing {

struct Cell {
  enum Side { Top, Right, Bottom, Left };

  bool visited = false;
  bool walls[4] = {true, true, true, true};
};

class Maze {
  int width, height;
  std::vector<std::vector<Cell>> grid;

  public:
    Maze(int w, int h);

    int get_width() const { return width; }
    int get_height() const { return height; }
    Cell& cell(int x, int y) { return grid[y][x]; }
    const Cell& cell(int x, int y) const { return grid[y][x]; }

    void generate(GenAlgo algo, int start_x = 0, int start_y = 0);

    // Solves from (0, 0) to (width - 1, height - 1).
    std::optional<Path> solve(SolveAlgo algo) const;
    std::optional<Path> solve(SolveAlgo algo, int start_x, int start_y, int end_x, int end_y) const;

    void to_json();
};

} // namespace amazing

#endif
