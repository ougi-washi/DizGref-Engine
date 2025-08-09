## Syphax Engine - 𐤒𐤐𐤎
Simple, fast, and lightweight 2D/3D engine in C.

### Features
* Handle shaders and their uniforms
* (WIP) Handle SDF objects and operations

### Building
CMake 3.22 is required.
Make sure to:
git clone with `--recurse-submodules` or run `git submodule update --init --recursive`

Simply run:
```bash
build.sh
```
or manually:
```bash
mkdir build
cd build
cmake ..
make
```

### Usage

Examples are located in the `examples` directory.
To run an example, simply run:
```bash
./bin/example_name
```

### Library Dependencies
Locally under `/lib/`
* [GLFW](https://github.com/glfw/glfw)
* [STB](https://github.com/nothings/stb)

TODO: move to `/lib/`
* [portaudio](https://github.com/PortAudio/portaudio)

### License
MIT License
