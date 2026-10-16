# Simulator design

## Current graph architecture

The maintained implementation uses an undirected graph for electrical
connectivity and derived logical nets for simulation. See the
[current architecture and construction guide](../../docs/index.md) and
[local documentation instructions](../../docs/README.md) for the current API.

```mermaid
flowchart TD
    Project -->|owns and edits| Circuit
    Circuit -->|owns| Components
    Components -->|own| Ports
    Circuit -->|owns| WireNodes
    Circuit -->|owns| WireEdges
    WireEdges -->|join two endpoints| WireNodes
    Ports -->|node: authoritative attachment| WireNodes
    Circuit -->|derives from connected graph components| Nets
    Ports -->|net: derived membership| Nets
    CircuitState -->|owns independent Net values| NetValues[LogicVector values keyed by Net]
    CircuitState -->|owns component and child-instance state| RuntimeState
    Simulator -->|FIFO events| Events["(CircuitState, Component) pairs"]
    Events -->|evaluate components and propagate changed Net values| CircuitState
```

`WireNode` and `WireEdge` contain connectivity, not runtime values. `Net`
contains one network's driver, listeners, and compatible width; `CircuitState`
stores each runtime instance's values independently. The simulator schedules
components listening to a changed net; it does not schedule individual edges.
Complete structural edits before creating runtime states. Clear simulator
queues and destroy affected runtime hierarchies before editing again; derived
Net references become invalid after successful connectivity edits.

## Historical design

The original sketch below is preserved verbatim as design history. It predates
both graph-based wiring and the separation of runtime values into
`CircuitState`; its `Wire`, stored pin values, component-only queue, and older
function signatures do not describe the current API. Some sketched components
also remain unimplemented.

The original [SVG export](<LogicVector Input-2026-09-14-145819.svg>) is likewise
an old reference drawing, preserved unchanged rather than published as current
Doxygen architecture documentation.

```mermaid
classDiagram

class LogicValue {
    <<enumeration>>
    LOW
    HIGH
    UNKNOWN
}

class LogicVector {
    -vector~ LogicValue ~ bits_
}

class PortDirection {
    <<enumeration>>
    INPUT
    OUTPUT
}

class PinType {
    <<enumeration>>
    INPUT_PIN
    OUTPUT_PIN
}

class Port {
    -int id_
    -int width_
    -PortDirection direction_
    -Component * owner_
    -Wire * wire_
    -string label_
}

class Component {
    <<abstract>>
    -int id_
    -string label_
    -vector~ Port * ~ inputs_
    -vector~ Port * ~ outputs_
    +evaluate(Simulator & simulator)*
}

class Gate {
    <<abstract>>
    +Gate(int id, int input_count)
    +output() Port *
}

class AndGate {
    +evaluate(Simulator & simulator)
}

class OrGate {
    +evaluate(Simulator & simulator)
}

class NotGate {
    +evaluate(Simulator & simulator)
}

class NandGate {
    +evaluate(Simulator & simulator)
}

class NorGate {
    +evaluate(Simulator & simulator)
}

class XorGate {
    +evaluate(Simulator & simulator)
}

class XnorGate {
    +evaluate(Simulator & simulator)
}

class Pin {
    -PinType type_
    -LogicVector value_
    +set(LogicVector value, Simulator & simulator)
    +toggle(Simulator & simulator)
    +evaluate(Simulator & simulator)
    +reset()
}

class HalfAdder {
    +evaluate(Simulator & simulator)
}

class FullAdder {
    +evaluate(Simulator & simulator)
}

class Mux {
    +evaluate(Simulator & simulator)
}

class Register {
    +evaluate(Simulator & simulator)
}

class RAM {
    +evaluate(Simulator & simulator)
}

class ROM {
    +evaluate(Simulator & simulator)
}

class Wire {
    -int id_
    -string label_
    -LogicVector value_
    -Port * driver_
    -vector~ Port * ~ listeners_
}

class Simulator {
    -queue~ Component * ~ work_queue_
    -unordered_set~ Component * ~ queued_components_
}

class Circuit {
    -vector~ Component * ~ components_
    -vector~ Wire * ~ wires_
}

Component <|-- Gate
Component <|-- Pin
Component <|-- HalfAdder
Component <|-- FullAdder
Component <|-- Mux
Component <|-- Register
Component <|-- RAM
Component <|-- ROM

Gate <|-- AndGate
Gate <|-- OrGate
Gate <|-- NotGate
Gate <|-- NandGate
Gate <|-- NorGate
Gate <|-- XorGate
Gate <|-- XnorGate

Circuit *-- Component : components_
Circuit *-- Wire : wires_

Component *-- Port : inputs_
Component *-- Port : outputs_

Port --> Component : owner_
Port --> Wire : wire_
Port --> PortDirection : direction_

Wire *-- LogicVector : value_
Wire --> Port : driver_
Wire --> Port : listeners_

LogicVector *-- LogicValue : bits_

Pin *-- PinType : type_
Pin *-- LogicVector : value_

Simulator --> Component : work_queue_
Simulator --> Component : queued_components_

Component ..> Simulator : evaluate()
Pin ..> Simulator : set() / toggle()
```
