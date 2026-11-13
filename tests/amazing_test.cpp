// Testes da biblioteca amazing.
//
// Todo algoritmo deve gerar um labirinto "perfeito":
//   - paredes consistentes entre células vizinhas;
//   - borda externa fechada;
//   - todas as células visitadas;
//   - exatamente uma passagem a menos que o número de células (árvore);
//   - todas as células alcançáveis a partir de (0, 0).

#include <cstdio>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

#include <amazing/generators.h>
#include <amazing/maze.h>
#include <amazing/ui.h>

namespace {

int failures = 0;

#define CHECK(cond, ...)                                    \
    do {                                                    \
        if (!(cond)) {                                      \
            ++failures;                                     \
            std::fprintf(stderr, "FALHA %s:%d: %s | ",      \
                         __FILE__, __LINE__, #cond);        \
            std::fprintf(stderr, __VA_ARGS__);              \
            std::fprintf(stderr, "\n");                     \
        }                                                   \
    } while (0)

using amazing::Cell;
using amazing::GenAlgo;
using amazing::Maze;

const int dx[] = {0, 1, 0, -1};
const int dy[] = {-1, 0, 1, 0};

const char* name(GenAlgo algo)
{
    switch (algo) {
    case GenAlgo::DFS: return "DFS";
    case GenAlgo::Prim: return "Prim";
    case GenAlgo::BinaryTree: return "BinaryTree";
    }
    return "?";
}

void checkPerfectMaze(const Maze& maze, const char* label)
{
    const int w = maze.get_width();
    const int h = maze.get_height();
    int passages = 0;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const Cell& c = maze.cell(x, y);
            CHECK(c.visited, "%s: celula (%d,%d) nao visitada", label, x, y);

            for (int side = 0; side < 4; ++side) {
                const int nx = x + dx[side];
                const int ny = y + dy[side];
                const bool inside = nx >= 0 && nx < w && ny >= 0 && ny < h;

                if (!inside) {
                    CHECK(c.walls[side], "%s: borda aberta em (%d,%d) lado %d", label, x, y, side);
                    continue;
                }
                const bool opposite = maze.cell(nx, ny).walls[(side + 2) % 4];
                CHECK(c.walls[side] == opposite,
                      "%s: parede assimetrica entre (%d,%d) e (%d,%d)", label, x, y, nx, ny);
                // Conta cada passagem uma única vez (lados Right e Bottom).
                if ((side == Cell::Right || side == Cell::Bottom) && !c.walls[side]) ++passages;
            }
        }
    }

    CHECK(passages == w * h - 1, "%s: %d passagens, esperado %d", label, passages, w * h - 1);

    std::vector<bool> seen(static_cast<size_t>(w) * h, false);
    std::queue<std::pair<int, int>> queue;
    queue.push({0, 0});
    seen[0] = true;
    int reached = 0;
    while (!queue.empty()) {
        auto [x, y] = queue.front();
        queue.pop();
        ++reached;
        for (int side = 0; side < 4; ++side) {
            if (maze.cell(x, y).walls[side]) continue;
            const int nx = x + dx[side];
            const int ny = y + dy[side];
            const size_t idx = static_cast<size_t>(ny) * w + nx;
            if (!seen[idx]) {
                seen[idx] = true;
                queue.push({nx, ny});
            }
        }
    }
    CHECK(reached == w * h, "%s: %d de %d celulas alcancaveis", label, reached, w * h);
}

void testAlgorithms()
{
    const GenAlgo algos[] = {GenAlgo::DFS, GenAlgo::Prim, GenAlgo::BinaryTree};
    const int sizes[][2] = {{1, 1}, {1, 7}, {7, 1}, {2, 2}, {5, 3}, {15, 15}, {40, 25}};

    for (GenAlgo algo : algos) {
        for (const auto& size : sizes) {
            for (int run = 0; run < 20; ++run) {
                Maze maze(size[0], size[1]);
                maze.generate(algo);
                char label[64];
                std::snprintf(label, sizeof label, "%s %dx%d", name(algo), size[0], size[1]);
                checkPerfectMaze(maze, label);
            }
        }
    }
}

void testStartPosition()
{
    // DFS e Prim aceitam ponto de partida arbitrário.
    for (GenAlgo algo : {GenAlgo::DFS, GenAlgo::Prim}) {
        Maze maze(9, 6);
        maze.generate(algo, 8, 5);
        checkPerfectMaze(maze, name(algo));
    }
}

void testNewMazeIsClosed()
{
    Maze maze(3, 2);
    CHECK(maze.get_width() == 3 && maze.get_height() == 2, "dimensoes incorretas");
    for (int y = 0; y < 2; ++y)
        for (int x = 0; x < 3; ++x) {
            const Cell& c = maze.cell(x, y);
            CHECK(!c.visited, "celula nova ja visitada");
            for (bool wall : c.walls) CHECK(wall, "celula nova com parede aberta");
        }
}

void testPrint()
{
    Maze maze(4, 3);
    maze.generate(GenAlgo::DFS);

    std::ostringstream lines;
    amazing::print(maze, amazing::OutputStyle::CrossAndLine, lines);
    int count = 0;
    std::string line;
    std::istringstream in(lines.str());
    while (std::getline(in, line)) {
        ++count;
        CHECK(line.size() == 4 * 4 + 1, "CrossAndLine: linha com %zu chars", line.size());
    }
    CHECK(count == 2 * 3 + 1, "CrossAndLine: %d linhas, esperado 7", count);

    std::ostringstream blocks;
    amazing::print(maze, amazing::OutputStyle::Block, blocks);
    count = 0;
    std::istringstream in2(blocks.str());
    while (std::getline(in2, line)) ++count;
    CHECK(count == 2 * 3 + 1, "Block: %d linhas, esperado 7", count);
}

} // namespace

int main()
{
    testNewMazeIsClosed();
    testAlgorithms();
    testStartPosition();
    testPrint();

    if (failures) {
        std::fprintf(stderr, "%d falha(s)\n", failures);
        return 1;
    }
    std::printf("Todos os testes passaram.\n");
    return 0;
}
