#include "TicTacToe.hpp"
#include "Renderer.hpp"

#include <glm/glm.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

namespace ttt {
    static void drawGrid(Renderer& renderer, const Camera& cam, const glm::vec2 size) {
        const glm::vec2 min = screenToWorldPoint(glm::vec2(0.0f), cam, size);
        const glm::vec2 max = screenToWorldPoint(glm::vec2(size), cam, size);

        renderer.setColor(glm::vec4(0.863f, 0.863f, 0.863f, 1.0f));

        const int x0 = static_cast<int>(std::floor(min.x)) - 1;
        const int x1 = static_cast<int>(std::ceil(max.x)) + 1;
        const int y0 = static_cast<int>(std::floor(min.y)) - 1;
        const int y1 = static_cast<int>(std::ceil(max.y)) + 1;

        for (int x = x0; x <= x1; ++x) {
            renderer.drawWorldLine(cam, size, { static_cast<float>(x), min.y - 1.0f }, { static_cast<float>(x), max.y + 1.0f });
        }

        for (int y = y0; y <= y1; ++y) {
            renderer.drawWorldLine(cam, size, {min.x - 1.0f, static_cast<float>(y)}, {max.x + 1.0f, static_cast<float>(y)});
        }
    }

    static float scaledStrokeThickness(const Camera& cam) {
        const float base = 8.0f;
        const float zoomScale = std::sqrt(cam.zoom / 90.0f);

        return std::clamp(base * zoomScale, 3.0f, 18.0f);
    }

    static void drawX(
        Renderer& renderer,
        const Camera& cam,
        const glm::vec2 size,
        const glm::vec2 cell
    ) {
        renderer.setColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

        const float pad = 0.18f;
        const float thick = scaledStrokeThickness(cam);
        renderer.drawWorldThickLine(
            cam,
            size,
            {cell.x + pad, cell.y + pad},
            {cell.x + 1.0f - pad, cell.y + 1.0f - pad},
            thick
        );

        renderer.drawWorldThickLine(
            cam,
            size,
            {cell.x + 1.0f - pad, cell.y + pad},
            {cell.x + pad, cell.y + 1.0f - pad},
            thick
        );
    }

    static void drawO(
        Renderer& renderer,
        const Camera& cam,
        const glm::vec2 size,
        const glm::vec2 cell
    ) {
        renderer.setColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        const float thick = scaledStrokeThickness(cam);

        renderer.drawWorldRing(
            cam,
            size,
            {cell.x + 0.5f, cell.y + 0.5f},
            0.34f,
            thick,
            96
        );
    }
}

int main(int, char**) {
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "wayland,x11");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Tic Tac Toe", 1280, 720, SDL_WINDOW_RESIZABLE);

    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        return 1;
    }

    ttt::Renderer renderer;
    if (!renderer.Init(window)) {
        SDL_Log("Failed to initialize renderer");
        return 1;
    }

    ttt::Camera cam;
    //FpsCounter fps;

    bool running = true;
    bool fullscreen = false;
    bool dragging = false;

    glm::vec2 dragWorld(0.0f);
    glm::vec2 mouse(0.0f);

    ttt::TicTacToe ticTacToe;

    while (running) {
        SDL_Event e{};

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                running = false;
            }

            if (e.type == SDL_EVENT_KEY_DOWN) {
                if (e.key.key == SDLK_F11) {
                    fullscreen = !fullscreen;
                    SDL_SetWindowFullscreen(window, fullscreen);
                }

                if (e.key.key == SDLK_PERIOD) {
                    cam = ttt::Camera{};
                }
            }

            if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                if (e.button.button == SDL_BUTTON_LEFT ||
                    e.button.button == SDL_BUTTON_MIDDLE) {
                    glm::ivec2 size;
                    SDL_GetWindowSize(window, &size.x, &size.y);

                    dragging = true;
                    mouse = {e.button.x, e.button.y};
                    dragWorld = screenToWorldPoint(mouse, cam, size);

                    ticTacToe.setCell(
                        glm::ivec2(
                            static_cast<int>(std::floor(dragWorld.x)),
                            static_cast<int>(std::floor(dragWorld.y))
                        ),
                        ttt::CellState::X
                    );
                }
            }

            if (e.type == SDL_EVENT_MOUSE_BUTTON_UP) {
                if (e.button.button == SDL_BUTTON_LEFT ||
                    e.button.button == SDL_BUTTON_MIDDLE) {
                    dragging = false;
                }
            }

            if (e.type == SDL_EVENT_MOUSE_MOTION) {
                mouse = {e.motion.x, e.motion.y};
            }

            if (e.type == SDL_EVENT_MOUSE_WHEEL) {
                glm::ivec2 size;
                SDL_GetWindowSize(window, &size.x, &size.y);

                const glm::vec2 cursor{e.wheel.mouse_x, e.wheel.mouse_y};
                const glm::vec2 before = screenToWorldPoint(cursor, cam, size);

                const float factor = std::pow(1.15f, static_cast<float>(e.wheel.y));
                cam.zoom = std::clamp(cam.zoom * factor, 8.0f, 600.0f);

                const glm::vec2 after = screenToWorldPoint(cursor, cam, size);

                cam.center.x += before.x - after.x;
                cam.center.y += before.y - after.y;
            }
        }

        //fps.update();

        glm::ivec2 size{0, 0};
        SDL_GetWindowSize(window, &size.x, &size.y);

        if (dragging) {
            float mx = 0.0f;
            float my = 0.0f;
            SDL_GetMouseState(&mx, &my);

            mouse = {mx, my};

            cam.center = dragWorld - (mouse - glm::vec2(size) * 0.5f) / cam.zoom;
        }

        renderer.setColor(glm::vec4(1.0f));
        renderer.clear();

        drawGrid(renderer, cam, size);

        for (int i = 0; i < ticTacToe.m_wins.size(); i += 2) {
            glm::vec2 win0 = ticTacToe.m_wins[i];
            glm::vec2 win1 = ticTacToe.m_wins[i + 1];
            renderer.setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
            const float thick = scaledStrokeThickness(cam) * 1.5f;

            renderer.drawWorldThickLine(
                cam,
                size,
                {win0.x + 0.5f, win0.y + 0.5f},
                {win1.x + 0.5f, win1.y + 0.5f},
                thick
            );
        }
        for (const auto& [cell, state] : ticTacToe.m_cells) {
            if (state == ttt::CellState::X) {
                drawX(renderer, cam, size, cell);
            } else if (state == ttt::CellState::O) {
                drawO(renderer, cam, size, cell);
            }
        }

        renderer.setColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

        //char text[128]{};
        //std::snprintf(text, sizeof(text), "FPS: %.1f  zoom: %.1f", fps.fps, cam.zoom);

        //SDL_RenderDebugText(renderer, 10.0f, 10.0f, text);

        renderer.Render();
    }

    renderer.Shutdown();
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
