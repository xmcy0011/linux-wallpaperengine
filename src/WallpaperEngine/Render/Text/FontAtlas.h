#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

namespace WallpaperEngine::Render::Text {

struct GlyphInfo {
    glm::ivec2 size;      // bitmap w/h
    glm::ivec2 bearing;   // left/top
    uint32_t advance;     // 26.6
    glm::vec4 uv;         // u0,v0,u1,v1
};

class FontAtlas {
public:
    FontAtlas(const std::string& fontPath, uint32_t pixelSize);
    FontAtlas(std::vector<uint8_t> fontData, uint32_t pixelSize);
    ~FontAtlas();

    const GlyphInfo& getGlyph(char32_t codepoint);
    [[nodiscard]] uint32_t getTextureId() const { return m_textureId; }
    [[nodiscard]] uint32_t getPixelSize() const { return m_pixelSize; }

private:
    void ensureGlyph(char32_t cp);
    void uploadGlyphToAtlas(char32_t cp, FT_GlyphSlot slot);

private:
    FT_Library m_ftLib = nullptr;
    FT_Face m_face = nullptr;
    uint32_t m_pixelSize = 0;
    std::vector<uint8_t> m_fontData;

    uint32_t m_textureId = 0;
    int m_atlasW = 2048;
    int m_atlasH = 2048;
    int m_cursorX = 1;
    int m_cursorY = 1;
    int m_rowH = 0;

    std::unordered_map<char32_t, GlyphInfo> m_glyphs;
};

using FontAtlasPtr = std::shared_ptr<FontAtlas>;
}