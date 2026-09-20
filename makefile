.PHONY: p program e exe b build g git r run c clean

p program e exe: build
	cmake --build build

b build:
	cmake -S . -B build

a asan:
	(cmake -S . -B build -DCMAKE_BUILD_TYPE=debug) \
	&& (cmake --build build --config debug)

g git:
	git add -A
	git diff --cached --quiet || git commit -m "sync: $(shell date '+%Y-%m-%d %H:%M:%S')"
	git push

r run: p
	./build/logisimp

c clean:
	rm -rf build
