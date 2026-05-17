#pragma once

#include "WallpaperEngine/Render/Camera.h"

#include "WallpaperEngine/Render/CWallpaper.h"

namespace WallpaperEngine::Render {
class Camera;
class CObject;
}

namespace WallpaperEngine::Render::Wallpapers {
using namespace WallpaperEngine::Data::Model;

class CScene final : public CWallpaper {
public:
    CScene (
	const Wallpaper& wallpaper, RenderContext& context, AudioContext& audioContext,
	const WallpaperState::TextureUVsScaling& scalingMode, const uint32_t& clampMode
    );

    ~CScene () override;

    [[nodiscard]] Camera& getCamera () const;

    [[nodiscard]] const Scene& getScene () const;

    [[nodiscard]] int getWidth () const override;
    [[nodiscard]] int getHeight () const override;
    [[nodiscard]] int getOutputHeight () const;
    [[nodiscard]] int getOutputWidth () const;
    
    // Time accessors used by dynamic text layers (CText + ScriptEngine).
    // Read from the application-wide g_Time/g_TimeLast globals that other
    // renderers already consume via extern (e.g. CParticle).
    [[nodiscard]] float getTime () const;
    [[nodiscard]] float getDeltaTime () const;
    [[nodiscard]] float getFps () const;

    const glm::vec2* getMousePosition () const;
    const glm::vec2* getMousePositionLast () const;
    const glm::vec2* getMousePositionNormalized () const;
    const glm::vec2* getParallaxDisplacement () const;

    [[nodiscard]] const std::vector<CObject*>& getObjectsByRenderOrder () const;
    [[nodiscard]] const CObject* getObject (int id) const;
    /** Like getObject but returns nullptr if id is missing (avoids exceptions on bad scene data). */
    [[nodiscard]] const CObject* tryGetObject (int id) const noexcept;

protected:
    void renderFrame (const glm::ivec4& viewport) override;
    void updateMouse (const glm::ivec4& viewport);

    friend class CWallpaper;

private:
    Render::CObject* createObject (const Object& object);
    void addObjectToRenderOrder (const Object& object);

    std::unique_ptr<Camera> m_camera;
    ObjectUniquePtr m_bloomObjectData;
    CObject* m_bloomObject = nullptr;
    std::map<int, CObject*> m_objects = {};
    std::vector<CObject*> m_objectsByRenderOrder = {};
    glm::vec2 m_mousePosition = {};
    glm::vec2 m_mousePositionLast = {};
    glm::vec2 m_mousePositionNormalized = {};
    glm::vec2 m_parallaxDisplacement = {};
    int m_outputHeight = 0;
    int m_outputWidth = 0;
    std::shared_ptr<const CFBO> _rt_4FrameBuffer = nullptr;
    std::shared_ptr<const CFBO> _rt_8FrameBuffer = nullptr;
    std::shared_ptr<const CFBO> _rt_Bloom = nullptr;
    std::shared_ptr<const CFBO> _rt_shadowAtlas = nullptr;
};
} // namespace WallpaperEngine::Render::Wallpaper
