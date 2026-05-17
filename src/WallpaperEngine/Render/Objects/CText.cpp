#include "CText.h"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "WallpaperEngine/Data/Model/DynamicValue.h"
#include "WallpaperEngine/Data/Model/Object.h"
#include "WallpaperEngine/Data/Model/UserSetting.h"
#include "WallpaperEngine/Logging/Log.h"
#include "WallpaperEngine/Render/CObject.h"
#include "WallpaperEngine/Render/Camera.h"
#include "WallpaperEngine/Render/Text/Utf8.h"
#include "WallpaperEngine/Render/Wallpapers/CScene.h"
#include "WallpaperEngine/Scripting/ScriptEngine.h"

using namespace WallpaperEngine::Render::Objects;
using WallpaperEngine::Data::Model::DynamicValue;
using WallpaperEngine::Render::CObject;

namespace {
// TODO: Phase 2 – load font from wallpaper's materials/fonts/ using AssetLocator
// Phase 1 uses a system font instead of the font shipped by the wallpaper.
// Wallpaper Engine bundles .ttf files in `materials/fonts/`; wiring those in
// is deferred to Phase 2 along with dynamic/scripted text.
#if defined(_WIN32)
const std::vector<std::string> kFontCandidates = {
    "C:/Windows/Fonts/segoeui.ttf",
    "C:/Windows/Fonts/arial.ttf",
    "C:/Windows/Fonts/calibri.ttf",
    "C:/Windows/Fonts/msyh.ttc",
};
#else
const std::vector<std::string> kFontCandidates = {
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/TTF/DejaVuSans.ttf",
    "/usr/share/fonts/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
};
#endif

const char* kVertexShader = R"glsl(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
uniform mat4 uMVP;
out vec2 vUV;
void main() {
    vUV = aUV;
    gl_Position = uMVP * vec4(aPos, 0.0, 1.0);
}
)glsl";

const char* kFragmentShader = R"glsl(
#version 330 core
in vec2 vUV;
uniform sampler2D uTexture;
uniform vec4 uColor;
out vec4 FragColor;
void main() {
    float coverage = texture(uTexture, vUV).r;
    FragColor = vec4(uColor.rgb, uColor.a * coverage);
}
)glsl";

// Sum this object's origin with every ancestor origin (WE space), same as CImage does at setup.
glm::vec3 accumulatedTextOriginWE (
    const WallpaperEngine::Render::Wallpapers::CScene& scene, const WallpaperEngine::Data::Model::Text& text
) {
    glm::vec3 o = text.origin->value->getVec3 ();
    std::optional<int> pid = text.parent;
    for (int guard = 0; pid.has_value () && guard < 64; ++guard) {
        const CObject* parentObj = scene.tryGetObject (*pid);
        if (parentObj == nullptr) {
            break;
        }
        // 根据测试：相对 parent 的 origin，x,y 好像都是2倍的关系？不清楚为什么，先照着写
        o = glm::vec3 (o.x / 2, o.y / 2, 0) + parentObj->getObject ().origin->value->getVec3 ();
        pid = parentObj->getObject ().parent;
    }
    return o;
}

// WE text `origin` is an alignment anchor; the quad is centered in local space. Move anchor → bbox center (y down).
glm::vec3 textAnchorToCenterWE (
    glm::vec3 anchorWE, const std::string& horizontalAlign, const std::string& verticalAlign, float halfW, float halfH
) {
    glm::vec3 c = anchorWE;
    if (horizontalAlign.find ("left") != std::string::npos) {
        c.x += halfW;
    } else if (horizontalAlign.find ("right") != std::string::npos) {
        c.x -= halfW;
    }
    if (verticalAlign.find ("top") != std::string::npos) {
        c.y += halfH;
    } else if (verticalAlign.find ("bottom") != std::string::npos) {
        c.y -= halfH;
    }
    return c;
}

GLuint compileShader (GLenum type, const char* source) {
    GLuint shader = glCreateShader (type);
    glShaderSource (shader, 1, &source, nullptr);
    glCompileShader (shader);

    GLint status = GL_FALSE;
    glGetShaderiv (shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char log[1024];
        glGetShaderInfoLog (shader, sizeof (log), nullptr, log);
        sLog.error ("CText shader compile failed: ", log);
        glDeleteShader (shader);
        return 0;
    }
    return shader;
}
} // namespace

