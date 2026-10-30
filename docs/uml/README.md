# Generated UML diagrams

These diagrams are generated from the C++ source with clang-uml. Regenerate
them from the repository root with `make uml-svg`, or:

```sh
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --target uml-svg
```

- [Full simulator reference](simulator_full.svg) · [PlantUML](simulator_full.puml)
- [Simulator architecture](simulator_architecture.svg) · [PlantUML](simulator_architecture.puml)

Open the SVG files in a browser to zoom. The full diagram includes members and
visibility; the architecture image hides members. Do not edit generated files.
See the [UML guide](../UML_GUIDE.md) for installation, separate Qt prototype
generation, customization, and limits of automatic ownership inference.
