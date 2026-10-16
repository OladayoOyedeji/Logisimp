# include

Public simulator interfaces live here. `Project.h` is the editing entry point;
`Circuit.h` owns the blueprint's components, graph nodes/edges, and derived
nets. `Port.h` distinguishes a port's authoritative `WireNode` attachment from
its current logical `Net`. `WireNode.h`, `WireEdge.h`, and `Net.h` document graph
inspection and the lifetime of borrowed objects.

`CircuitState.h` and `Simulator.h` supply runtime values and event processing.
Complete structural edits before constructing runtime states. Before editing
an affected blueprint again, clear simulator queues and destroy its runtime
hierarchies, including parent placements. `Test.h` is a disabled historical
harness; its supported replacement snippets use the current graph API.