CText::CText (Wallpapers::CScene& scene, const Data::Model::Text& text) : CObject (scene, text), m_text (text) { }

CText::~CText () {
    if (m_layerHandle != Scripting::kInvalidLayerHandle) {
        Scripting::ScriptEngine::instance ().destroyLayer (m_layerHandle);
        m_layerHandle = Scripting::kInvalidLayerHandle;
    }
    if (m_vbo != 0) {
        glDeleteBuffers (1, &m_vbo);
    }
    if (m_vao != 0) {
        glDeleteVertexArrays (1, &m_vao);
    }
    if (m_program != 0) {
        glDeleteProgram (m_program);
    }
    if (m_texture != 0) {
        glDeleteTextures (1, &m_texture);
    }
    if (m_ftFace != nullptr) {
        FT_Done_Face (m_ftFace);
    }
    if (m_ftLibrary != nullptr) {
        FT_Done_FreeType (m_ftLibrary);
    }
}

void CText::setup () {
    const bool scripted = !m_text.script.empty ();

    // Nothing to render and no script to produce text later → bail.
    if (m_text.text.empty () && !scripted) {
        return;
    }

    if (!initFreeType ()) {
        return;
    }

    if (!loadEmbeddedFont () && !loadSystemFont ()) {
        return;
    }

    buildShader ();

    if (scripted) {
        initScriptLayer ();
    }

    m_valid = m_program != 0;
}

bool CText::initFreeType () {
    if (FT_Init_FreeType (&m_ftLibrary) == 0) {
        return true;
    }
    sLog.error ("CText: FT_Init_FreeType failed for object ", m_text.name);
    return false;
}

bool CText::loadEmbeddedFont () {
    // Wallpapers packed in .pkg don't expose physical paths, so we read the font
    // into memory and use FT_New_Memory_Face. m_fontData must outlive the face.
    // `systemfont_*` references signal "use a system font"; let the fallback handle them.
    if (m_text.font.empty () || m_text.font.rfind ("systemfont_", 0) == 0) {
        return false;
    }

    try {
        auto stream = getAssetLocator ().read (m_text.font);
        stream->seekg (0, std::ios::end);
        const auto size = stream->tellg ();
        stream->seekg (0, std::ios::beg);
        m_fontData.resize (static_cast<size_t> (size));
        stream->read (reinterpret_cast<char*> (m_fontData.data ()), size);

        if (FT_New_Memory_Face (
                m_ftLibrary, m_fontData.data (), static_cast<FT_Long> (m_fontData.size ()), 0, &m_ftFace
            )
            == 0) {
            return true;
        }

        sLog.error ("CText: FT_New_Memory_Face failed for '", m_text.font, "', falling back to system font");
    } catch (const std::exception& e) {
        sLog.error ("CText: cannot read font '", m_text.font, "': ", e.what (), ", falling back to system font");
    }

    m_fontData.clear ();
    return false;
}

bool CText::loadSystemFont () {
    std::string fontPath;
    for (const auto& candidate : kFontCandidates) {
        if (std::filesystem::exists (candidate)) {
            fontPath = candidate;
            break;
        }
    }
#if defined(_WIN32)
    if (fontPath.empty ()) {
        if (const char* windir = std::getenv ("WINDIR")) {
            const std::filesystem::path fontsDir = std::filesystem::path (windir) / "Fonts";
            for (const char* name : { "segoeui.ttf", "arial.ttf", "calibri.ttf", "msyh.ttc" }) {
                const auto p = fontsDir / name;
                if (std::filesystem::exists (p)) {
                    fontPath = p.string ();
                    break;
                }
            }
        }
    }
#endif
    if (fontPath.empty ()) {
        sLog.error ("CText: no usable system font found");
        return false;
    }
    if (FT_New_Face (m_ftLibrary, fontPath.c_str (), 0, &m_ftFace) != 0) {
        sLog.error ("CText: FT_New_Face failed for ", fontPath);
        return false;
    }
    return true;
}

