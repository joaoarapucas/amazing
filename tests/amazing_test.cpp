// Testes da biblioteca amazing.
//
// Todo algoritmo deve gerar um labirinto "perfeito":
//   - paredes consistentes entre células vizinhas;
//   - borda externa fechada;
//   - todas as células visitadas;
//   - exatamente uma passagem a menos que o número de células (árvore);
//   - todas as células alcançáveis a partir de (0, 0).
//
// Todo solver deve devolver uma sequência de movimentos que, reproduzida a
// partir do início, atravessa apenas paredes abertas e termina no fim.

#include <cstdio>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

#include <amazing/generators.h>
#include <amazing/maze.h>
#include <amazing/solvers.h>
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
using amazing::Move;
using amazing::Path;
using amazing::SolveAlgo;

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

const char* name(SolveAlgo algo)
{
    switch (algo) {
    case SolveAlgo::LeftWallFollower: return "LeftWallFollower";
    }
    return "?";
}

void openWall(Maze& maze, int x, int y, int side)
{
    maze.cell(x, y).walls[side] = false;
    maze.cell(x + dx[side], y + dy[side]).walls[(side + 2) % 4] = false;
}

// Distância (em movimentos) do caminho mais curto entre start e end, via BFS.
int shortestDistance(const Maze& maze, int sx, int sy, int ex, int ey)
{
    const int w = maze.get_width();
    std::vector<int> dist(static_cast<size_t>(w) * maze.get_height(), -1);
    std::queue<std::pair<int, int>> queue;
    queue.push({sx, sy});
    dist[static_cast<size_t>(sy) * w + sx] = 0;
    while (!queue.empty()) {
        auto [x, y] = queue.front();
        queue.pop();
        const int d = dist[static_cast<size_t>(y) * w + x];
        if (x == ex && y == ey) return d;
        for (int side = 0; side < 4; ++side) {
            if (maze.cell(x, y).walls[side]) continue;
            const size_t idx = static_cast<size_t>(y + dy[side]) * w + (x + dx[side]);
            if (dist[idx] < 0) {
                dist[idx] = d + 1;
                queue.push({x + dx[side], y + dy[side]});
            }
        }
    }
    return -1;
}

// Reproduz o caminho e verifica que é válido e termina em (ex, ey).
// Devolve o tamanho do caminho após cancelar idas e voltas imediatas
// (ex.: Right seguido de Left), que é o caminho efetivo sem becos.
int replayPath(const Maze& maze, const Path& path, int sx, int sy, int ex, int ey, const char* label)
{
    const int w = maze.get_width();
    const int h = maze.get_height();
    int x = sx;
    int y = sy;
    std::vector<int> reduced;

    for (size_t i = 0; i < path.size(); ++i) {
        const int side = static_cast<int>(path[i]);
        if (side < 0 || side > 3) {
            CHECK(false, "%s: movimento %zu invalido (%d)", label, i, side);
            return -1;
        }
        if (maze.cell(x, y).walls[side]) {
            CHECK(false, "%s: movimento %zu atravessa parede em (%d,%d) lado %d", label, i, x, y, side);
            return -1;
        }
        x += dx[side];
        y += dy[side];
        if (x < 0 || x >= w || y < 0 || y >= h) {
            CHECK(false, "%s: movimento %zu sai do labirinto", label, i);
            return -1;
        }
        if (!reduced.empty() && reduced.back() == (side + 2) % 4) reduced.pop_back();
        else reduced.push_back(side);
    }

    CHECK(x == ex && y == ey, "%s: terminou em (%d,%d), esperado (%d,%d)", label, x, y, ex, ey);
    return static_cast<int>(reduced.size());
}

void checkSolution(const Maze& maze, SolveAlgo algo, int sx, int sy, int ex, int ey, const char* label)
{
    const auto path = maze.solve(algo, sx, sy, ex, ey);
    CHECK(path.has_value(), "%s: sem solucao de (%d,%d) a (%d,%d)", label, sx, sy, ex, ey);
    if (!path) return;

    const int effective = replayPath(maze, *path, sx, sy, ex, ey, label);
    // Em uma árvore, o caminho sem idas e voltas é o único caminho simples,
    // logo tem exatamente o tamanho do caminho mais curto.
    const int shortest = shortestDistance(maze, sx, sy, ex, ey);
    CHECK(effective == shortest, "%s: caminho efetivo %d, mais curto %d", label, effective, shortest);
    // Seguir a parede percorre cada passagem no máximo uma vez em cada sentido.
    const int passages = maze.get_width() * maze.get_height() - 1;
    CHECK(static_cast<int>(path->size()) <= 2 * passages,
          "%s: %zu movimentos, limite %d", label, path->size(), 2 * passages);
}

void testSolversOnGeneratedMazes()
{
    const GenAlgo gens[] = {GenAlgo::DFS, GenAlgo::Prim, GenAlgo::BinaryTree};
    const SolveAlgo solvers[] = {SolveAlgo::LeftWallFollower};
    const int sizes[][2] = {{1, 1}, {1, 7}, {7, 1}, {2, 2}, {5, 3}, {15, 15}, {40, 25}};

    for (SolveAlgo solver : solvers) {
        for (GenAlgo gen : gens) {
            for (const auto& size : sizes) {
                const int w = size[0];
                const int h = size[1];
                for (int run = 0; run < 20; ++run) {
                    Maze maze(w, h);
                    maze.generate(gen);
                    char label[96];
                    std::snprintf(label, sizeof label, "%s/%s %dx%d", name(solver), name(gen), w, h);

                    // Overload padrão: (0, 0) -> (w - 1, h - 1).
                    const auto path = maze.solve(solver);
                    CHECK(path.has_value(), "%s: sem solucao padrao", label);
                    if (path) replayPath(maze, *path, 0, 0, w - 1, h - 1, label);

                    checkSolution(maze, solver, 0, 0, w - 1, h - 1, label);
                    checkSolution(maze, solver, w - 1, h - 1, 0, 0, label);
                    checkSolution(maze, solver, run % w, (run * 7) % h, (run * 3) % w, (run * 5) % h, label);
                }
            }
        }
    }
}

