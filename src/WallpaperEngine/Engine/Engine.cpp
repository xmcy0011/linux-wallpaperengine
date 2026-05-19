#include "WallpaperEngine/Engine/Engine.h"

#include "WallpaperEngine/Application/WallpaperApplication.h"
#include "WallpaperEngine/Audio/Drivers/SDLAudioDriver.h"
#include "WallpaperEngine/Audio/Drivers/Detectors/AudioPlayingDetector.h"
#include "WallpaperEngine/Audio/Drivers/Recorders/PlaybackRecorder.h"
#include "WallpaperEngine/Data/Model/Property.h"
#include "WallpaperEngine/Data/Parsers/ProjectParser.h"
#include "WallpaperEngine/FileSystem/Container.h"
#include "WallpaperEngine/Logging/Log.h"
#include "WallpaperEngine/Render/Drivers/GLFWOpenGLDriver.h"
#include "WallpaperEngine/Render/Drivers/VideoFactories.h"
#include "WallpaperEngine/Render/RenderContext.h"
#include "WallpaperEngine/Render/Wallpapers/CScene.h"
#include "WallpaperEngine/Render/Wallpapers/CVideo.h"

#include <SDL2/SDL.h>
#include <ctime>

using namespace WallpaperEngine::Engine;
using namespace WallpaperEngine::Assets;
using namespace WallpaperEngine::Application;
using namespace WallpaperEngine::Data::Model;
using namespace WallpaperEngine::FileSystem;

// Global time variables (from WallpaperEngine)
extern float g_Time;
extern float g_TimeLast;
extern float g_Daytime;

/*==============================================================================
 * Constructor/Destructor
 *============================================================================*/

Engine::Engine()
	: m_isRunning(false)
	, m_isPaused(false)
	, m_maxFPS(30) {
	// Initialize SDL for the engine
	SDL_SetMainReady();
}

Engine::~Engine() {
	// Stop the render thread if running
	Stop();

	// Cleanup application
	if (m_app) {
		m_app->cleanup();
		m_app.reset();
	}

	// Cleanup context
	m_context.reset();
	m_project.reset();
}

/*==============================================================================
 * Configuration
 *============================================================================*/

bool Engine::SetAssetsPath(const std::filesystem::path& path) {
	std::unique_lock<std::mutex> lock(m_mutex);

	if (!std::filesystem::exists(path)) {
		SetLastError("Assets path does not exist: " + path.string());
		return false;
	}

	m_assetsPath = path.string();
	return true;
}