unsigned int CText::computeEffectivePixelSize () const {
    // WE text objects often come with scale ~0.09 that, combined with a modest
    // pointsize, would rasterize glyphs to ~2px on screen (invisible). Rasterize
    // at higher resolution so that after the model scale is applied in render()
    // the on-screen size matches the intended pointsize.
    const glm::vec3 initialScale = m_text.scale->value->getVec3 ();
    const float avgScale = (initialScale.x + initialScale.y) * 0.5f;
    const float compensate = (avgScale > 0.0f && avgScale < 1.0f) ? std::min (1.0f / avgScale, 32.0f) : 1.0f;
    unsigned int effectiveSize
        = std::max<unsigned int> (1u, static_cast<unsigned int> (static_cast<float> (m_text.pointsize) * compensate));

    // 高分辨率（场景->实际 viewport）缩放补偿
    auto rawSize = static_cast<unsigned int> (
        m_text.pointsize * static_cast<float> (this->getScene ().getHeight ())
        / static_cast<float> (this->getScene ().getOutputHeight ())
    );
    effectiveSize = std::max (effectiveSize, rawSize);

    // windows 系统 dpi 缩放补偿
    auto systemDpi = GetDpiForWindow (GetDesktopWindow ());
    if (systemDpi > 0) {
        effectiveSize = effectiveSize * systemDpi / 96;
    }

    return effectiveSize;
}

void CText::initScriptLayer () {
    std::map<std::string, DynamicValue*> initialProps;
    for (const auto& [k, setting] : m_text.scriptProperties) {
        if (setting && setting->value) {
            initialProps.emplace (k, setting->value.get ());
        }
    }
    m_layerHandle = Scripting::ScriptEngine::instance ().createLayerScript (m_text.script, initialProps, m_text.text);
    if (m_layerHandle == Scripting::kInvalidLayerHandle) {
        sLog.error ("CText: createLayerScript failed for '", m_text.name, "'");
    }
}

void CText::measureText (
    unsigned int fontPointSize, const std::u32string& text, int& penX, int& maxAscent, int& maxDescent
) {
    FT_Set_Pixel_Sizes (m_ftFace, 0, static_cast<FT_UInt> (fontPointSize));

    FT_GlyphSlot slot = m_ftFace->glyph;
    for (char32_t c : text) {
        if (FT_Load_Char (m_ftFace, c, FT_LOAD_RENDER) != 0) {
            continue;
        }
        penX += slot->advance.x >> 6;
        maxAscent = std::max (maxAscent, slot->bitmap_top);
        maxDescent = std::max (maxDescent, static_cast<int> (slot->bitmap.rows) - slot->bitmap_top);
    }
}

void CText::rebuildTextureFrom (const std::string text, const glm::vec2& size) {
    // use utf32 to avoid encoding issues (eg: simple chinese characters)
    const std::u32string text32 = WallpaperEngine::Render::Text::utf8ToUtf32 (text.empty () ? std::string (" ") : text);

    int penX = 0;
    int maxAscent = 0; // 升部高
    int maxDescent = 0; // 降部高

    // Two-pass rasterization: first measure the bounding box, then rasterize
    // every glyph into a single grayscale bitmap. Phase 1 renders one line —
    // multi-line wrapping, alignment, and padding come with Phase 2.
    //
    // Safe to call repeatedly: GL handles (texture, VAO, VBO) are reused when
    // already allocated, so dynamic/scripted text can regenerate the glyph
    // bitmap every time the rendered string changes without leaking.
    //
    // 许多字体引擎（如 FreeType / DirectWrite）在计算文本 quad 的大小时，会用 BoundingBox * (LineHeight / FontSize)。
    // 对于 Arial 字体，其行高因子（Line Height Factor）通常在 1.15 到 1.25 之间
    auto fontPointSize = computeEffectivePixelSize ();
    measureText (fontPointSize, text32, penX, maxAscent, maxDescent);

    auto boundScale = static_cast<float> (fontPointSize) / static_cast<float> (maxAscent + maxDescent);
    if (boundScale > 1.0f) {
        auto fixedFontPointSize = static_cast<unsigned int> (static_cast<float> (fontPointSize) * boundScale);
        if (fixedFontPointSize > fontPointSize) {
            fontPointSize = fixedFontPointSize;
            if (fontPointSize > static_cast<unsigned int> (size.y)) {
                fontPointSize = static_cast<unsigned int> (size.y);
            }
            measureText (fontPointSize, text32, penX, maxAscent, maxDescent);
        }
    }

    const int width = std::max (1, penX);
    const int height = std::max (1, maxAscent + maxDescent);
    std::vector<uint8_t> pixels (static_cast<size_t> (width) * height, 0);

    FT_GlyphSlot slot = m_ftFace->glyph;

    penX = 0;
    for (char32_t c : text32) {
        if (FT_Load_Char (m_ftFace, c, FT_LOAD_RENDER) != 0) {
            continue;
        }

        const auto& bmp = slot->bitmap;
        const int originX = penX + slot->bitmap_left;
        const int originY = maxAscent - slot->bitmap_top;

        for (unsigned int row = 0; row < bmp.rows; ++row) {
            for (unsigned int col = 0; col < bmp.width; ++col) {
                const int dstX = originX + static_cast<int> (col);
                const int dstY = originY + static_cast<int> (row);
                if (dstX < 0 || dstX >= width || dstY < 0 || dstY >= height) {
                    continue;
                }
                pixels[static_cast<size_t> (dstY) * width + dstX] = bmp.buffer[row * bmp.pitch + col];
            }
        }

        penX += slot->advance.x >> 6;
    }

    const bool firstUpload = (m_texture == 0);
    if (firstUpload) {
        glGenTextures (1, &m_texture);
    }
    glBindTexture (GL_TEXTURE_2D, m_texture);
    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels.data ());
    if (firstUpload) {
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    m_quadSize = { static_cast<float> (width), static_cast<float> (height) };
    m_lastRenderedText = text;

    uploadQuadVertices ();
}

