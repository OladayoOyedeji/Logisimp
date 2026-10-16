@page doxygen_guide Writing Doxygen comments

Full Doxygen command list can be found in the [Doxygen command reference](https://www.doxygen.nl/manual/commands.html).

## File header

Start every `.h` and `.cpp` file with:

```cpp
/**
 * @file WireNode.h
 * @brief Junction or endpoint in a circuit's wire graph.
 * @author Cole Schwandt
 */
```

## Class

Place the comment above the class declaration:

```cpp
/**
 * @brief Stores runtime values for one circuit instance.
 */
class CircuitState;
```

## Function

Explain arguments, results, and ownership when relevant:

```cpp
/**
 * @brief Creates an unattached wire node.
 * @param circuit Circuit that will own the node.
 * @param width Bit width; zero means unconstrained.
 * @param label Descriptive text; matching labels do not connect nodes.
 * @return Borrowed reference, valid until removal or circuit destruction.
 */
WireNode & create_wire_node(Circuit & circuit, int width = 0, const std::string & label = "");
```

Parameter names must match the signature. Omit `@return` for constructors and `void` functions. Add `@throws`, `@note`, or `@warning` when needed.

## Member

Use `///<` after a member:

```cpp
int width_; ///< Bit width; zero means unconstrained.
```

## Generate documentation

From the repository root:

```sh
make d
```

To view:
```
make do
```