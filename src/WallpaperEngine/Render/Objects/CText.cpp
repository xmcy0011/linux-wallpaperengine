#include "CText.h"

#include <GL/glew.h>

#include <algorithm>
#include <cstddef>
#include <glm/gtc/matrix_transform.hpp>
#include <iterator>

#include "WallpaperEngine/Data/Model/Object.h"
#include "WallpaperEngine/Logging/Log.h"
#include "WallpaperEngine/Render/Text/Utf8.h"

using namespace WallpaperEngine::Render::Objects;

namespace {
std::string lowerAscii (std::string s) {
    for (char& c : s) {
        if (static_cast<unsigned char> (c) >= 'A' && static_cast<unsigned char> (c) <= 'Z') {
            c = static_cast<char> (c - 'A' + 'a');
        }
    }
    return s;
}

GLuint compileShader (GLenum type, const char* source) {
    const GLuint id = glCreateShader (type);
    glShaderSource (id, 1, &source, nullptr);
    glCompileShader (id);

    GLint ok = GL_FALSE;
    glGetShaderiv (id, GL_COMPILE_STATUS, &ok);
    if (ok != GL_TRUE) {
        GLint len = 0;
        glGetShaderiv (id, GL_INFO_LOG_LENGTH, &len);
        std::string log (static_cast<size_t> (len), '\0');
        if (len > 0) {
            glGetShaderInfoLog (id, len, nullptr, log.data ());
        }
        sLog.error ("CText shader compile failed: ", log);
    }

    return id;
}
} // namespace

CText::CText (Wallpapers::CScene& scene, const Data::Model::Text& text) : CObject (scene, text), m_text (text) { }

CText::~CText () {
    if (this->m_program != 0) {
        glDeleteProgram (this->m_program);
    }
    if (this->m_ebo != 0) {
        glDeleteBuffers (1, &this->m_ebo);
    }
    if (this->m_vbo != 0) {
        glDeleteBuffers (1, &this->m_vbo);
    }
    if (this->m_vao != 0) {
        glDeleteVertexArrays (1, &this->m_vao);
    }
}

void CText::ensureProgram () {
    if (this->m_program != 0) {
        return;
    }

    constexpr const char* vs = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec2 aUv;
        uniform mat4 uMvp;
        out vec2 vUv;
        void main() {
            gl_Position = uMvp * vec4(aPos, 1.0);
            vUv = aUv;
        }
    )";

    constexpr const char* fs = R"(
        #version 330 core
        in vec2 vUv;
        uniform sampler2D uAtlas;
        uniform vec4 uColor;
        out vec4 outColor;
        void main() {
            float a = texture(uAtlas, vUv).r;
            outColor = vec4(uColor.rgb, uColor.a * a);
        }
    )";

    const GLuint v = compileShader (GL_VERTEX_SHADER, vs);
    const GLuint f = compileShader (GL_FRAGMENT_SHADER, fs);

    this->m_program = glCreateProgram ();
    glAttachShader (this->m_program, v);
    glAttachShader (this->m_program, f);
    glLinkProgram (this->m_program);

    glDeleteShader (v);
    glDeleteShader (f);

    GLint ok = GL_FALSE;
    glGetProgramiv (this->m_program, GL_LINK_STATUS, &ok);
    if (ok != GL_TRUE) {
        GLint len = 0;
        glGetProgramiv (this->m_program, GL_INFO_LOG_LENGTH, &len);
        std::string log (static_cast<size_t> (len), '\0');
        if (len > 0) {
            glGetProgramInfoLog (this->m_program, len, nullptr, log.data ());
        }
        sLog.error ("CText shader link failed: ", log);
    }

    this->m_uMvp = glGetUniformLocation (this->m_program, "uMvp");
    this->m_uColor = glGetUniformLocation (this->m_program, "uColor");
    this->m_uAtlas = glGetUniformLocation (this->m_program, "uAtlas");
}

void CText::setup () {
    if (this->m_initialized) {
        return;
    }

    this->ensureProgram ();

    glGenVertexArrays (1, &this->m_vao);
    glGenBuffers (1, &this->m_vbo);
    glGenBuffers (1, &this->m_ebo);

    glBindVertexArray (this->m_vao);
    glBindBuffer (GL_ARRAY_BUFFER, this->m_vbo);
    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, this->m_ebo);

    glEnableVertexAttribArray (0);
    glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), reinterpret_cast<void*> (offsetof (Vertex, pos)));

    glEnableVertexAttribArray (1);
    glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, sizeof (Vertex), reinterpret_cast<void*> (offsetof (Vertex, uv)));

    this->m_initialized = true;
}

