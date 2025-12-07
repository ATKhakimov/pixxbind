.PHONY: build cpp clean

build:
	python -m build

cpp:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build --config Release

clean:
	cmake -E remove_directory build
	cmake -E remove_directory _skbuild
	cmake -E remove_directory dist