bool Engine::LoadWallpaper(const std::filesystem::path& path) {
	std::unique_lock<std::mutex> lock(m_mutex);

	// Check if assets path is set
	if (m_assetsPath.empty()) {
		SetLastError("Assets path not set. Call WE_SetAssetsPath first.");
		return false;
	}

	// Check if wallpaper path exists
	if (!std::filesystem::exists(path)) {
		SetLastError("Wallpaper path does not exist: " + path.string());
		return false;
	}

	try {
		// Initialize context if not already done
		if (!m_context) {
			if (!InitializeContext()) {
				return false;
			}
		}

		// Update context with wallpaper path BEFORE creating app
		m_context->settings.general.defaultBackground = path;
		m_context->settings.general.assets = m_assetsPath;

		// Setup asset locator
		auto container = std::make_unique<Container>();
		container->mount(path, "/");

		try {
			container->mount(path / "scene.pkg", "/");
		} catch (std::runtime_error&) {
		}

		try {
			container->mount(path / "gifscene.pkg", "/");
		} catch (std::runtime_error&) {
		}

		try {
			container->mount(m_assetsPath, "/");
		} catch (std::runtime_error& e) {
			SetLastError("Cannot mount assets folder: " + std::string(e.what()));
			return false;
		}

		try {
			container->mount(std::filesystem::current_path(), "/");
		} catch (std::runtime_error&) {
		}

		// Add VFS entries for bloom (copied from WallpaperApplication)
		auto& vfs = container->getVFS();
		vfs.add(
			"effects/wpenginelinux/bloomeffect.json",
			{ { "name", "camerabloom_wpengine_linux" },
			  { "group", "wpengine_linux_camera" },
			  { "dependencies", WallpaperEngine::Data::JSON::JSON::array() },
			  { "passes",
			    WallpaperEngine::Data::JSON::JSON::array(
			        { { { "material", "materials/util/downsample_quarter_bloom.json" },
			            { "target", "_rt_4FrameBuffer" },
			            { "bind", WallpaperEngine::Data::JSON::JSON::array({ { { "name", "_rt_FullFrameBuffer" }, { "index", 0 } } }) } },
			          { { "material", "materials/util/downsample_eighth_blur_v.json" },
			            { "target", "_rt_8FrameBuffer" },
			            { "bind", WallpaperEngine::Data::JSON::JSON::array({ { { "name", "_rt_4FrameBuffer" }, { "index", 0 } } }) } },
			          { { "material", "materials/util/blur_h_bloom.json" },
			            { "target", "_rt_Bloom" },
			            { "bind", WallpaperEngine::Data::JSON::JSON::array({ { { "name", "_rt_8FrameBuffer" }, { "index", 0 } } }) } },
			          { { "material", "materials/util/combine.json" },
			            { "target", "_rt_FullFrameBuffer" },
			            { "bind",
			              WallpaperEngine::Data::JSON::JSON::array(
			                  { { { "name", "_rt_imageLayerComposite_-1_a" }, { "index", 0 } },
			            { { "name", "_rt_Bloom" }, { "index", 1 } } }
			              ) } } }
			    ),
			  } }
		);
		vfs.add("models/wpenginelinux.json", { { "material", "materials/wpenginelinux.json" } });
		vfs.add(
			"materials/wpenginelinux.json",
			{ { "passes",
			    WallpaperEngine::Data::JSON::JSON::array(
			        { { { "blending", "normal" },
			            { "cullmode", "nocull" },
			            { "depthtest", "disabled" },
			            { "depthwrite", "disabled" },
			            { "shader", "genericimage2" },
			            { "textures", WallpaperEngine::Data::JSON::JSON::array({ "_rt_FullFrameBuffer" }) } } }
			    ) } }
		);
		vfs.add(
			"shaders/commands/copy.frag",
			"uniform sampler2D g_Texture0;\n"
			"in vec2 v_TexCoord;\n"
			"void main () {\n"
			"out_FragColor = texture (g_Texture0, v_TexCoord);\n"
			"}\n"
		);
		vfs.add(
			"shaders/commands/copy.vert",
			"in vec3 a_Position;\n"
			"in vec2 a_TexCoord;\n"
			"out vec2 v_TexCoord;\n"
			"void main () {\n"
			"gl_Position = vec4 (a_Position, 1.0);\n"
			"v_TexCoord = a_TexCoord;\n"
			"}\n"
		);

		auto assetLocator = std::make_unique<AssetLocator>(std::move(container));

		// Parse project.json
		auto json = WallpaperEngine::Data::JSON::JSON::parse(assetLocator->readString("project.json"));
		m_project = WallpaperEngine::Data::Parsers::ProjectParser::parse(json, std::move(assetLocator));

		// Store the project for property access
		// The WallpaperApplication will use the context settings to load the wallpaper

		ClearError();
		return true;

	} catch (const std::exception& e) {
		SetLastError("Failed to load wallpaper: " + std::string(e.what()));
		m_project.reset();
		return false;
	}
}

void Engine::SetMaxFPS(int fps) {
	m_maxFPS = fps;
	if (m_context) {
		m_context->settings.render.maximumFPS = fps;
	}
}

void Engine::SetScalingMode(WallpaperEngine::Render::WallpaperState::TextureUVsScaling mode) {
	if (m_context) {
		m_context->settings.render.window.scalingMode = mode;
	}
}

void Engine::SetClampMode(WallpaperEngine::Data::Assets::TextureFlags mode) {
	if (m_context) {
		m_context->settings.render.window.clamp = mode;
	}
}

void Engine::SetParticlesEnabled(bool enable) {
	if (m_context) {
		m_context->settings.general.disableParticles = !enable;
	}
}

/*==============================================================================
 * Playback Control
 *============================================================================*/

bool Engine::Play() {
	std::unique_lock<std::mutex> lock(m_mutex);

	if (m_isRunning) {
		// Already running, just unpause if paused
		if (m_isPaused) {
			m_isPaused = false;
			ClearError();
			return true;
		}
		SetLastError("Already playing");
		return false;
	}

	if (!m_project || !m_app) {
		SetLastError("No wallpaper loaded. Call WE_LoadWallpaper first.");
		return false;
	}

	m_isRunning = true;
	m_isPaused = false;

	// Start render thread
	m_renderThread = std::make_unique<std::thread>(&Engine::RenderLoop, this);

	ClearError();
	return true;
}