glm::vec4 CText::getColor4 () const {
    const auto& dv = *this->m_text.color->value;
    const auto type = dv.getType ();
    if (type == DynamicValue::Null) {
        return glm::vec4 (1.0f);
    }
    if (type == DynamicValue::Vec4) {
        return dv.getVec4 ();
    }
    if (type == DynamicValue::IVec4) {
        const glm::ivec4 c = dv.getIVec4 ();
        constexpr float s = 1.0f / 255.0f;
        return glm::vec4 (static_cast<float> (c.x) * s, static_cast<float> (c.y) * s, static_cast<float> (c.z) * s,
                          static_cast<float> (c.w) * s);
    }
    if (type == DynamicValue::Vec3) {
        return glm::vec4 (dv.getVec3 (), 1.0f);
    }
    if (type == DynamicValue::IVec3) {
        const glm::ivec3 c = dv.getIVec3 ();
        constexpr float s = 1.0f / 255.0f;
        return glm::vec4 (static_cast<float> (c.x) * s, static_cast<float> (c.y) * s, static_cast<float> (c.z) * s,
                          1.0f);
    }
    return glm::vec4 (1.0f);
}

void CText::rebuildMeshIfNeeded () {
    const std::string content = this->m_text.text->value->getString ();
    const float fontSize = this->m_text.fontSize->value->getFloat ();
    const std::string fontPath = this->m_text.font->value->getString ();
    glm::vec3 originNow = m_text.origin->value->getVec3 ();
    const glm::vec3 scaleNow = this->m_text.scale->value->getVec3 ();

    if (m_text.parent.has_value ()) {
        originNow += this->getScene ().getObject (m_text.parent.value ())->getObject ().origin->value->getVec3 ();
    }

    if (content == this->m_cachedText && fontSize == this->m_cachedSize && fontPath == this->m_cachedFont
        && originNow == this->m_cachedOrigin) {
        return;
    }

    if (!this->m_atlas || fontSize != this->m_cachedSize || fontPath != this->m_cachedFont) {
        // Prefer VFS stream; workshop fonts usually don't exist as real OS files.
        const auto stream = this->getAssetLocator ().read (fontPath);
        std::vector<uint8_t> fontBytes;
        fontBytes.assign (std::istreambuf_iterator<char> (*stream), std::istreambuf_iterator<char> ());

        if (!fontBytes.empty ()) {
            this->m_atlas = std::make_shared<WallpaperEngine::Render::Text::FontAtlas> (
                std::move (fontBytes), static_cast<uint32_t> (std::max (fontSize, 8.0f))
            );
        } else {
            throw std::runtime_error ("empty font stream");
        }
    }

    this->m_vertices.clear ();
    this->m_indices.clear ();

    if (content.empty ()) {
        this->m_cachedText = content;
        this->m_cachedSize = fontSize;
        this->m_cachedFont = fontPath;
        this->m_cachedOrigin = originNow;
        return;
    }

    const glm::vec3 origin = originNow;
    const glm::vec3 scale = scaleNow;
    const float sx = scale.x;
    const float sy = scale.y;
    const float sceneW = static_cast<float> (this->getScene ().getWidth ());
    const float sceneH = static_cast<float> (this->getScene ().getHeight ());

    const std::string hAlign = lowerAscii (this->m_text.horizontalAlign);
    const std::string vAlign = lowerAscii (this->m_text.verticalAlign);

    const std::u32string text32 = WallpaperEngine::Render::Text::utf8ToUtf32 (content);
    std::vector<std::u32string> lines;
    lines.emplace_back ();
    for (char32_t cp : text32) {
        if (cp == U'\n') {
            lines.emplace_back ();
        } else {
            lines.back ().push_back (cp);
        }
    }

    std::vector<float> lineWidths;
    lineWidths.reserve (lines.size ());
    for (const auto& line : lines) {
        float w = 0.0f;
        for (char32_t cp : line) {
            const auto& g = this->m_atlas->getGlyph (cp);
            w += static_cast<float> (g.advance >> 6) * sx;
        }
        lineWidths.push_back (w);
    }

    const float lineH = fontSize * sy;
    const auto lineStartX = [&] (const float lineWidth) {
        float x = origin.x - sceneW / 2.0f;
        if (hAlign.find ("center") != std::string::npos) {
            x -= lineWidth * 0.5f;
        } else if (hAlign.find ("right") != std::string::npos) {
            x -= lineWidth;
        }
        return x;
    };

    float penY = sceneH / 2.0f - origin.y;
    if (vAlign.find ("center") != std::string::npos) {
        if (lines.size () == 1) {
            penY += lineH * 0.5f;
        } else {
            penY += (static_cast<float> (lines.size ()) - 1.0f) * lineH * 0.5f;
        }
    } else if (vAlign.find ("bottom") != std::string::npos) {
        penY += (static_cast<float> (lines.size ()) - 1.0f) * lineH;
    }

    size_t lineIndex = 0;
    float penX = lineStartX (lineWidths[lineIndex]);

    for (char32_t cp : text32) {
        if (cp == U'\n') {
            lineIndex++;
            penX = lineStartX (lineWidths[lineIndex]);
            penY -= lineH;
            continue;
        }

        const auto& glyph = this->m_atlas->getGlyph (cp);
        const float x0 = penX + static_cast<float> (glyph.bearing.x) * sx;
        const float y0 = penY - static_cast<float> (glyph.bearing.y) * sy;
        const float w = static_cast<float> (glyph.size.x) * sx;
        const float h = static_cast<float> (glyph.size.y) * sy;
        const float x1 = x0 + w;
        const float y1 = y0 + h;

        const GLuint base = static_cast<GLuint> (this->m_vertices.size ());
        this->m_vertices.push_back ({ .pos = { x0, y0, 0.0f }, .uv = { glyph.uv.x, glyph.uv.y } });
        this->m_vertices.push_back ({ .pos = { x1, y0, 0.0f }, .uv = { glyph.uv.z, glyph.uv.y } });
        this->m_vertices.push_back ({ .pos = { x1, y1, 0.0f }, .uv = { glyph.uv.z, glyph.uv.w } });
        this->m_vertices.push_back ({ .pos = { x0, y1, 0.0f }, .uv = { glyph.uv.x, glyph.uv.w } });

        this->m_indices.push_back (base + 0);
        this->m_indices.push_back (base + 1);
        this->m_indices.push_back (base + 2);
        this->m_indices.push_back (base + 2);
        this->m_indices.push_back (base + 3);
        this->m_indices.push_back (base + 0);

        penX += static_cast<float> (glyph.advance >> 6) * sx;
    }

    glBindBuffer (GL_ARRAY_BUFFER, this->m_vbo);
    glBufferData (
        GL_ARRAY_BUFFER, static_cast<GLsizeiptr> (this->m_vertices.size () * sizeof (Vertex)), this->m_vertices.data (),
        GL_DYNAMIC_DRAW
    );
    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, this->m_ebo);
    glBufferData (
        GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr> (this->m_indices.size () * sizeof (GLuint)),
        this->m_indices.data (), GL_DYNAMIC_DRAW
    );

    this->m_cachedText = content;
    this->m_cachedSize = fontSize;
    this->m_cachedFont = fontPath;
    this->m_cachedOrigin = originNow;
}

