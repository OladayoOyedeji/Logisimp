@page doxygen_guide Writing Doxygen comments

Full command list: [Doxygen command reference](https://www.doxygen.nl/manual/commands.html)

## Common Doxygen commands

| Command | Purpose | File | Class | Function | Member variable |
| --- | --- | :---: | :---: | :---: | :---: |
| `@file` | Identifies the source or header file. | ✓ | — | — | — |
| `@brief` | Gives a short description of a class, function, or variable. | ✓ | ✓ | ✓ | ✓ |
| `@details` | Provides a longer explanation when needed. | ✓ | ✓ | ✓ | ✓ |
| `@param` | Documents a function parameter. | — | — | ✓ | — |
| `@return` | Documents the return value. | — | — | ✓ | — |
| `@throws` | Describes exceptions a function may throw. | — | — | ✓ | — |
| `@pre` | States a condition required before a function is called. | — | ✓ | ✓ | — |
| `@post` | States a condition guaranteed after successful execution. | — | — | ✓ | — |
| `@invariant` | Describes a condition that must remain true for an object. | — | ✓ | — | — |
| `@note` | Adds useful context or clarification. | ✓ | ✓ | ✓ | ✓ |
| `@warning` | Highlights potentially dangerous behavior. | ✓ | ✓ | ✓ | ✓ |
| `@see` | Points readers to related documentation. | ✓ | ✓ | ✓ | ✓ |
| `@ref` | Creates a cross-reference to a documented symbol or page. | ✓ | ✓ | ✓ | ✓ |
| `@tparam` | Documents a template parameter. | — | ✓* | ✓* | — |
| `@todo` | Records planned work. | ✓ | ✓ | ✓ | ✓ |
| `@deprecated` | Marks an obsolete API and, ideally, its replacement. | — | ✓ | ✓ | ✓ |
| `@code` / `@endcode` | Encloses a formatted code example. | ✓ | ✓ | ✓ | ✓ |
| `@copydoc` | Copies documentation from another symbol. | ✓ | ✓ | ✓ | ✓ |

## File header

Begin each `.h` and `.cpp` file with a file-level comment:

```cpp
/**
 * @file Example.h
 * @brief Describes the purpose of this file.
 * @author Joe Schmo
 */
```

## Class

Place the comment above the class declaration or definition:

```cpp
/**
 * @brief Stores a number.
 */
class Counter;
```

## Function

Describe what the function does, its parameters, and its return value:

```cpp
/**
 * @brief Adds two integers.
 * @param first First integer.
 * @param second Second integer.
 * @return Sum of the two integers.
 */
int add(int first, int second);
```

Omit `@return` for constructors and `void` functions. Add `@throws`, `@pre`, `@post`, `@note`, or `@warning` when applicable.

## Member variable

Use `///<` after a member:

```cpp
int count_; ///< Number of items currently stored; -1 means count_ hasn't bee intialized
```

Document special values, units, and ownership when relevant. Use a multiline Doxygen comment above the declaration if a single line is insufficient.

## Generate documentation

From the repository root:

```sh
make d
```

To view:

```sh
make do
```
