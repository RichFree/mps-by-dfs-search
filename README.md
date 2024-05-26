# Maximum Planar Subgraph approximation by searching DFS Trees

## Build binaries

To build guided-mutation code:
```bash
cd guided_mutation
make build CXXFLAGS='-O3 -march=native'
```
The binary "dpt_planarizer" will be in the bin folder

To build the binaries for other heuristics:
```bash
cd into heuristics
bash build_script.sh
```
The binaries will be in the bin folder

## Getting the data

The datasets have already been pre-processed and can be obtained from the following:

## Running the benchmark

In the 'benchmark_scripts' folder, there is the `benchmark.sh` shell script.

Run with the following:
```bash
bash benchmark.sh /path/to/datasets
```
Note: make sure that the file-path does not terminate with a slash

If your machine has a large number of threads, you can modify the `benchmark.sh`
script `THREADS` variable to match the number of threads that you require.


## License

OGDF is licensed under the GNU General Public License (GPL).

Deferred Planarity Test implementation is from https://code.google.com/archive/p/planarity-algorithms/. The original code is licensed under the "New BSD License".
