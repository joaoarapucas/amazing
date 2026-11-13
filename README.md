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

## Uso

```cpp
#include <amazing/maze.h>
#include <amazing/ui.h>

amazing::Maze maze(15, 15);
maze.generate(amazing::GenAlgo::DFS);          // ponto de partida opcional: (x, y)
amazing::print(maze, amazing::OutputStyle::Block);

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
