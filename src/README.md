# Source Codes

`main.cpp` demonstrates explicit graph edges and branching, all primitive gate
truth tables, and a nested full adder built from reusable half adders. Output
values are checked after queued propagation settles.

`Circuit.cpp` owns graph editing, validation, and connected-component net
rebuilding. `Project.cpp` validates ownership and coordinates reusable circuit
interfaces. `CircuitState.cpp` owns per-instance net values; `Simulator.cpp`
schedules logical components rather than individual graph edges.

`Test.cpp` retains a disabled earlier interactive harness for historical
context. It is not the maintained test suite; use the CTest targets instead.
