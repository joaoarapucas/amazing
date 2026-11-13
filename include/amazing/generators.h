#ifndef AMAZING_GENERATORS_H
#define AMAZING_GENERATORS_H

namespace amazing {

class Maze;

enum class GenAlgo {
  DFS,
  Prim,
  BinaryTree
};

void generate_DFS(Maze& maze, int x, int y);
void generate_Prim(Maze& maze, int x, int y);
void generate_BinaryTree(Maze& maze);

} // namespace amazing

#endif
