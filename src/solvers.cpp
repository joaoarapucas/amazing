#include <array>
#include <climits>
#include <cstdlib>
#include <functional>
#include <optional>
#include <queue>
#include <utility>
#include <vector>
#include <amazing/solvers.h>
#include <amazing/maze.h>

namespace amazing {

namespace {

const int dx[] = {0, 1, 0, -1};
const int dy[] = {-1, 0, 1, 0};

bool inside(const Maze& maze, int x, int y) {
    return x >= 0 && x < maze.get_width() && y >= 0 && y < maze.get_height();
}

// open wall that leads to a cell inside the maze
bool can_move(const Maze& maze, int x, int y, int dir) {
    return !maze.cell(x, y).walls[dir] && inside(maze, x + dx[dir], y + dy[dir]);
}

enum class Hand { Left, Right };

std::optional<Path> follow_wall(const Maze& maze, Hand hand, int start_x, int start_y, int end_x, int end_y) {
    if (!inside(maze, start_x, start_y) || !inside(maze, end_x, end_y)) return std::nullopt;

    // Directions are clockwise (Top, Right, Bottom, Left): +1 turns right, +3 turns left.
    const int toward_wall = hand == Hand::Left ? 3 : 1;
    const int away_from_wall = hand == Hand::Left ? 1 : 3;

    // The walk is deterministic on (cell, heading). With 4 * width * height
    // possible states, walking more moves than that means a state repeated
    // without reaching the end: the follower is circling a loop.
    const size_t max_moves = static_cast<size_t>(4) * maze.get_width() * maze.get_height();

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
            if (can_move(maze, x, y, dir)) {
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

std::optional<Path> solve_Greedy(const Maze& maze, int start_x, int start_y, int end_x, int end_y) {
    if (!inside(maze, start_x, start_y) || !inside(maze, end_x, end_y)) return std::nullopt;

    // The choice depends only on the current cell, so walking more moves than
    // there are cells means a cell repeated: the walk is stuck in a cycle.
    const size_t max_moves = static_cast<size_t>(maze.get_width()) * maze.get_height();

    Path path;
    int x = start_x;
    int y = start_y;

    while (x != end_x || y != end_y) {
        if (path.size() >= max_moves) return std::nullopt;

        // open neighbor closest to the end (Manhattan distance); ties keep
        // the first direction in Top, Right, Bottom, Left order
        int next = -1;
        int next_distance = INT_MAX;
        for (int dir = 0; dir < 4; dir++) {
            if (!can_move(maze, x, y, dir)) continue;

            const int distance = std::abs(x + dx[dir] - end_x) + std::abs(y + dy[dir] - end_y);
            if (distance < next_distance) {
                next = dir;
                next_distance = distance;
            }
        }

        if (next < 0) return std::nullopt;

        x += dx[next];
        y += dy[next];
        path.push_back(static_cast<Move>(next));
    }

    return path;
}

std::optional<Path> solve_Dijkstra(const Maze& maze, int start_x, int start_y, int end_x, int end_y) {
    if (!inside(maze, start_x, start_y) || !inside(maze, end_x, end_y)) return std::nullopt;

    const int width = maze.get_width();
    const int height = maze.get_height();
    const int unreachable = INT_MAX;

    auto index = [&](int x, int y) { return y * width + x; };

    // Walls the solver knows about. It starts knowing only the outer border
    // and assumes every other passage is open until it senses otherwise.
    std::vector<std::array<bool, 4>> known_wall(static_cast<size_t>(width) * height, {false, false, false, false});
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            for (int dir = 0; dir < 4; dir++) {
                if (!inside(maze, x + dx[dir], y + dy[dir])) known_wall[index(x, y)][dir] = true;
            }
        }
    }

    // reads the walls of the current cell; returns true if any was new
    auto sense = [&](int x, int y) {
        bool changed = false;
        for (int dir = 0; dir < 4; dir++) {
            if (!maze.cell(x, y).walls[dir] || known_wall[index(x, y)][dir]) continue;

            known_wall[index(x, y)][dir] = true;
            const int nx = x + dx[dir];
            const int ny = y + dy[dir];
            if (inside(maze, nx, ny)) known_wall[index(nx, ny)][(dir + 2) % 4] = true;
            changed = true;
        }
        return changed;
    };

    // distance from every cell to the end over the known map
    std::vector<int> dist;
    auto plan = [&]() {
        dist.assign(static_cast<size_t>(width) * height, unreachable);
        using Entry = std::pair<int, int>; // (distance, cell index)
        std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> queue;

        dist[index(end_x, end_y)] = 0;
        queue.push({0, index(end_x, end_y)});

        while (!queue.empty()) {
            auto [d, cur] = queue.top();
            queue.pop();
            if (d > dist[cur]) continue;

            const int x = cur % width;
            const int y = cur / width;
            for (int dir = 0; dir < 4; dir++) {
                if (known_wall[cur][dir]) continue;

                const int prev = index(x + dx[dir], y + dy[dir]);
                if (d + 1 < dist[prev]) {
                    dist[prev] = d + 1;
                    queue.push({d + 1, prev});
                }
            }
        }
    };

    Path path;
    int x = start_x;
    int y = start_y;
    bool planned = false;

    // Each move either follows the plan one step closer to the end or comes
    // right after a new wall was sensed. Walls are finite, so the walk ends.
    while (x != end_x || y != end_y) {
        if (sense(x, y) || !planned) {
            plan();
            planned = true;
        }

        const int here = index(x, y);
        if (dist[here] == unreachable) return std::nullopt;

        // the current cell was just sensed, so its known walls are its real walls
        int next = -1;
        for (int dir = 0; dir < 4; dir++) {
            if (known_wall[here][dir]) continue;

            if (dist[index(x + dx[dir], y + dy[dir])] == dist[here] - 1) {
                next = dir;
                break;
            }
        }

        // unreachable: a finite distance always has a neighbor one step closer
        if (next < 0) return std::nullopt;

        x += dx[next];
        y += dy[next];
        path.push_back(static_cast<Move>(next));
    }

    return path;
}

} // namespace amazing