void testLeftWallFollowerKnownPaths()
{
    // Corredor horizontal 3x1.
    {
        Maze maze(3, 1);
        openWall(maze, 0, 0, Cell::Right);
        openWall(maze, 1, 0, Cell::Right);
        const auto path = maze.solve(SolveAlgo::LeftWallFollower);
        CHECK(path && *path == Path({Move::Right, Move::Right}), "corredor: caminho incorreto");
    }

    // Grade 2x2 toda aberta: com a mão esquerda na parede, segue a borda
    // superior (Right, Down). Um seguidor pela direita faria (Down, Right).
    {
        Maze maze(2, 2);
        openWall(maze, 0, 0, Cell::Right);
        openWall(maze, 0, 0, Cell::Bottom);
        openWall(maze, 1, 0, Cell::Bottom);
        openWall(maze, 0, 1, Cell::Right);
        const auto path = maze.solve(SolveAlgo::LeftWallFollower);
        CHECK(path && *path == Path({Move::Right, Move::Down}), "2x2 aberto: caminho incorreto");
    }

    // Beco sem saída: o caminho percorrido inclui a ida e a volta.
    //   (0,0) - (1,0)      inicio (0,1), fim (1,1)
    //     |                (0,1) -> (0,0) -> (1,0) -> volta -> (0,0) -> (0,1) -> (1,1)
    //   (0,1) - (1,1)
    {
        Maze maze(2, 2);
        openWall(maze, 0, 0, Cell::Right);
        openWall(maze, 0, 0, Cell::Bottom);
        openWall(maze, 0, 1, Cell::Right);
        const auto path = maze.solve(SolveAlgo::LeftWallFollower, 0, 1, 1, 1);
        const Path expected = {Move::Up, Move::Right, Move::Left, Move::Down, Move::Right};
        CHECK(path && *path == expected, "beco: caminho incorreto (%zu movimentos)", path ? path->size() : 0);
    }
}

void testLeftWallFollowerEdgeCases()
{
    Maze maze(3, 3);
    maze.generate(GenAlgo::DFS);

    // Início igual ao fim: caminho vazio.
    const auto same = maze.solve(SolveAlgo::LeftWallFollower, 1, 1, 1, 1);
    CHECK(same && same->empty(), "inicio == fim deveria gerar caminho vazio");

    // Coordenadas fora do labirinto.
    CHECK(!maze.solve(SolveAlgo::LeftWallFollower, -1, 0, 2, 2), "inicio fora aceito");
    CHECK(!maze.solve(SolveAlgo::LeftWallFollower, 0, 0, 3, 2), "fim fora aceito");

    // Labirinto sem passagens: célula de início fechada.
    Maze closed(3, 3);
    CHECK(!closed.solve(SolveAlgo::LeftWallFollower), "labirinto fechado resolvido");

    // Fim inalcançável: (0,0) e (1,0) conectados, (2,0) isolado.
    Maze split(3, 1);
    openWall(split, 0, 0, Cell::Right);
    CHECK(!split.solve(SolveAlgo::LeftWallFollower), "fim isolado resolvido");

    // Labirinto com ciclo: grade 3x3 toda aberta. Seguindo a parede externa
    // o resolvedor nunca alcança o centro; deve detectar o laço e parar.
    Maze open(3, 3);
    for (int y = 0; y < 3; ++y)
        for (int x = 0; x < 3; ++x) {
            if (x < 2) openWall(open, x, y, Cell::Right);
            if (y < 2) openWall(open, x, y, Cell::Bottom);
        }
    CHECK(!open.solve(SolveAlgo::LeftWallFollower, 0, 0, 1, 1), "laco nao detectado");
    // Mas alcança qualquer célula da borda.
    const auto border = open.solve(SolveAlgo::LeftWallFollower, 0, 0, 0, 2);
    CHECK(border.has_value(), "borda inalcancavel em grade aberta");
    if (border) replayPath(open, *border, 0, 0, 0, 2, "grade aberta");

    // Borda externa aberta não pode levar para fora do labirinto.
    Maze leaky(2, 1);
    openWall(leaky, 0, 0, Cell::Right);
    leaky.cell(0, 0).walls[Cell::Top] = false;
    leaky.cell(0, 0).walls[Cell::Left] = false;
    const auto inside = leaky.solve(SolveAlgo::LeftWallFollower);
    CHECK(inside && *inside == Path({Move::Right}), "borda aberta: caminho incorreto");
}

} // namespace

int main()
{
    testNewMazeIsClosed();
    testAlgorithms();
    testStartPosition();
    testPrint();
    testSolversOnGeneratedMazes();
    testLeftWallFollowerKnownPaths();
    testLeftWallFollowerEdgeCases();

    if (failures) {
        std::fprintf(stderr, "%d falha(s)\n", failures);
        return 1;
    }
    std::printf("Todos os testes passaram.\n");
    return 0;
}
