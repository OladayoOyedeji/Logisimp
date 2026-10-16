# Logisimp

Logisimp separates reusable circuit blueprints from independent runtime states.
Build connectivity through `Project`: create `WireNode` junctions, join them
with undirected `WireEdge` segments, and attach component ports to nodes.
`Circuit` derives logical `Net` objects; `CircuitState` stores their values,
and `Simulator` propagates changes through its event queue.

Run `make run` for primitive truth tables, a graph branch, and nested full-adder
examples. The executable checks their expected Boolean results in every build. Use CMake's
CTest targets to run the maintained automated tests.

See [the local documentation guide](docs/README.md) to generate and view the
Doxygen Awesome HTML website. From the repository root, run `make docs` to
generate it or `make docs-open` to generate it when needed and open it on Linux.
The homepage is `build/docs/html/index.html`; `make docs-clean` removes only
the generated documentation.
