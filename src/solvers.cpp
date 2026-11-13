#include <optional>
#include <vector>
#include <amazing/solvers.h>
#include <amazing/maze.h>

namespace amazing {

namespace {

enum class Hand { Left, Right };

std::optional<Path> follow_wall(const Maze& maze, Hand hand, int start_x, int start_y, int end_x, int end_y) {
    const int width = maze.get_width();
    const int height = maze.get_height();

    auto inside = [&](int x, int y) {
        return x >= 0 && x < width && y >= 0 && y < height;
    };

    if (!inside(start_x, start_y) || !inside(end_x, end_y)) return std::nullopt;

    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};

    // open wall that leads to a cell inside the maze
    auto can_move = [&](int x, int y, int dir) {
        return !maze.cell(x, y).walls[dir] && inside(x + dx[dir], y + dy[dir]);
    };

    // Directions are clockwise (Top, Right, Bottom, Left): +1 turns right, +3 turns left.
    const int toward_wall = hand == Hand::Left ? 3 : 1;
    const int away_from_wall = hand == Hand::Left ? 1 : 3;

    // The walk is deterministic on (cell, heading). With 4 * width * height
    // possible states, walking more moves than that means a state repeated
    // without reaching the end: the follower is circling a loop.
    const size_t max_moves = static_cast<size_t>(4) * width * height;

    Path path;
    int x = start_x;
    int y = start_y;
    int heading = Cell::Right;

    while (x != end_x || y != end_y) {
        if (path.size() >= max_moves) return std::nullopt;

        // keep the hand on the wall: turn toward it, straight, turn away, back
        const int order[] = {
            (heading + toward_wall) % 4,
            heading,
            (heading + away_from_wall) % 4,
            (heading + 2) % 4
        };

        int next = -1;
        for (int dir : order) {
            if (can_move(x, y, dir)) {
                next = dir;
                break;
            }
        }

        // enclosed cell: nowhere to go
        if (next < 0) return std::nullopt;

        heading = next;
        x += dx[next];
        y += dy[next];
        path.push_back(static_cast<Move>(next));
    }

    return path;
}

} // namespace

std::optional<Path> solve_LeftWallFollower(const Maze& maze, int start_x, int start_y, int end_x, int end_y) {
    return follow_wall(maze, Hand::Left, start_x, start_y, end_x, end_y);
}

std::optional<Path> solve_RightWallFollower(const Maze& maze, int start_x, int start_y, int end_x, int end_y) {
    return follow_wall(maze, Hand::Right, start_x, start_y, end_x, end_y);
}

} // namespace amazing