bool Engine::Pause() {
	if (!m_isRunning) {
		SetLastError("Not playing");
		return false;
	}

	m_isPaused = true;
	ClearError();
	return true;
}

bool Engine::Stop() {
	if (!m_isRunning) {
		// Already stopped
		ClearError();
		return true;
	}

	// Signal the render thread to stop
	m_isRunning = false;

	// Wait for the render thread to finish
	if (m_renderThread && m_renderThread->joinable()) {
		m_renderThread->join();
	}
	m_renderThread.reset();
	m_isPaused = false;

	ClearError();
	return true;
}

bool Engine::IsPlaying() const {
	return m_isRunning.load();
}

bool Engine::IsPaused() const {
	return m_isPaused.load();
}

/*==============================================================================
 * Properties
 *============================================================================*/

std::unique_ptr<PropertyList> Engine::ListProperties() {
	std::unique_lock<std::mutex> lock(m_mutex);

	if (!m_project) {
		SetLastError("No wallpaper loaded");
		return nullptr;
	}

	std::vector<PropertyInfo> properties;

	for (const auto& [name, prop] : m_project->properties) {
		PropertyInfo info;
		info.name = prop->name;
		info.text = prop->text;
		info.type = ConvertPropertyType(prop.get());
		info.value = prop->toString();

		// Get type-specific info
		if (auto* slider = dynamic_cast<const PropertySlider*>(prop.get())) {
			info.minValue = std::to_string(slider->min);
			info.maxValue = std::to_string(slider->max);
			info.step = std::to_string(slider->step);
		} else if (auto* combo = dynamic_cast<const PropertyCombo*>(prop.get())) {
			info.comboOptions = combo->values;
		}

		properties.push_back(std::move(info));
	}

	ClearError();
	return std::make_unique<PropertyList>(std::move(properties));
}

bool Engine::SetProperty(const std::string& name, const std::string& value) {
	std::unique_lock<std::mutex> lock(m_mutex);

	if (!m_project) {
		SetLastError("No wallpaper loaded");
		return false;
	}

	auto it = m_project->properties.find(name);
	if (it == m_project->properties.end()) {
		SetLastError("Property not found: " + name);
		return false;
	}

	try {
		it->second->update(value);
		ClearError();
		return true;
	} catch (const std::exception& e) {
		SetLastError("Failed to set property: " + std::string(e.what()));
		return false;
	}
}

std::optional<std::string> Engine::GetProperty(const std::string& name) {
	std::unique_lock<std::mutex> lock(m_mutex);

	if (!m_project) {
		SetLastError("No wallpaper loaded");
		return std::nullopt;
	}

	auto it = m_project->properties.find(name);
	if (it == m_project->properties.end()) {
		SetLastError("Property not found: " + name);
		return std::nullopt;
	}

	ClearError();
	return it->second->toString();
}

/*==============================================================================
 * Window Control
 *============================================================================*/

void Engine::ShowWindow() {
	// Implementation through VideoDriver
	// This requires access to the underlying driver
}

void Engine::HideWindow() {
	// Implementation through VideoDriver
}

void Engine::ResizeWindow(int x, int y, int width, int height) {
	if (m_context) {
		m_context->settings.render.mode = ApplicationContext::EXPLICIT_WINDOW;
		m_context->settings.render.window.geometry = glm::ivec4(x, y, width, height);
	}
}

/*==============================================================================
 * Audio Control
 *============================================================================*/

void Engine::SetAudioEnabled(bool enable) {
	if (m_context) {
		m_context->settings.audio.enabled = enable;
	}
}

void Engine::SetVolume(int volume) {
	if (m_context) {
		m_context->settings.audio.volume = std::clamp(volume, 0, 128);
	}
}

void Engine::SetMuted(bool mute) {
	if (m_context) {
		m_context->settings.audio.volume = mute ? 0 : 15;
	}
}

void Engine::SetAutoMute(bool enable) {
	if (m_context) {
		m_context->settings.audio.automute = enable;
	}
}

/*==============================================================================
 * Screenshot
 *============================================================================*/

bool Engine::TakeScreenshot(const std::filesystem::path& path) {
	std::unique_lock<std::mutex> lock(m_mutex);

	if (!m_app) {
		SetLastError("No wallpaper loaded");
		return false;
	}

	try {
		m_app->takeScreenshot(path);
		ClearError();
		return true;
	} catch (const std::exception& e) {
		SetLastError("Failed to take screenshot: " + std::string(e.what()));
		return false;
	}
}

/*==============================================================================
 * Error Handling
 *============================================================================*/

