#pragma once

#include <glm/vec3.hpp>
#include <memory>
#include <string>
#include <vector>

#include "WallpaperEngine/Render/CObject.h"
#include "WallpaperEngine/Render/Text/FontAtlas.h"
#include "WallpaperEngine/Render/Wallpapers/CScene.h"

namespace WallpaperEngine::Render::Objects {
class CText final : public CObject {
public:
    CText (Wallpapers::CScene& scene, const Data::Model::Text& text);
    ~CText () override;

    void setup ();
    void render () override;

private:
    void rebuildMeshIfNeeded ();
    void ensureProgram ();
    glm::vec4 getColor4 () const;

private:
    struct Vertex {
        glm::vec3 pos;
        glm::vec2 uv;
    };

    const Data::Model::Text& m_text;
    WallpaperEngine::Render::Text::FontAtlasPtr m_atlas;
    GLuint m_program = 0;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;
    GLint m_uMvp = -1;
    GLint m_uColor = -1;
    GLint m_uAtlas = -1;

    std::vector<Vertex> m_vertices;
    std::vector<GLuint> m_indices;
    std::string m_cachedText;
    float m_cachedSize = -1.0f;
    std::string m_cachedFont;
    glm::vec3 m_cachedOrigin { 0.0f };
    bool m_initialized = false;
};
} // namespace WallpaperEngine::Render::Objects
