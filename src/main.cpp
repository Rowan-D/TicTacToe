#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <cmath>
#include <cstdio>
#include <string_view>
#include <vector>
#include <iostream>

struct Camera2D {
    glm::vec2 center{0.0f, 0.0f};
    float zoom = 48.0f; // pixels per world unit

    [[nodiscard]] glm::vec2 screen_to_world(glm::vec2 p, glm::ivec2 size) const {
        const glm::vec2 half = glm::vec2(size) * 0.5f;
        glm::vec2 y_up{p.x - half.x, half.y - p.y};
        return center + y_up / zoom;
    }

    [[nodiscard]] glm::mat4 world_to_clip(glm::ivec2 size) const {
        const float half_w = static_cast<float>(size.x) / (2.0f * zoom);
        const float half_h = static_cast<float>(size.y) / (2.0f * zoom);
        return glm::ortho(center.x - half_w, center.x + half_w,
                          center.y - half_h, center.y + half_h,
                         -1.0f, 1.0f);
    }

    void zoom_about_screen_point(float factor, glm::vec2 screen, glm::ivec2 size) {
        factor = glm::clamp(factor, 0.1f, 10.0f);
        const glm::vec2 before = screen_to_world(screen, size);
        zoom = glm::clamp(zoom * factor, 5.0f, 600.0f);
        const glm::vec2 after = screen_to_world(screen, size);
        center += before - after;
    }

    void reset() {
        center = {0.0f, 0.0f};
        zoom = 48.0f;
    }
};

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
};

static GLuint compile_shader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048]{};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::fprintf(stderr, "shader compile failed: %s\n", log);
    }
    return shader;
}

static GLuint make_program() {
    const char* vs = R"(
        #version 330 core
        layout(location = 0) in vec2 a_pos;
        layout(location = 1) in vec3 a_color;
        uniform mat4 u_mvp;
        out vec3 v_color;
        void main() {
            v_color = a_color;
            gl_Position = u_mvp * vec4(a_pos, 0.0, 1.0);
        }
    )";
    const char* fs = R"(
        #version 330 core
        in vec3 v_color;
        out vec4 frag_color;
        void main() {
            frag_color = vec4(v_color, 1.0);
        }
    )";

    GLuint v = compile_shader(GL_VERTEX_SHADER, vs);
    GLuint f = compile_shader(GL_FRAGMENT_SHADER, fs);
    GLuint p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);
    glDeleteShader(v);
    glDeleteShader(f);

    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048]{};
        glGetProgramInfoLog(p, sizeof(log), nullptr, log);
        std::fprintf(stderr, "program link failed: %s\n", log);
    }
    return p;
}

struct LineBatch {
    GLuint vao = 0;
    GLuint vbo = 0;
    std::vector<Vertex> vertices;

    void init() {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 1024 * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                              sizeof(Vertex), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                              sizeof(Vertex), reinterpret_cast<void*>(sizeof(glm::vec2)));
    }

    void line(glm::vec2 a, glm::vec2 b, glm::vec3 c) {
        vertices.push_back({a, c});
        vertices.push_back({b, c});
    }

    void rect(glm::vec2 min, glm::vec2 max, glm::vec3 c) {
        line({min.x, min.y}, {max.x, min.y}, c);
        line({max.x, min.y}, {max.x, max.y}, c);
        line({max.x, max.y}, {min.x, max.y}, c);
        line({min.x, max.y}, {min.x, min.y}, c);
    }

    void circle(glm::vec2 center, float radius, glm::vec3 c) {
        constexpr int steps = 48;
        for (int i = 0; i < steps; ++i) {
            float a0 = (static_cast<float>(i) / steps) * 6.28318530718f;
            float a1 = (static_cast<float>(i + 1) / steps) * 6.28318530718f;
            line(center + radius * glm::vec2(std::cos(a0), std::sin(a0)),
                 center + radius * glm::vec2(std::cos(a1), std::sin(a1)), c);
        }
    }

    void x_mark(glm::ivec2 cell, glm::vec3 c) {
        const glm::vec2 p = glm::vec2(cell);
        const float pad = 0.18f;
        line(p + glm::vec2(pad, pad), p + glm::vec2(1.0f - pad), c);
        line(p + glm::vec2(pad, 1.0f - pad), p + glm::vec2(1.0f - pad, pad), c);
    }

    void o_mark(glm::ivec2 cell, glm::vec3 c) {
        circle(glm::vec2(cell) + glm::vec2(0.5f), 0.33f, c);
    }

    void draw(GLuint program, const glm::mat4& mvp) {
        glUseProgram(program);
        glUniformMatrix4fv(glGetUniformLocation(program, "u_mvp"),
                           1, GL_FALSE, &mvp[0][0]);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                     vertices.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertices.size()));
        vertices.clear();
    }

    void shutdown() {
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }
};

