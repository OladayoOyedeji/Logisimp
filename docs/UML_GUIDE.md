@page uml_guide Generating UML class diagrams

[clang-uml](https://github.com/bkryza/clang-uml) uses Clang to read the real C++
declarations and compiler options. This project uses it to regenerate class
diagrams as the source changes. It complements Doxygen; neither the simulator
build nor the Doxygen workflow requires the UML tools.

## Install the optional tools

Use clang-uml 0.6.2 or newer, PlantUML, Java, and Graphviz (`dot` on `PATH`).
PlantUML/Java/Graphviz are needed only for SVG rendering; clang-uml alone produces
the editable `.puml` sources. CMake 3.31 or newer and a C++17 compiler are the
existing core build requirements. Ninja or a Makefile generator must be used
to export a compilation database; Visual Studio generators do not support it.

On Ubuntu, the official clang-uml instructions provide a PPA:

```sh
sudo apt install software-properties-common ninja-build g++ graphviz default-jre plantuml
sudo add-apt-repository ppa:bkryza/clang-uml
sudo apt update
sudo apt install clang-uml
```

Install a sufficiently recent [CMake](https://cmake.org/download/) separately if
the distribution package is older than 3.31. For other Linux distributions, use
the packages, release downloads, or source installation described in the
[official clang-uml installation guide](https://clang-uml.github.io/md_docs_2installation.html).

On Windows:

1. Install CMake, [Ninja](https://ninja-build.org/), and a C++ compiler. When using
   MSVC, run CMake from its Developer PowerShell or developer command prompt.
2. Install clang-uml using the Windows installer from its
   [official releases](https://github.com/bkryza/clang-uml/releases), and ensure
   `clang-uml` is on `PATH`.
3. Install a Java runtime supported by PlantUML (Java 17 or newer is suitable),
   [Graphviz](https://graphviz.org/download/), and put `java` and `dot` on `PATH`.
4. Download the [PlantUML jar](https://plantuml.com/download). Pass its location
   with `-DPLANTUML_JAR=...`, as shown below. A `plantuml` launcher on `PATH`
   works as an alternative.

WSL with the Linux instructions is another option. Configure and generate
diagrams within the same environment: compilation databases contain compiler
and include paths specific to that environment.

## Generate the core diagrams

Run these commands from the repository root:

```sh
cmake -S . -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --target uml-svg
```

If `build/` already uses a Makefile generator, omit `-G Ninja` to reuse it.
Both CMake projects enable `CMAKE_EXPORT_COMPILE_COMMANDS` themselves; the flag
above makes the requirement explicit. No simulator compilation is required to
generate core diagrams. The source files must still parse with the compiler
options in `build/compile_commands.json`.

For a downloaded PlantUML jar on Windows, adjust the path in this example:

```powershell
cmake -S . -B build -G Ninja "-DPLANTUML_JAR=C:/path/to/plantuml.jar"
cmake --build build --target uml-svg
```

If tools are outside `PATH`, configure `CLANG_UML_EXECUTABLE` and/or
`PLANTUML_EXECUTABLE` with their executable paths. `PLANTUML_JAR` takes precedence
over the launcher. These are local CMake cache settings; no machine paths belong
in the tracked configuration.

For PlantUML sources without rendering, use `cmake --build build --target uml`.
With the existing GNU Make wrapper, `make uml` and `make uml-svg` configure the
usual `build/` directory and invoke the corresponding targets. Regeneration uses
the same commands; normal `cmake --build build` never runs either UML target.

The configuration can also be used directly:

```sh
clang-uml --config .clang-uml --validate-only
clang-uml --config .clang-uml --compile-database build --output-directory docs/uml --generator plantuml
plantuml -tsvg docs/uml/simulator_full.puml docs/uml/simulator_architecture.puml
```

Use `java -jar <downloaded-jar> -tsvg ...` instead of `plantuml` when using a jar.
For a different CMake build directory, the CMake target automatically passes its
own database directory; direct clang-uml invocation needs `--compile-database`.

## Generated files and interpretation

All outputs go to `docs/uml/`:

| Diagram | PlantUML source | Browser image |
| --- | --- | --- |
| Full core reference | `simulator_full.puml` | `simulator_full.svg` |
| Core architecture | `simulator_architecture.puml` | `simulator_architecture.svg` |
| Optional Qt prototype | `gui_prototype.puml` | `gui_prototype.svg` |

Open an SVG directly in a browser and zoom to inspect it. The full reference
includes implementation helpers, methods, fields, and `+` public, `#` protected,
and `-` private visibility. PlantUML sources can be opened in a PlantUML editor
or rerendered with the commands above. The generated core files are committed;
regenerate and review their changes alongside architectural changes.

The architecture view excludes transaction helpers and nested bookkeeping
types, and uses PlantUML's `hide members`. Its `.puml` still contains member data
for relationship inference; the SVG hides fields and methods. Header-only types
are discovered through the selected translation units' includes. Empty placeholder
headers and commented-out archived code are not C++ class definitions.

Relationships are inferred from source types: inheritance, value/container
aggregation, pointer/reference associations, and dependencies. All relationship
kinds remain enabled, including composition where clang-uml can establish it.
**Raw pointers do not express ownership.** For example, owning and borrowed
`std::vector< Component * >` fields cannot be distinguished by that type alone.
This code currently produces no automatically inferred composition arrows.
The diagrams therefore do not prove which pointer containers delete their
contents; use the ownership documentation for that detail. We do not force
global relationship hints that would mislabel borrowed pointers.

## Keep the Qt prototype separate

The current prototype is in `UI/` (the older `UI/TestUI/` layout has been moved).
Its independent `Logisimp` CMake target uses Qt 6 Widgets and OpenGLWidgets. The
core configuration never selects its source files or compilation database.

With the Qt 6 development packages installed:

```sh
cmake -S UI -B build/ui -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build/ui --target uml-svg
```

Pass `CMAKE_PREFIX_PATH` if needed to locate your Qt installation, and set
`PLANTUML_JAR` in this build's cache when using a jar. The GUI target runs Qt's
AUTOGEN step first to create headers such as `ui_mainwindow.h`. It uses
`.clang-uml-ui` and `build/ui/compile_commands.json`; outputs are named
`gui_prototype.*`, so overlapping class names never merge with the core diagrams.
GNU Make alternatives are `make uml-ui` and `make uml-ui-svg`.

If Qt development packages or a valid GUI database are unavailable, generate
only the core diagrams. No GUI diagram is committed until it can be generated
from a successfully configured Qt project.

## Customize the diagrams

Edit `.clang-uml` for core or `.clang-uml-ui` for the prototype. `glob` selects
translation units from the compilation database; `include.paths` selects project
declarations; `exclude.namespaces: [std]` prevents standard-library template
nodes. Update these paths if source directories move. New included project
classes are discovered automatically without maintaining a class whitelist.

Adjust the architecture exclusions or PlantUML `before` styling to change the
level of detail. The full view's `include_relations_also_as_members: true` retains
fields even when they also create arrows. Consult the official
[configuration reference](https://clang-uml.github.io/md_docs_2configuration__file.html),
[class diagram guide](https://clang-uml.github.io/md_docs_2class__diagrams.html),
[filters](https://clang-uml.github.io/md_docs_2diagram__filters.html), and
[relationship decorators](https://clang-uml.github.io/md_docs_2comment__decorators.html)
before changing inference settings.

Verified with clang-uml 0.6.3, PlantUML 1.2026.9, Java 21, and Graphviz 2.42.4.
Core generation and SVG rendering were tested. GUI generation could not be
tested in the implementation environment because Qt development packages were
unavailable; the separate GUI configuration passed clang-uml's validation.
