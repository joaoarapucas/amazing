#include <optional>
#include <vector>
#include <amazing/solvers.h>
#include <amazing/maze.h>

namespace amazing {

std::optional<Path> solve_LeftWallFollower(const Maze& maze, int start_x, int start_y, int end_x, int end_y) {
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

        // keep the left hand on the wall: left, straight, right, back
        const int order[] = {(heading + 3) % 4, heading, (heading + 1) % 4, (heading + 2) % 4};

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

} // namespace amazing