struct TouchState {
    bool active = false;
    SDL_FingerID a{};
    SDL_FingerID b{};
    glm::vec2 pa{};
    glm::vec2 pb{};

    [[nodiscard]] static glm::vec2 to_pixels(const SDL_TouchFingerEvent& e,
                                             glm::ivec2 size) {
        return {e.x * static_cast<float>(size.x),
                e.y * static_cast<float>(size.y)};
    }

    void finger_down(const SDL_TouchFingerEvent& e, glm::ivec2 size) {
        if (!active) {
            a = e.fingerID;
            pa = to_pixels(e, size);
            b = 0;
            pb = pa;
            active = true;
            return;
        }
        if (b == 0 && e.fingerID != a) {
            b = e.fingerID;
            pb = to_pixels(e, size);
        }
    }

    void finger_up(const SDL_TouchFingerEvent& e) {
        if (e.fingerID == a || e.fingerID == b) {
            active = false;
            b = 0;
        }
    }

    void finger_motion(const SDL_TouchFingerEvent& e,
                       glm::ivec2 size,
                       Camera2D& cam) {
        if (!active) return;

        glm::vec2 p = to_pixels(e, size);
        if (b == 0 && e.fingerID == a) {
            const glm::vec2 before = cam.screen_to_world(pa, size);
            const glm::vec2 after = cam.screen_to_world(p, size);
            cam.center += before - after;
            pa = p;
            return;
        }

        glm::vec2 old_a = pa;
        glm::vec2 old_b = pb;
        if (e.fingerID == a) pa = p;
        if (e.fingerID == b) pb = p;

        const float old_d = glm::length(old_b - old_a);
        const float new_d = glm::length(pb - pa);
        if (old_d > 1.0f && new_d > 1.0f) {
            const glm::vec2 old_mid = (old_a + old_b) * 0.5f;
            const glm::vec2 new_mid = (pa + pb) * 0.5f;

            const glm::vec2 before = cam.screen_to_world(old_mid, size);
            cam.zoom_about_screen_point(new_d / old_d, new_mid, size);
            const glm::vec2 after = cam.screen_to_world(new_mid, size);
            cam.center += before - after;
        }
    }
};

static void emit_grid(LineBatch& batch, const Camera2D& cam, glm::ivec2 size) {
    glm::vec2 a = cam.screen_to_world({0.0f, static_cast<float>(size.y)}, size);
    glm::vec2 b = cam.screen_to_world({static_cast<float>(size.x), 0.0f}, size);

    const int min_x = static_cast<int>(std::floor(a.x)) - 1;
    const int max_x = static_cast<int>(std::ceil(b.x)) + 1;
    const int min_y = static_cast<int>(std::floor(a.y)) - 1;
    const int max_y = static_cast<int>(std::ceil(b.y)) + 1;

    const glm::vec3 grid{0.23f, 0.23f, 0.25f};
    const glm::vec3 axis{0.45f, 0.45f, 0.50f};

    for (int x = min_x; x <= max_x; ++x) {
        batch.line({static_cast<float>(x), static_cast<float>(min_y)},
                   {static_cast<float>(x), static_cast<float>(max_y)},
                   x == 0 ? axis : grid);
    }
    for (int y = min_y; y <= max_y; ++y) {
        batch.line({static_cast<float>(min_x), static_cast<float>(y)},
                   {static_cast<float>(max_x), static_cast<float>(y)},
                   y == 0 ? axis : grid);
    }
}

