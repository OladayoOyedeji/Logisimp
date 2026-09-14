p program: build
	cmake --build build

b build:
	cmake -S . -B build

.PHONY: git

g git:
	git add -A
	git diff --cached --quiet || git commit -m "sync: $(shell date '+%Y-%m-%d %H:%M:%S')"
	git push

r run:
	build/program

c clean:
	rm -rf build