void CText::buildShader () {
    GLuint vs = compileShader (GL_VERTEX_SHADER, kVertexShader);
    GLuint fs = compileShader (GL_FRAGMENT_SHADER, kFragmentShader);
    if (vs == 0 || fs == 0) {
        if (vs) {
            glDeleteShader (vs);
        }
        if (fs) {
            glDeleteShader (fs);
        }
        return;
    }

    m_program = glCreateProgram ();
    glAttachShader (m_program, vs);
    glAttachShader (m_program, fs);
    glLinkProgram (m_program);
    glDeleteShader (vs);
    glDeleteShader (fs);

    GLint status = GL_FALSE;
    glGetProgramiv (m_program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char log[1024];
        glGetProgramInfoLog (m_program, sizeof (log), nullptr, log);
        sLog.error ("CText program link failed: ", log);
        glDeleteProgram (m_program);
        m_program = 0;
        return;
    }

    m_uMVP = glGetUniformLocation (m_program, "uMVP");
    m_uColor = glGetUniformLocation (m_program, "uColor");
    m_uTexture = glGetUniformLocation (m_program, "uTexture");
}

void CText::uploadQuadVertices () {
    // Quad centered at the origin, sized in pixels. Scene-space placement is
    // done via the model matrix using the object's origin/scale. VBO contents
    // are re-uploaded whenever the glyph bitmap is rebuilt so the quad always
    // matches the current texture dimensions.
    const float hx = m_quadSize.x * 0.5f;
    const float hy = m_quadSize.y * 0.5f;

    // GL Y-up: +hy is the visual top of the quad. FreeType row 0 = glyph top → glTexImage2D row 0 = texture V=0.
    // Store V as (1 − v) so sampling matches the former fragment flip without shader remapping.
    const float verts[] = {
        // pos        // uv
        -hx, -hy, 0.0f, 0.0f, hx, -hy, 1.0f, 0.0f, hx,  hy, 1.0f, 1.0f,
        -hx, -hy, 0.0f, 0.0f, hx, hy,  1.0f, 1.0f, -hx, hy, 0.0f, 1.0f,
    };

    const bool firstUpload = (m_vao == 0);
    if (firstUpload) {
        glGenVertexArrays (1, &m_vao);
        glGenBuffers (1, &m_vbo);
    }
    glBindVertexArray (m_vao);
    glBindBuffer (GL_ARRAY_BUFFER, m_vbo);
    glBufferData (GL_ARRAY_BUFFER, sizeof (verts), verts, GL_DYNAMIC_DRAW);
    if (firstUpload) {
        glEnableVertexAttribArray (0);
        glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof (float), reinterpret_cast<void*> (0));
        glEnableVertexAttribArray (1);
        glVertexAttribPointer (
            1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof (float), reinterpret_cast<void*> (2 * sizeof (float))
        );
    }
    glBindVertexArray (0);
}

