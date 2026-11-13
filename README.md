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

Os resolvedores simulam um robô dentro do labirinto: só enxergam as paredes da
célula em que estão. Além disso podem conhecer as dimensões do labirinto e as
coordenadas do fim, mas nunca paredes de células por onde não passaram.

| `SolveAlgo`         | Descrição                                                        | Garante chegar?          |
|---------------------|------------------------------------------------------------------|--------------------------|
| `LeftWallFollower`  | Segue a parede pela esquerda                                     | Só em labirintos perfeitos |
| `RightWallFollower` | Segue a parede pela direita                                      | Só em labirintos perfeitos |
| `Greedy`            | Vai para o vizinho aberto mais próximo do fim (Manhattan), sem memória | Não                |
| `Dijkstra`          | Dijkstra online (flood fill do Micromouse): supõe passagens não vistas abertas, segue o caminho mais curto do mapa conhecido e replaneja ao descobrir paredes | Sim, sempre que o fim é alcançável |

Um resolvedor devolve `std::optional<Path>`, onde `Path` é um
`std::vector<Move>` com todos os movimentos (`Up`, `Right`, `Down`, `Left`)
efetivamente percorridos, na ordem, incluindo idas e voltas em becos sem saída.
Devolve `std::nullopt` quando não alcança o fim (coordenadas fora do labirinto,
fim inalcançável ou laço detectado). Início igual ao fim gera um caminho vazio.

Os valores de `Move` coincidem com `Cell::Side`, então
`maze.cell(x, y).walls[static_cast<int>(move)]` é a parede atravessada.

Quando um resolvedor entra em ciclo (o seguidor de parede circulando uma
"ilha", o guloso indo e voltando de um beco), o laço é detectado e o resultado
é `std::nullopt`.

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
