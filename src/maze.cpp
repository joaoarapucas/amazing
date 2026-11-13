#include <vector>
#include <amazing/maze.h>
#include <amazing/generators.h>
#include <amazing/solvers.h>

namespace amazing {

Maze::Maze(int w, int h) : width(w), height(h), grid(h, std::vector<Cell>(w)) {}

void Maze::generate(GenAlgo algo, int start_x, int start_y) {
  switch (algo) {
    case GenAlgo::DFS:
      generate_DFS(*this, start_x, start_y);
    break;

    case GenAlgo::Prim:
      generate_Prim(*this, start_x, start_y);
    break;

    case GenAlgo::BinaryTree:
      generate_BinaryTree(*this);
    break;
  }
}

std::optional<Path> Maze::solve(SolveAlgo algo) const {
  return solve(algo, 0, 0, width - 1, height - 1);
}

std::optional<Path> Maze::solve(SolveAlgo algo, int start_x, int start_y, int end_x, int end_y) const {
  switch (algo) {
    case SolveAlgo::LeftWallFollower:
      return solve_LeftWallFollower(*this, start_x, start_y, end_x, end_y);

    case SolveAlgo::RightWallFollower:
      return solve_RightWallFollower(*this, start_x, start_y, end_x, end_y);
  }
  return std::nullopt;
}

} // namespace amazing
