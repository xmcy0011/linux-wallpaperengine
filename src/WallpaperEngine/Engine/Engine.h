#pragma once

#include "WallpaperEngine/Core/Export.h"
#include "WallpaperEngine/Application/ApplicationContext.h"
#include "WallpaperEngine/Data/Assets/Types.h"
#include "WallpaperEngine/Render/WallpaperState.h"

#include <glm/vec4.hpp>

#include <atomic>
#include <filesystem>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

// Forward declarations
namespace WallpaperEngine::Application {
	class WallpaperApplication;
}

namespace WallpaperEngine::Data::Model {
	class Project;
	class Property;
	class PropertySlider;
	class PropertyBoolean;
	class PropertyColor;
	class PropertyCombo;
	class PropertyText;
	class PropertyTextInput;
	class PropertySceneTexture;
	class PropertyFile;
}

namespace WallpaperEngine::Engine {

// Property types matching the C API
enum class PropertyType {
	Unknown = 0,
	Slider,
	Boolean,
	Color,
	Combo,
	Text,
	TextInput,
	SceneTexture,
	File
};

// Property info structure
struct PropertyInfo {
	std::string name;
	std::string text;
	PropertyType type;
	std::string value;
	std::string minValue;
	std::string maxValue;
	std::string step;
	std::map<std::string, std::string> comboOptions;
};

// Property list for C API
class PropertyList {
public:
	std::vector<PropertyInfo> properties;

	PropertyList() = default;
	explicit PropertyList(std::vector<PropertyInfo> props) : properties(std::move(props)) {}
};

// Main Engine class - wraps WallpaperApplication for DLL usage
class Engine {
public:
	Engine();
	~Engine();

	// Prevent copying
	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;

	/*==============================================================================
	 * Configuration
	 *============================================================================*/

	/// @brief Set the assets directory path
	bool SetAssetsPath(const std::filesystem::path& path);

	/// @brief Load a wallpaper from a file path
	bool LoadWallpaper(const std::filesystem::path& path);

	/// @brief Set the maximum FPS
	void SetMaxFPS(int fps);

	/// @brief Set the scaling mode
	void SetScalingMode(WallpaperEngine::Render::WallpaperState::TextureUVsScaling mode);

	/// @brief Set the clamping mode
	void SetClampMode(WallpaperEngine::Data::Assets::TextureFlags mode);

	/// @brief Enable or disable particles
	void SetParticlesEnabled(bool enable);

	/*==============================================================================
	 * Playback Control
	 *============================================================================*/

	/// @brief Start the wallpaper rendering (starts render thread)
	bool Play();

	/// @brief Pause the wallpaper rendering (keeps thread alive)
	bool Pause();

	/// @brief Stop the wallpaper rendering (stops render thread)
	bool Stop();

	/// @brief Check if playing
	bool IsPlaying() const;

	/// @brief Check if paused
	bool IsPaused() const;

	/*==============================================================================
	 * Properties
	 *============================================================================*/

	/// @brief List all properties
	std::unique_ptr<PropertyList> ListProperties();

	/// @brief Set a property value
	bool SetProperty(const std::string& name, const std::string& value);

	/// @brief Get a property value
	std::optional<std::string> GetProperty(const std::string& name);

	/*==============================================================================
	 * Window Control
	 *============================================================================*/

	/// @brief Set the parent window handle for embedding
	/// @param hwnd Native window handle (HWND on Windows, Window on X11, etc.)
	/// @note Must be called before Play() to take effect
	void SetWindowHandle(void* hwnd);

	/// @brief Get the current window handle
	/// @return Native window handle or nullptr if not set
	void* GetWindowHandle() const;

	/// @brief Show the window
	void ShowWindow();

	/// @brief Hide the window
	void HideWindow();

	/// @brief Resize the window
	void ResizeWindow(int x, int y, int width, int height);

	/*==============================================================================
	 * Audio Control
	 *============================================================================*/

	/// @brief Enable or disable audio
	void SetAudioEnabled(bool enable);

	/// @brief Set the volume
	void SetVolume(int volume);

	/// @brief Mute or unmute
	void SetMuted(bool mute);

	/// @brief Enable or disable auto-mute
	void SetAutoMute(bool enable);

	/*==============================================================================
	 * Screenshot
	 *============================================================================*/

	/// @brief Take a screenshot
	bool TakeScreenshot(const std::filesystem::path& path);

	/*==============================================================================
	 * Error Handling
	 *============================================================================*/

	/// @brief Get the last error message
	const std::string& GetLastError() const;

	/// @brief Set an error message
	void SetLastError(const std::string& error);

	/// @brief Clear the last error
	void ClearError();

private:
	/*==============================================================================
	 * Internal State
	 *============================================================================*/

	// Application context and application
	std::unique_ptr<WallpaperEngine::Application::ApplicationContext> m_context;
	std::unique_ptr<WallpaperEngine::Application::WallpaperApplication> m_app;

	// Current wallpaper project
	std::unique_ptr<WallpaperEngine::Data::Model::Project> m_project;

	// Render thread
	std::unique_ptr<std::thread> m_renderThread;
	std::atomic<bool> m_isRunning;
	std::atomic<bool> m_isPaused;

	// Configuration
	std::string m_assetsPath;
	int m_maxFPS;
	void* m_windowHandle;  // External window handle (HWND on Windows, etc.)
	glm::ivec4 m_windowGeometry; // Cached window geometry (x, y, width, height)
	bool m_hasWindowGeometry = false;

	// Error handling
	std::string m_lastError;
	mutable std::mutex m_mutex;

	/*==============================================================================
	 * Helper Methods
	 *============================================================================*/

	/// @brief Initialize the application context
	bool InitializeContext();

	/// @brief Initialize the application
	bool InitializeApplication();

	/// @brief Render loop (runs in m_renderThread)
	void RenderLoop();

	/// @brief Setup a single viewport for windowed mode
	bool SetupViewport();

	/// @brief Convert Property type to PropertyInfo::PropertyType
	static PropertyType ConvertPropertyType(const WallpaperEngine::Data::Model::Property* prop);

	/// @brief Get the current project (thread-safe)
	WallpaperEngine::Data::Model::Project* GetProject();
};

} // namespace WallpaperEngine::Engine
