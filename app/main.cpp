#include <amazing/maze.h>
#include <amazing/generators.h>
#include <amazing/ui.h>

int main() {
    amazing::Maze maze(15, 15);
    maze.generate(amazing::GenAlgo::DFS);
    amazing::print(maze, amazing::OutputStyle::Block);
    return 0;
}