void CText::render () {
    if (!m_valid) {
        return;
    }
    if (!m_text.visible->value->getBool ()) {
        return;
    }
    if (m_text.parent.has_value ()) {
        auto parent = getScene ().tryGetObject (m_text.parent.value ());
        if (parent != nullptr && parent->getObject ().is<Data::Model::Text> ()) {
            auto* parentText = dynamic_cast<const CText*> (parent);
            if (!parentText->m_text.visible->value->getBool ()) {
                return;
            }
        }
    }

    if (m_layerHandle != Scripting::kInvalidLayerHandle) {
        auto& se = Scripting::ScriptEngine::instance ();
        se.tickLayer (
            m_layerHandle, static_cast<double> (getScene ().getTime ()),
            static_cast<double> (getScene ().getDeltaTime ()), static_cast<double> (getScene ().getFps ())
        );
        const std::string current = se.layerText (m_layerHandle);
        if (current != m_lastRenderedText) {
            rebuildTextureFrom (current, m_text.size);
            if (m_texture == 0 || m_vao == 0) {
                sLog.error ("CText: failed to rebuild texture for '", m_text.name, "'");
                return;
            }
        }
    }
    if (m_texture == 0) { // static text
        rebuildTextureFrom (m_text.text, m_text.size);
        if (m_texture == 0 || m_vao == 0) {
            sLog.error ("CText: failed to rebuild texture for '", m_text.name, "'");
            return;
        }
    }

    const DynamicValue& colorDv = *m_text.color->value;
    glm::vec4 color;
    switch (colorDv.getType ()) {
        case DynamicValue::Vec3:
            color = glm::vec4 (colorDv.getVec3 (), 1.0f);
            break;
        case DynamicValue::IVec3:
            {
                const glm::ivec3 ic = colorDv.getIVec3 ();
                color = glm::vec4 (glm::vec3 (ic) / 255.0f, 1.0f);
                break;
            }
        default:
            color = colorDv.getVec4 ();
            break;
    }
    const float alpha = m_text.alpha->value->getFloat ();
    const glm::vec3 scale = m_text.scale->value->getVec3 ();

    glm::vec3 weAnchor = accumulatedTextOriginWE (getScene (), m_text);
    const float halfW = m_quadSize.x * std::abs (scale.x) * 0.5f;
    const float halfH = m_quadSize.y * std::abs (scale.y) * 0.5f;
    const glm::vec3 weCenter = textAnchorToCenterWE (weAnchor, m_text.alignment, m_text.verticalalign, halfW, halfH);

    // WE: Y increases downward, origin top-left of scene. Convert anchor/center to GL Y-up centered scene space.
    const float scene_w = getScene ().getCamera ().getWidth ();
    const float scene_h = getScene ().getCamera ().getHeight ();
    // Force billboard Z=0 like CImage quads; scripted origin.z can push the quad outside the clip volume with
    // projection * lookAt.
    const glm::vec3 gl_origin = {
        weCenter.x - scene_w * 0.5f,
        scene_h * 0.5f - weCenter.y,
        0.0f,
    };

    glm::mat4 model = glm::translate (glm::mat4 (1.0f), gl_origin);
    model = glm::scale (model, scale);

    const glm::mat4 mvp = getScene ().getCamera ().getProjection () * getScene ().getCamera ().getLookAt () * model;

    const GLboolean depthWas = glIsEnabled (GL_DEPTH_TEST);
    const GLboolean cullWas = glIsEnabled (GL_CULL_FACE);
    GLboolean colorMaskWas[4] = { GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE };
    glGetBooleanv (GL_COLOR_WRITEMASK, colorMaskWas);

    // Image passes often enable culling and depth; our quad winding reads as back-facing
    // under CCW, and depth can hide screen-space text drawn at z=0.
    glDisable (GL_DEPTH_TEST);
    glDisable (GL_CULL_FACE);
    glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram (m_program);
    glUniformMatrix4fv (m_uMVP, 1, GL_FALSE, glm::value_ptr (mvp));
    glUniform4f (m_uColor, color.r, color.g, color.b, color.a * alpha);

    glActiveTexture (GL_TEXTURE0);
    glBindTexture (GL_TEXTURE_2D, m_texture);
    glUniform1i (m_uTexture, 0);

    glBindVertexArray (m_vao);
    glDrawArrays (GL_TRIANGLES, 0, 6);
    glBindVertexArray (0);

    if (depthWas == GL_TRUE) {
        glEnable (GL_DEPTH_TEST);
    } else {
        glDisable (GL_DEPTH_TEST);
    }
    if (cullWas == GL_TRUE) {
        glEnable (GL_CULL_FACE);
    } else {
        glDisable (GL_CULL_FACE);
    }
    glColorMask (colorMaskWas[0], colorMaskWas[1], colorMaskWas[2], colorMaskWas[3]);
}