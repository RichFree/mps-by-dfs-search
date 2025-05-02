# Maximum Planar Subgraph approximation by searching DFS Trees

This repo contains the implementation that uses a DFS-tree local search
procedure to find a better MPS approximation. It uses the Deferred Planarity
Test to generate in linear-time a new Maximal Planar Subgraph for a given DFS
candidate.

## Dependencies

This project uses C++ and GNU Make.

This project uses the `OGDF` library (version: [dogwood](https://github.com/ogdf/ogdf/releases/tag/dogwood-202202))

Refer to the [guide](https://github.com/ogdf/ogdf/blob/master/doc/build.md) to install.

## Build binaries

To build guided-mutation code:
```bash
cd guided_mutation
make build CXXFLAGS='-O3 -march=native'
```

The binary "dpt_planarizer" will be in the bin folder

To build the binaries for other heuristics:
```bash
cd heuristics
bash build_script.sh
```
The binaries will be in the bin folder

## Make test graphs

```bash
cd make_graphs
bash build_script.sh
bash make_sample_graphs.sh
```

This will produce a graph in `make_graphs/graphs`

## Run

Assuming that you have ran all previous codes,

```bash
bash run_mps_algorithm.sh
```

Example output for particular graph:
```
guided mutation
graphn100e4s42i0, 59, 0.15722
cactus
graphn100e4s42i0, 67, 0.002273
fast
graphn100e4s42i0, 71, 0.003005
bm
graphn100e4s42i0, 79, 0.002023
```

## License

OGDF is licensed under the GNU General Public License (GPL).

Deferred Planarity Test implementation is from https://code.google.com/archive/p/planarity-algorithms/. The original code is licensed under the "New BSD License".
