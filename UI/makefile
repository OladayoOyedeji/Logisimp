.PHONY: p program e exe b build a asan g git r run c clean
.PHONY: d docs do docs-open dc docs-clean

DOXYGEN ?= doxygen

# Detect operating system
ifeq ($(OS),Windows_NT)
    PLATFORM := windows
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        PLATFORM := macos
    else
        PLATFORM := linux
    endif
endif

# Change config based off PLATFORM
ifeq ($(PLATFORM),windows)
    CMAKE_GENERATOR := -G "MinGW Makefiles"
    EXECUTABLE := ./build/Logisimp.exe
    REMOVE_BUILD := powershell -NoProfile -Command "Remove-Item -Recurse -Force -ErrorAction SilentlyContinue build"
    OPEN_FILE = cmd /c start "" "$(subst /,\,$(abspath $(1)))"
    DOCS_INPUTS := Doxyfile $(shell git ls-files include src docs)
    DATE_COMMAND := powershell -NoProfile -Command "Get-Date -Format 'yyyy-MM-dd HH:mm:ss'"
else
    CMAKE_GENERATOR :=
    EXECUTABLE := ./build/Logisimp
    REMOVE_BUILD := rm -rf build
    DOCS_INPUTS := Doxyfile include/. src/. docs/. $(shell find include src docs -mindepth 1)
    DATE_COMMAND := date '+%Y-%m-%d %H:%M:%S'

    ifeq ($(PLATFORM),macos)
        OPEN_FILE = open "$(1)"
    else
        OPEN_FILE = xdg-open "$(1)"
    endif
endif

# Documentation directories
DOCS_DIR := build/docs
DOCS_INDEX := $(DOCS_DIR)/html/index.html

# Build
p program e exe: build
	cmake --build build

b build:
	cmake -S . -B build $(CMAKE_GENERATOR)

# Debug build
a asan:
	cmake -S . -B build $(CMAKE_GENERATOR) -DCMAKE_BUILD_TYPE=Debug
	cmake --build build --config Debug

# Git
g git:
	git add -A
	git diff --cached --quiet || git commit -m "sync: $(shell $(DATE_COMMAND))"
	git push

# Run
r run: p
	$(EXECUTABLE)

# Clean
c clean:
	$(REMOVE_BUILD)

# Documentation
d docs:
	$(DOXYGEN) Doxyfile

$(DOCS_INDEX): $(DOCS_INPUTS)
	$(MAKE) docs

do docs-open: $(DOCS_INDEX)
	$(call OPEN_FILE,$(abspath $(DOCS_INDEX)))

# Clean documentation
dc docs-clean:
ifeq ($(PLATFORM),windows)
	powershell -NoProfile -Command "Remove-Item -Recurse -Force -ErrorAction SilentlyContinue '$(DOCS_DIR)'"
else
	rm -rf -- "$(DOCS_DIR)"
endif
