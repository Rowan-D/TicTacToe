#ifndef TIC_TAC_TOE_RENDERER_HPP
#define TIC_TAC_TOE_RENDERER_HPP

#include <glm/glm.hpp>
#include <SDL3/SDL.h>

namespace ttt {

    struct Camera {
        glm::vec2 center{0.5, 0.5};
        float zoom = 90.0;
    };

    inline glm::vec2 screenToWorldPoint(const glm::vec2 screen, const Camera& cam, const glm::vec2 size) {
        return cam.center + (screen - glm::vec2(size) * 0.5f) / cam.zoom;
    }

    inline glm::vec2 worldToScreenPoint(const glm::vec2 world, const Camera& cam, const glm::vec2 size) {
        return (world - cam.center) * cam.zoom + glm::vec2(size) * 0.5f;
    }

    class Renderer
    {
    public:
        Renderer();
        ~Renderer();

        bool Init(SDL_Window* window);
        void Shutdown();
        void Render();

        void setColor(glm::vec4 color);
        void clear();
        void drawScreenLine(const glm::vec2 a, const glm::vec2 b);
        void drawWorldLine(const Camera& cam, const glm::vec2 size, const glm::vec2 a, const glm::vec2 b);
        void drawScreenThickLine(const glm::vec2 a, const glm::vec2 b, const float thickness);
        inline void drawWorldThickLine(
            const Camera& cam,
            const glm::vec2 size,
            const glm::vec2 a,
            const glm::vec2 b,
            const float thickness
        ) {
            const glm::vec2 sa = worldToScreenPoint(a, cam, size);
            const glm::vec2 sb = worldToScreenPoint(b, cam, size);

            drawScreenThickLine(sa, sb, thickness);
        }
        void drawScreenRing(const glm::vec2 center, const float radius, const float thickness, const int segments);
        void drawWorldRing(
            const Camera& cam,
            const glm::vec2 size,
            const glm::vec2 center,
            const float radiusWorld,
            const float thicknessPixels,
            const int segments
        );

        SDL_Renderer* m_renderer;
        glm::vec4 m_color;
    };

}

#endif