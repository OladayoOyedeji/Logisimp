# Executed only by the optional uml / uml-svg targets, never by normal builds.
if(UML_MODE STREQUAL "source")
    if(NOT EXISTS "${UML_DATABASE_DIR}/compile_commands.json")
        message(FATAL_ERROR
            "No compilation database at ${UML_DATABASE_DIR}/compile_commands.json. "
            "Configure with Ninja or Makefiles and -DCMAKE_EXPORT_COMPILE_COMMANDS=ON; "
            "Visual Studio generators do not export this database. See docs/UML_GUIDE.md.")
    endif()

    if(CLANG_UML_EXECUTABLE)
        set(uml_tool "${CLANG_UML_EXECUTABLE}")
    else()
        find_program(uml_tool NAMES clang-uml)
    endif()
    if(NOT uml_tool)
        message(FATAL_ERROR "Install clang-uml or set CLANG_UML_EXECUTABLE. See docs/UML_GUIDE.md.")
    endif()

    file(MAKE_DIRECTORY "${UML_OUTPUT_DIR}")
    execute_process(
        COMMAND "${uml_tool}" --config "${UML_CONFIG}"
            --compile-database "${UML_DATABASE_DIR}"
            --output-directory "${UML_OUTPUT_DIR}" --generator plantuml
        WORKING_DIRECTORY "${UML_ROOT}"
        RESULT_VARIABLE uml_result
    )
    if(NOT uml_result STREQUAL "0")
        message(FATAL_ERROR "clang-uml failed (${uml_result}); diagrams were not successfully regenerated.")
    endif()
elseif(UML_MODE STREQUAL "svg")
    find_program(uml_dot NAMES dot)
    if(NOT uml_dot)
        message(FATAL_ERROR "Install Graphviz and put dot on PATH to render SVG diagrams.")
    endif()

    if(PLANTUML_JAR)
        find_program(uml_java NAMES java)
        if(NOT uml_java OR NOT EXISTS "${PLANTUML_JAR}")
            message(FATAL_ERROR "PLANTUML_JAR must name an existing jar and Java must be on PATH.")
        endif()
        set(uml_renderer "${uml_java}" -jar "${PLANTUML_JAR}")
    else()
        if(PLANTUML_EXECUTABLE)
            set(uml_plantuml "${PLANTUML_EXECUTABLE}")
        else()
            find_program(uml_plantuml NAMES plantuml plantuml.bat)
        endif()
        if(NOT uml_plantuml)
            message(FATAL_ERROR "Install PlantUML or set PLANTUML_JAR. See docs/UML_GUIDE.md.")
        endif()
        set(uml_renderer "${uml_plantuml}")
    endif()

    set(uml_sources)
    foreach(diagram IN LISTS UML_DIAGRAMS)
        set(uml_source "${UML_OUTPUT_DIR}/${diagram}.puml")
        if(NOT EXISTS "${uml_source}")
            message(FATAL_ERROR "Missing ${uml_source}; generate PlantUML sources with the uml target first.")
        endif()
        list(APPEND uml_sources "${uml_source}")
    endforeach()

    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E env "GRAPHVIZ_DOT=${uml_dot}"
            ${uml_renderer} -failfast2 -tsvg -charset UTF-8 ${uml_sources}
        WORKING_DIRECTORY "${UML_ROOT}"
        RESULT_VARIABLE uml_result
    )
    if(NOT uml_result STREQUAL "0")
        message(FATAL_ERROR "PlantUML SVG rendering failed (${uml_result}).")
    endif()
else()
    message(FATAL_ERROR "Unknown UML_MODE: ${UML_MODE}")
endif()
