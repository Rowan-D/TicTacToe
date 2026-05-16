# Infinite Tic Tac Toe Viewer

SDL3 + OpenGL starter for:
- resizable window
- F11 borderless fullscreen toggle
- middle-click drag pan
- Ctrl + scroll zoom around screen center
- Ctrl + middle click reset
- one-finger pan
- two-finger pinch/pan
- infinite grid
- hardware-accelerated x/o/rect/circle line rendering

## Build

Requires SDL3, glad, glm, CMake, Ninja.

```sh
cmake -S . -B build -G Ninja
cmake --build build
./build/infinite_ttt
```

On Linux, SDL3 prefers Wayland when available. You can force it for testing:

```sh
SDL_VIDEO_DRIVER=wayland ./build/infinite_ttt
```

or fallback:

```sh
SDL_VIDEO_DRIVER=x11 ./build/infinite_ttt
```
