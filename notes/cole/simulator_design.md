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
