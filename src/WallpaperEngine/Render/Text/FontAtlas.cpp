#include "FontAtlas.h"

#include <GL/glew.h>
#include <algorithm>

#include "WallpaperEngine/Logging/Log.h"

using namespace WallpaperEngine::Render::Text;

namespace {
constexpr int kPadding = 1;

void initAtlasTexture (uint32_t& textureId, int atlasW, int atlasH) {
    glGenTextures (1, &textureId);
    glBindTexture (GL_TEXTURE_2D, textureId);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_R8, atlasW, atlasH, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
}
}

FontAtlas::FontAtlas (const std::string& fontPath, uint32_t pixelSize) : m_pixelSize (pixelSize) {
    if (FT_Init_FreeType (&this->m_ftLib) != 0) {
        sLog.error ("FontAtlas: FT_Init_FreeType failed");
        return;
    }

    if (FT_New_Face (this->m_ftLib, fontPath.c_str (), 0, &this->m_face) != 0) {
        sLog.error ("FontAtlas: cannot load font: ", fontPath);
        return;
    }

    if (FT_Set_Pixel_Sizes (this->m_face, 0, this->m_pixelSize) != 0) {
        sLog.error ("FontAtlas: cannot set pixel size: ", this->m_pixelSize);
    }

    initAtlasTexture (this->m_textureId, this->m_atlasW, this->m_atlasH);
}

FontAtlas::FontAtlas (std::vector<uint8_t> fontData, uint32_t pixelSize) :
    m_pixelSize (pixelSize), m_fontData (std::move (fontData)) {
    if (FT_Init_FreeType (&this->m_ftLib) != 0) {
        sLog.error ("FontAtlas: FT_Init_FreeType failed");
        return;
    }

    if (this->m_fontData.empty ()) {
        sLog.error ("FontAtlas: empty in-memory font data");
        return;
    }

    if (FT_New_Memory_Face (
            this->m_ftLib, reinterpret_cast<const FT_Byte*> (this->m_fontData.data ()),
            static_cast<FT_Long> (this->m_fontData.size ()), 0, &this->m_face
        )
        != 0) {
        sLog.error ("FontAtlas: cannot load in-memory font face");
        return;
    }

    if (FT_Set_Pixel_Sizes (this->m_face, 0, this->m_pixelSize) != 0) {
        sLog.error ("FontAtlas: cannot set pixel size: ", this->m_pixelSize);
    }

    initAtlasTexture (this->m_textureId, this->m_atlasW, this->m_atlasH);
}

FontAtlas::~FontAtlas () {
    if (this->m_textureId != 0) {
        glDeleteTextures (1, &this->m_textureId);
    }

    if (this->m_face != nullptr) {
        FT_Done_Face (this->m_face);
    }

    if (this->m_ftLib != nullptr) {
        FT_Done_FreeType (this->m_ftLib);
    }
}

const GlyphInfo& FontAtlas::getGlyph (char32_t codepoint) {
    this->ensureGlyph (codepoint);
    return this->m_glyphs.at (codepoint);
}

void FontAtlas::ensureGlyph (char32_t cp) {
    if (this->m_glyphs.contains (cp)) {
        return;
    }

    if (this->m_face == nullptr) {
        this->m_glyphs.emplace (
            cp, GlyphInfo { .size = { 0, 0 }, .bearing = { 0, 0 }, .advance = 0, .uv = { 0.0f, 0.0f, 0.0f, 0.0f } }
        );
        return;
    }

    const auto loadGlyph = [&] (char32_t codepoint) -> bool {
        return FT_Load_Char (this->m_face, static_cast<FT_ULong> (codepoint), FT_LOAD_RENDER) == 0;
    };

    if (loadGlyph (cp)) {
        this->uploadGlyphToAtlas (cp, this->m_face->glyph);
        return;
    }

    // Non-recursive fallback to '?' to satisfy clang-tidy misc-no-recursion.
    if (cp != U'?' && loadGlyph (U'?')) {
        this->uploadGlyphToAtlas (U'?', this->m_face->glyph);
        this->m_glyphs.emplace (cp, this->m_glyphs.at (U'?'));
        return;
    }

    this->m_glyphs.emplace (
        cp, GlyphInfo { .size = { 0, 0 }, .bearing = { 0, 0 }, .advance = 0, .uv = { 0.0f, 0.0f, 0.0f, 0.0f } }
    );
}

void FontAtlas::uploadGlyphToAtlas (char32_t cp, FT_GlyphSlot slot) {
    const int w = static_cast<int> (slot->bitmap.width);
    const int h = static_cast<int> (slot->bitmap.rows);

    if (w == 0 || h == 0) {
        this->m_glyphs.emplace (
            cp,
            GlyphInfo { .size = { w, h },
                        .bearing = { slot->bitmap_left, slot->bitmap_top },
                        .advance = static_cast<uint32_t> (slot->advance.x),
                        .uv = { 0.0f, 0.0f, 0.0f, 0.0f } }
        );
        return;
    }

    if (this->m_cursorX + w + kPadding >= this->m_atlasW) {
        this->m_cursorX = kPadding;
        this->m_cursorY += this->m_rowH + kPadding;
        this->m_rowH = 0;
    }

    if (this->m_cursorY + h + kPadding >= this->m_atlasH) {
        sLog.error ("FontAtlas: atlas is full, codepoint skipped: ", static_cast<int> (cp));
        this->m_glyphs.emplace (
            cp,
            GlyphInfo { .size = { 0, 0 },
                        .bearing = { slot->bitmap_left, slot->bitmap_top },
                        .advance = static_cast<uint32_t> (slot->advance.x),
                        .uv = { 0.0f, 0.0f, 0.0f, 0.0f } }
        );
        return;
    }

    glBindTexture (GL_TEXTURE_2D, this->m_textureId);
    glTexSubImage2D (
        GL_TEXTURE_2D, 0, this->m_cursorX, this->m_cursorY, w, h, GL_RED, GL_UNSIGNED_BYTE, slot->bitmap.buffer
    );

    const float u0 = static_cast<float> (this->m_cursorX) / static_cast<float> (this->m_atlasW);
    const float v0 = static_cast<float> (this->m_cursorY) / static_cast<float> (this->m_atlasH);
    const float u1 = static_cast<float> (this->m_cursorX + w) / static_cast<float> (this->m_atlasW);
    const float v1 = static_cast<float> (this->m_cursorY + h) / static_cast<float> (this->m_atlasH);

    this->m_glyphs.emplace (
        cp,
        GlyphInfo { .size = { w, h },
                    .bearing = { slot->bitmap_left, slot->bitmap_top },
                    .advance = static_cast<uint32_t> (slot->advance.x),
                    .uv = { u0, v0, u1, v1 } }
    );

    this->m_cursorX += w + kPadding;
    this->m_rowH = std::max (this->m_rowH, h);
}
