#include <vector>
#include <amazing/maze.h>
#include <amazing/generators.h>

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

} // namespace amazing
