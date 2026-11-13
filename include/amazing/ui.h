#ifndef AMAZING_UI_H
#define AMAZING_UI_H

#include <iostream>

namespace amazing {

class Maze;

enum class OutputStyle {
  CrossAndLine,
  Block
};

void print(const Maze& maze, OutputStyle style, std::ostream& out = std::cout);

} // namespace amazing

#endif