void CText::render () {
    if (!this->m_initialized) {
        this->setup ();
    }

    if (!this->m_text.visible->value->getBool ()) {
        return;
    }

    this->rebuildMeshIfNeeded ();
    if (this->m_indices.empty () || !this->m_atlas) {
        return;
    }

    GLint prevProgram = 0;
    GLint prevVao = 0;
    glGetIntegerv (GL_CURRENT_PROGRAM, &prevProgram);
    glGetIntegerv (GL_VERTEX_ARRAY_BINDING, &prevVao);

    const GLboolean depthTestWas = glIsEnabled (GL_DEPTH_TEST);
    const GLboolean cullFaceWas = glIsEnabled (GL_CULL_FACE);
    GLboolean colorMaskWas[4] = { GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE };
    glGetBooleanv (GL_COLOR_WRITEMASK, colorMaskWas);

    // Image passes often leave GL_CULL_FACE on; our screen-space quads use winding that reads as "back"
    // faces under the default CCW front rule, so the entire string would be culled.
    glDisable (GL_DEPTH_TEST);
    glDisable (GL_CULL_FACE);
    glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram (this->m_program);
    glBindVertexArray (this->m_vao);
    glActiveTexture (GL_TEXTURE0);
    glBindTexture (GL_TEXTURE_2D, this->m_atlas->getTextureId ());
    glUniform1i (this->m_uAtlas, 0);

    glm::mat4 mvp = this->getScene ().getCamera ().getProjection () * this->getScene ().getCamera ().getLookAt ();
    glm::vec3 angles = this->m_text.angles->value->getVec3 ();
    if (angles.x != 0.0f || angles.y != 0.0f || angles.z != 0.0f) {
        mvp = glm::rotate (mvp, -angles.z, glm::vec3 (0.0f, 0.0f, 1.0f));
        mvp = glm::rotate (mvp, angles.y, glm::vec3 (0.0f, 1.0f, 0.0f));
        mvp = glm::rotate (mvp, -angles.x, glm::vec3 (1.0f, 0.0f, 0.0f));
    }
    glUniformMatrix4fv (this->m_uMvp, 1, GL_FALSE, &mvp[0][0]);

    glm::vec4 color = this->getColor4 ();
    glUniform4f (this->m_uColor, color.r, color.g, color.b, color.a);

    glDrawElements (GL_TRIANGLES, static_cast<GLsizei> (this->m_indices.size ()), GL_UNSIGNED_INT, nullptr);

    glColorMask (colorMaskWas[0], colorMaskWas[1], colorMaskWas[2], colorMaskWas[3]);
    if (depthTestWas) {
        glEnable (GL_DEPTH_TEST);
    } else {
        glDisable (GL_DEPTH_TEST);
    }
    if (cullFaceWas) {
        glEnable (GL_CULL_FACE);
    } else {
        glDisable (GL_CULL_FACE);
    }

    glBindVertexArray (prevVao);
    glUseProgram (prevProgram);
}
