# Optional diagram targets shared by the core and standalone GUI projects.
set(LOGISIMP_UML_RUNNER "${CMAKE_CURRENT_LIST_DIR}/GenerateUml.cmake")

function(logisimp_add_uml_targets config)
    set(CLANG_UML_EXECUTABLE "" CACHE FILEPATH "Optional path to clang-uml")
    set(PLANTUML_EXECUTABLE "" CACHE FILEPATH "Optional path to the PlantUML launcher")
    set(PLANTUML_JAR "" CACHE FILEPATH "Optional PlantUML jar, used instead of its launcher")
    get_filename_component(uml_root "${config}" DIRECTORY)

    # Tool discovery happens only when these explicit targets are requested.
    add_custom_target(uml
        COMMAND "${CMAKE_COMMAND}"
            "-DUML_CONFIG=${config}"
            "-DUML_DATABASE_DIR=${CMAKE_BINARY_DIR}"
            "-DUML_OUTPUT_DIR=${uml_root}/docs/uml"
            "-DUML_ROOT=${uml_root}"
            "-DCLANG_UML_EXECUTABLE=${CLANG_UML_EXECUTABLE}"
            -DUML_MODE=source
            -P "${LOGISIMP_UML_RUNNER}"
        WORKING_DIRECTORY "${uml_root}"
        COMMENT "Generating PlantUML diagrams from C++ with clang-uml"
        VERBATIM
    )

    add_custom_target(uml-svg
        COMMAND "${CMAKE_COMMAND}"
            "-DUML_OUTPUT_DIR=${uml_root}/docs/uml"
            "-DUML_ROOT=${uml_root}"
            "-DUML_DIAGRAMS=${ARGN}"
            "-DPLANTUML_EXECUTABLE=${PLANTUML_EXECUTABLE}"
            "-DPLANTUML_JAR=${PLANTUML_JAR}"
            -DUML_MODE=svg
            -P "${LOGISIMP_UML_RUNNER}"
        WORKING_DIRECTORY "${uml_root}"
        COMMENT "Rendering UML diagrams as SVG"
        VERBATIM
    )
    add_dependencies(uml-svg uml)
endfunction()