const std::string& Engine::GetLastError() const {
	return m_lastError;
}

void Engine::SetLastError(const std::string& error) {
	m_lastError = error;
	sLog.error("Engine error: ", error);
}

void Engine::ClearError() {
	m_lastError.clear();
}

/*==============================================================================
 * Helper Methods
 *============================================================================*/

bool Engine::InitializeContext() {
	// Create a minimal ApplicationContext for DLL usage
	// We use a minimal argc/argv to initialize
	static const char* minimal_argv[] = {"wallpaper-engine"};
	static int argc = 1;

	m_context = std::make_unique<ApplicationContext>(argc, const_cast<char**>(minimal_argv));

	// Set default settings for DLL usage
	m_context->settings.render.mode = ApplicationContext::NORMAL_WINDOW;
	m_context->settings.render.maximumFPS = m_maxFPS;
	m_context->settings.audio.enabled = true;
	m_context->settings.audio.volume = 15;
	m_context->settings.audio.automute = false;
	m_context->settings.mouse.enabled = true;
	m_context->settings.mouse.disableparallax = false;

	return true;
}

bool Engine::InitializeApplication() {
	if (!m_context) {
		SetLastError("Context not initialized");
		return false;
	}

	try {
		// Create the application
		// Note: WallpaperApplication constructor calls loadBackgrounds() which
		// will parse the project.json again using the context settings
		m_app = std::make_unique<WallpaperApplication>(*m_context);

		// Store the project for property access (keeping our parsed version)
		// The app has its own copy from loadBackgrounds(), but we keep ours for API access

		// Setup properties
		m_app->setupProperties();

		// Setup output (video driver, fullscreen detector)
		m_app->setupOutput();

		// Setup audio
		m_app->setupAudio();

		// Prepare outputs (create wallpapers)
		m_app->prepareOutputs();

		ClearError();
		return true;

	} catch (const std::exception& e) {
		SetLastError("Failed to initialize application: " + std::string(e.what()));
		m_app.reset();
		return false;
	}
}

bool Engine::SetupViewport() {
	// Setup viewport for windowed mode
	// This is handled by the VideoDriver based on context settings
	return true;
}

void Engine::RenderLoop() {
	// Initialize time
	g_Time = 0.0f;
	g_TimeLast = 0.0f;

	// Simple timing for render loop
	static auto startTime = std::chrono::steady_clock::now();

	while (m_isRunning) {
		if (m_isPaused) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		try {
			// Update time
			g_TimeLast = g_Time;

			// Calculate time using simple timing
			auto now = std::chrono::steady_clock::now();
			g_Time = std::chrono::duration<float>(now - startTime).count();

			// Update day time
			time_t seconds;
			struct tm* timeinfo;
			time(&seconds);
			timeinfo = localtime(&seconds);
			g_Daytime = static_cast<float>((timeinfo->tm_hour * 60) + timeinfo->tm_min) / (24.0f * 60.0f);

			// Render frame (this handles audio updates internally)
			m_app->render();

			// FPS limiting
			if (m_maxFPS > 0) {
				const float minFrameTime = 1.0f / m_maxFPS;
				// Simple sleep-based FPS limiting
				std::this_thread::sleep_for(std::chrono::duration<float>(minFrameTime * 0.9f));
			}

		} catch (const std::exception& e) {
			SetLastError("Render loop error: " + std::string(e.what()));
			m_isRunning = false;
		}
	}
}

PropertyType Engine::ConvertPropertyType(const Property* prop) {
	if (dynamic_cast<const PropertySlider*>(prop)) {
		return PropertyType::Slider;
	} else if (dynamic_cast<const PropertyBoolean*>(prop)) {
		return PropertyType::Boolean;
	} else if (dynamic_cast<const PropertyColor*>(prop)) {
		return PropertyType::Color;
	} else if (dynamic_cast<const PropertyCombo*>(prop)) {
		return PropertyType::Combo;
	} else if (dynamic_cast<const PropertyText*>(prop)) {
		return PropertyType::Text;
	} else if (dynamic_cast<const PropertyTextInput*>(prop)) {
		return PropertyType::TextInput;
	} else if (dynamic_cast<const PropertySceneTexture*>(prop)) {
		return PropertyType::SceneTexture;
	} else if (dynamic_cast<const PropertyFile*>(prop)) {
		return PropertyType::File;
	}
	return PropertyType::Unknown;
}

Project* Engine::GetProject() {
	return m_project.get();
}