static void emit_test_marks(LineBatch& batch) {
    for (int y = -20; y <= 20; ++y) {
        for (int x = -20; x <= 20; ++x) {
            if ((x * 13 + y * 7) % 17 == 0) batch.x_mark({x, y}, {0.90f, 0.75f, 0.25f});
            if ((x * 5 - y * 11) % 23 == 0) batch.o_mark({x, y}, {0.25f, 0.65f, 0.95f});
        }
    }

    batch.rect({-3.0f, -2.0f}, {-1.0f, -0.5f}, {0.45f, 0.9f, 0.45f});
    batch.circle({3.0f, 1.0f}, 1.0f, {0.9f, 0.45f, 0.45f});
}

int main(int, char**) {
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "wayland,x11,windows,cocoa");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Window* win = SDL_CreateWindow("Infinite Tic Tac Toe Viewer",
                                       1280, 720,
                                       SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!win) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GLContext ctx = SDL_GL_CreateContext(win);
    if (!ctx) {
        std::fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        return 1;
    }

    if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress))) {
        std::cerr << "failed to load OpenGL\n";
        return 1;
    }

    SDL_GL_SetSwapInterval(1);

    GLuint program = make_program();
    LineBatch lines;
    lines.init();

    Camera2D cam;
    TouchState touch;
    bool running = true;
    bool fullscreen = false;
    bool middle_drag = false;
    glm::vec2 last_mouse{};

    while (running) {
        SDL_Event e{};
        while (SDL_PollEvent(&e)) {
            int w = 1;
            int h = 1;
            SDL_GetWindowSizeInPixels(win, &w, &h);
            glm::ivec2 size{w, h};

            switch (e.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;

            case SDL_EVENT_KEY_DOWN:
                if (e.key.key == SDLK_ESCAPE) running = false;
                if (e.key.key == SDLK_F11) {
                    fullscreen = !fullscreen;
                    SDL_SetWindowFullscreen(win, fullscreen);
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (e.button.button == SDL_BUTTON_MIDDLE) {
                    const bool ctrl = (SDL_GetModState() & SDL_KMOD_CTRL) != 0;
                    if (ctrl) {
                        cam.reset();
                    } else {
                        middle_drag = true;
                        last_mouse = {e.button.x, e.button.y};
                    }
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (e.button.button == SDL_BUTTON_MIDDLE) {
                    middle_drag = false;
                }
                break;

            case SDL_EVENT_MOUSE_MOTION:
                if (middle_drag) {
                    glm::vec2 p{e.motion.x, e.motion.y};
                    const glm::vec2 before = cam.screen_to_world(last_mouse, size);
                    const glm::vec2 after = cam.screen_to_world(p, size);
                    cam.center += before - after;
                    last_mouse = p;
                }
                break;

            case SDL_EVENT_MOUSE_WHEEL: {
                const bool ctrl = (SDL_GetModState() & SDL_KMOD_CTRL) != 0;
                if (ctrl) {
                    const float factor = std::pow(1.12f, e.wheel.y);
                    cam.zoom_about_screen_point(factor,
                        {static_cast<float>(size.x) * 0.5f,
                         static_cast<float>(size.y) * 0.5f},
                        size);
                }
                break;
            }

            case SDL_EVENT_FINGER_DOWN:
                touch.finger_down(e.tfinger, size);
                break;

            case SDL_EVENT_FINGER_UP:
                touch.finger_up(e.tfinger);
                break;

            case SDL_EVENT_FINGER_MOTION:
                touch.finger_motion(e.tfinger, size, cam);
                break;

            default:
                break;
            }
        }

        int w = 1;
        int h = 1;
        SDL_GetWindowSizeInPixels(win, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.08f, 0.08f, 0.095f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        emit_grid(lines, cam, {w, h});
        emit_test_marks(lines);
        lines.draw(program, cam.world_to_clip({w, h}));

        SDL_GL_SwapWindow(win);
    }

    lines.shutdown();
    glDeleteProgram(program);
    SDL_GL_DestroyContext(ctx);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
