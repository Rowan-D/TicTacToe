#include "Renderer.hpp"
#include <glm/gtc/constants.hpp>

namespace ttt {
    Renderer::Renderer() : m_renderer(nullptr), m_color(0.0f, 0.0f, 0.0f, 1.0f) {}
    Renderer::~Renderer() {
        Shutdown();
    }

    bool Renderer::Init(SDL_Window* window) {
        m_renderer = SDL_CreateRenderer(window, nullptr);
        if (!m_renderer) {
            SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
            return false;
        }

        SDL_SetRenderVSync(m_renderer, 1);
        return true;
    }

    void Renderer::Shutdown() {
        if (m_renderer) {
            SDL_DestroyRenderer(m_renderer);
            m_renderer = nullptr;
        }
    }

    void Renderer::Render() {
        SDL_RenderPresent(m_renderer);
    }

    void Renderer::setColor(glm::vec4 color) {
        m_color = color;
        SDL_SetRenderDrawColor(
            m_renderer,
            static_cast<Uint8>(glm::clamp(color.r * 255.0f, 0.0f, 255.0f)),
            static_cast<Uint8>(glm::clamp(color.g * 255.0f, 0.0f, 255.0f)),
            static_cast<Uint8>(glm::clamp(color.b * 255.0f, 0.0f, 255.0f)),
            static_cast<Uint8>(glm::clamp(color.a * 255.0f, 0.0f, 255.0f))
        );
    }

    void Renderer::clear() {
        SDL_RenderClear(m_renderer);
    }

    void Renderer::drawScreenLine(const glm::vec2 a, const glm::vec2 b) {
        SDL_RenderLine(m_renderer, a.x, a.y, b.x, b.y);
    }

    void Renderer::drawWorldLine(
        const Camera& cam,
        const glm::vec2 size,
        const glm::vec2 a,
        const glm::vec2 b
    ) {
        const glm::vec2 sa = worldToScreenPoint(a, cam, size);
        const glm::vec2 sb = worldToScreenPoint(b, cam, size);
        SDL_RenderLine(m_renderer, sa.x, sa.y, sb.x, sb.y);
    }
    

    void Renderer::drawScreenThickLine(const glm::vec2 a, const glm::vec2 b, const float thickness) {
        const glm::vec2 dir = b - a;
        const float len = glm::length(dir);

        if (len <= 0.0001f) {
            return;
        }

        const glm::vec2 n = glm::vec2(-dir.y, dir.x) / len;
        const glm::vec2 off = n * (thickness * 0.5f);

        SDL_Vertex verts[4] = {
            {{a.x + off.x, a.y + off.y}, {0, 0, 0, 255}, {0, 0}},
            {{a.x - off.x, a.y - off.y}, {0, 0, 0, 255}, {0, 0}},
            {{b.x - off.x, b.y - off.y}, {0, 0, 0, 255}, {0, 0}},
            {{b.x + off.x, b.y + off.y}, {0, 0, 0, 255}, {0, 0}},
        };

        int indices[6] = {0, 1, 2, 0, 2, 3};

        SDL_RenderGeometry(m_renderer, nullptr, verts, 4, indices, 6);
    }

    void Renderer::drawScreenRing(
        const glm::vec2 center,
        const float radius,
        const float thickness,
        const int segments
    ) {
        const float inner = radius - thickness * 0.5f;
        const float outer = radius + thickness * 0.5f;

        std::vector<SDL_Vertex> verts;
        std::vector<int> indices;

        verts.reserve(static_cast<size_t>(segments) * 2);
        indices.reserve(static_cast<size_t>(segments) * 6);

        for (int i = 0; i < segments; ++i) {
            const float a = static_cast<float>(i)
                * glm::two_pi<float>()
                / static_cast<float>(segments);

            const glm::vec2 dir{std::cos(a), std::sin(a)};

            verts.push_back({
                {center.x + dir.x * outer, center.y + dir.y * outer},
                {
        static_cast<Uint8>(glm::clamp(m_color.r * 255.0f, 0.0f, 255.0f)),
        static_cast<Uint8>(glm::clamp(m_color.g * 255.0f, 0.0f, 255.0f)),
        static_cast<Uint8>(glm::clamp(m_color.b * 255.0f, 0.0f, 255.0f)),
        static_cast<Uint8>(glm::clamp(m_color.a * 255.0f, 0.0f, 255.0f))
    },
                {0.0f, 0.0f}
            });

            verts.push_back({
                {center.x + dir.x * inner, center.y + dir.y * inner},
                {
        static_cast<Uint8>(glm::clamp(m_color.r * 255.0f, 0.0f, 255.0f)),
        static_cast<Uint8>(glm::clamp(m_color.g * 255.0f, 0.0f, 255.0f)),
        static_cast<Uint8>(glm::clamp(m_color.b * 255.0f, 0.0f, 255.0f)),
        static_cast<Uint8>(glm::clamp(m_color.a * 255.0f, 0.0f, 255.0f))
    },
                {0.0f, 0.0f}
            });
        }

        for (int i = 0; i < segments; ++i) {
            const int next = (i + 1) % segments;

            const int outer0 = i * 2;
            const int inner0 = i * 2 + 1;
            const int outer1 = next * 2;
            const int inner1 = next * 2 + 1;

            indices.push_back(outer0);
            indices.push_back(inner0);
            indices.push_back(inner1);

            indices.push_back(outer0);
            indices.push_back(inner1);
            indices.push_back(outer1);
        }

        SDL_RenderGeometry(
            m_renderer,
            nullptr,
            verts.data(),
            static_cast<int>(verts.size()),
            indices.data(),
            static_cast<int>(indices.size())
        );
}
void Renderer::drawWorldRing(
    const Camera& cam,
    const glm::vec2 size,
    const glm::vec2 center,
    const float radiusWorld,
    const float thicknessPixels,
    const int segments
) {
    const glm::vec2 screenCenter = worldToScreenPoint(center, cam, size);
    const float screenRadius = radiusWorld * cam.zoom;

    drawScreenRing(screenCenter, screenRadius, thicknessPixels, segments);
}
}