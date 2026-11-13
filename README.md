# amazing

Biblioteca de geração de labirintos em C++17.

## Algoritmos

| `GenAlgo`    | Descrição                                   |
|--------------|---------------------------------------------|
| `DFS`        | Busca em profundidade (recursive backtracker) |
| `Prim`       | Prim aleatorizado                           |
| `BinaryTree` | Árvore binária (viés para cima/direita)     |

Todos geram labirintos perfeitos: existe exatamente um caminho entre quaisquer
duas células.

## Resolvedores

| `SolveAlgo`        | Descrição                                          |
|--------------------|----------------------------------------------------|
| `LeftWallFollower` | Segue a parede pela esquerda até alcançar o fim    |
| `RightWallFollower`| Segue a parede pela direita até alcançar o fim     |

Um resolvedor devolve `std::optional<Path>`, onde `Path` é um
`std::vector<Move>` com todos os movimentos (`Up`, `Right`, `Down`, `Left`)
efetivamente percorridos, na ordem, incluindo idas e voltas em becos sem saída.
Devolve `std::nullopt` quando não alcança o fim (coordenadas fora do labirinto,
fim inalcançável ou laço detectado). Início igual ao fim gera um caminho vazio.

Os valores de `Move` coincidem com `Cell::Side`, então
`maze.cell(x, y).walls[static_cast<int>(move)]` é a parede atravessada.

O seguidor de parede sempre encontra a saída em labirintos perfeitos. Em
labirintos com ciclos ele pode circular uma "ilha" sem chegar ao fim; nesse
caso o laço é detectado e o resultado é `std::nullopt`.

## Uso

```cpp
#include <amazing/maze.h>
#include <amazing/ui.h>

amazing::Maze maze(15, 15);
maze.generate(amazing::GenAlgo::DFS);          // ponto de partida opcional: (x, y)
amazing::print(maze, amazing::OutputStyle::Block);

// (0, 0) -> (largura - 1, altura - 1), ou informe início e fim explicitamente
auto path = maze.solve(amazing::SolveAlgo::LeftWallFollower);
if (path) {
  for (amazing::Move m : *path) { /* ... */ }
}

const amazing::Cell& c = maze.cell(0, 0);
bool fechadoEmCima = c.walls[amazing::Cell::Top];
```

## Build

```bash
make          # build/libamazing.a + build/maze (demo)
make lib      # apenas a biblioteca
make run      # executa o demo
make test     # executa os testes
make clean
```

## Usando em outro projeto (git submodule)

```bash
git submodule add git@github.com:joaoarapucas/amazing.git external/amazing
```

No Makefile do projeto:

```make
AMAZING_DIR := external/amazing
AMAZING_LIB := $(AMAZING_DIR)/build/libamazing.a

CPPFLAGS += -I$(AMAZING_DIR)/include

$(AMAZING_LIB): FORCE
	$(MAKE) -C $(AMAZING_DIR) lib CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)"

FORCE:

meu_programa: $(OBJS) $(AMAZING_LIB)
	$(CXX) $^ -o $@
```
