/**
 * @file demo_win32.cpp
 * @brief Demo Win32 application showcasing the Wallpaper Engine DLL API
 *
 * This application demonstrates:
 * - DLL initialization and cleanup
 * - Loading a wallpaper from path
 * - Listing wallpaper properties
 * - Modifying properties at runtime
 * - Playback control (Play/Pause/Stop)
 * - Clean shutdown
 */

#include <engine.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

// Helper function to print property type as string
const char* PropertyTypeToString(WE_PropertyType type) {
	switch (type) {
		case WE_PROPERTY_SLIDER: return "Slider";
		case WE_PROPERTY_BOOLEAN: return "Boolean";
		case WE_PROPERTY_COLOR: return "Color";
		case WE_PROPERTY_COMBO: return "Combo";
		case WE_PROPERTY_TEXT: return "Text";
		case WE_PROPERTY_TEXTINPUT: return "TextInput";
		case WE_PROPERTY_SCENE_TEXTURE: return "SceneTexture";
		case WE_PROPERTY_FILE: return "File";
		default: return "Unknown";
	}
}

// Demo: List all properties
void DemoListProperties(WE_Engine* engine) {
	std::cout << "\n=== Listing Properties ===" << std::endl;

	WE_PropertyList* list = WE_ListProperties(engine);
	if (!list) {
		std::cerr << "Failed to list properties: " << WE_GetLastError(engine) << std::endl;
		return;
	}

	const int count = WE_PropertyList_GetCount(list);
	std::cout << "Found " << count << " properties:" << std::endl;

	for (int i = 0; i < count; ++i) {
		const char* name = WE_PropertyList_GetName(list, i);
		const char* text = WE_PropertyList_GetText(list, i);
		WE_PropertyType type = WE_PropertyList_GetType(list, i);
		const char* value = WE_PropertyList_GetValue(list, i);

		std::cout << "\n  [" << i << "] " << name << " (" << PropertyTypeToString(type) << ")" << std::endl;
		std::cout << "      Description: " << text << std::endl;
		std::cout << "      Value: " << value << std::endl;

		// Show additional info based on type
		if (type == WE_PROPERTY_SLIDER) {
			const char* min = WE_PropertyList_GetMinValue(list, i);
			const char* max = WE_PropertyList_GetMaxValue(list, i);
			const char* step = WE_PropertyList_GetStepValue(list, i);
			if (min && max && step) {
				std::cout << "      Range: [" << min << ", " << max << "] Step: " << step << std::endl;
			}
		} else if (type == WE_PROPERTY_COMBO) {
			const int optionCount = WE_PropertyList_GetComboOptionCount(list, i);
			std::cout << "      Options: " << optionCount << std::endl;
			for (int j = 0; j < optionCount; ++j) {
				const char* key = WE_PropertyList_GetComboOptionKey(list, i, j);
				const char* optValue = WE_PropertyList_GetComboOptionValue(list, i, j);
				std::cout << "        " << key << " = " << optValue << std::endl;
			}
		}
	}

	WE_PropertyList_Destroy(list);
}

// Demo: Modify a property
void DemoSetProperty(WE_Engine* engine, const char* name, const char* value) {
	std::cout << "\n=== Setting Property ===" << std::endl;
	std::cout << "Setting " << name << " = " << value << std::endl;

	if (WE_SetProperty(engine, name, value)) {
		std::cout << "Property set successfully!" << std::endl;
	} else {
		std::cerr << "Failed to set property: " << WE_GetLastError(engine) << std::endl;
	}
}

// Demo: Interactive property modification
void DemoInteractiveProperties(WE_Engine* engine) {
	std::cout << "\n=== Interactive Property Modification ===" << std::endl;
	std::cout << "Enter property name and new value (format: name=value)" << std::endl;
	std::cout << "Press Enter with empty line to continue..." << std::endl;

	std::string line;
	while (std::getline(std::cin, line) && !line.empty()) {
		const size_t eqPos = line.find('=');
		if (eqPos != std::string::npos) {
			std::string name = line.substr(0, eqPos);
			std::string value = line.substr(eqPos + 1);
			DemoSetProperty(engine, name.c_str(), value.c_str());
		} else {
			std::cout << "Invalid format. Use: name=value" << std::endl;
		}
		std::cout << "\nEnter next property (or empty line to continue): ";
	}
}

// Demo: Basic playback control
void DemoPlaybackControl(WE_Engine* engine) {
	std::cout << "\n=== Playback Control Demo ===" << std::endl;

	// Start playing
	std::cout << "Starting playback..." << std::endl;
	if (!WE_Play(engine)) {
		std::cerr << "Failed to start playback: " << WE_GetLastError(engine) << std::endl;
		return;
	}
	std::cout << "Playing! IsPlaying: " << (WE_IsPlaying(engine) ? "Yes" : "No") << std::endl;

	// Let it run for a few seconds
	std::cout << "Running for 3 seconds..." << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(3));

	// Pause
	std::cout << "Pausing..." << std::endl;
	if (WE_Pause(engine)) {
		std::cout << "Paused! IsPaused: " << (WE_IsPaused(engine) ? "Yes" : "No") << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}

	// Resume
	std::cout << "Resuming..." << std::endl;
	if (WE_Play(engine)) {
		std::cout << "Resumed!" << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}
}

// Demo: Window control
void DemoWindowControl(WE_Engine* engine) {
	std::cout << "\n=== Window Control Demo ===" << std::endl;

	// Show window
	std::cout << "Showing window..." << std::endl;
	WE_ShowWindow(engine);
	std::this_thread::sleep_for(std::chrono::seconds(2));

	// Resize window
	std::cout << "Resizing window to 1280x720 at position 100,100..." << std::endl;
	WE_ResizeWindow(engine, 100, 100, 1280, 720);
	std::this_thread::sleep_for(std::chrono::seconds(2));

	// Hide window
	std::cout << "Hiding window..." << std::endl;
	WE_HideWindow(engine);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	// Show again
	std::cout << "Showing window again..." << std::endl;
	WE_ShowWindow(engine);
}

// Demo: Screenshot
void DemoScreenshot(WE_Engine* engine) {
	std::cout << "\n=== Screenshot Demo ===" << std::endl;
	std::cout << "Taking screenshot..." << std::endl;

	if (WE_TakeScreenshot(engine, "screenshot.png")) {
		std::cout << "Screenshot saved to screenshot.png" << std::endl;
	} else {
		std::cerr << "Failed to take screenshot: " << WE_GetLastError(engine) << std::endl;
	}
}

// Demo: Audio control
void DemoAudioControl(WE_Engine* engine) {
	std::cout << "\n=== Audio Control Demo ===" << std::endl;

	// Set volume
	std::cout << "Setting volume to 64..." << std::endl;
	WE_SetVolume(engine, 64);

	// Enable audio
	std::cout << "Enabling audio..." << std::endl;
	WE_SetAudioEnabled(engine, true);

	std::this_thread::sleep_for(std::chrono::seconds(2));

	// Mute
	std::cout << "Muting audio..." << std::endl;
	WE_SetMuted(engine, true);

	std::this_thread::sleep_for(std::chrono::seconds(2));

	// Unmute
	std::cout << "Unmuting audio..." << std::endl;
	WE_SetMuted(engine, false);
}

//==============================================================================
// Main Entry Point
//==============================================================================

int main(int argc, char* argv[]) {
	std::cout << "====================================" << std::endl;
	std::cout << "Wallpaper Engine DLL Demo" << std::endl;
	std::cout << "====================================" << std::endl;

	//==========================================================================
	// Parse command line arguments
	//==========================================================================

	if (argc < 3) {
		std::cout << "\nUsage: " << argv[0] << " <assets_path> <wallpaper_path> [options]" << std::endl;
		std::cout << "\nArguments:" << std::endl;
		std::cout << "  assets_path    - Path to Wallpaper Engine assets directory" << std::endl;
		std::cout << "  wallpaper_path - Path to the wallpaper directory" << std::endl;
		std::cout << "\nOptions:" << std::endl;
		std::cout << "  --list         - List all properties and exit" << std::endl;
		std::cout << "  --interactive  - Interactive property modification mode" << std::endl;
		std::cout << "  --set <n>=<v>  - Set property name=value (can be used multiple times)" << std::endl;
		std::cout << "  --window       - Enable window control demo" << std::endl;
		std::cout << "  --audio        - Enable audio control demo" << std::endl;
		std::cout << "  --screenshot   - Take a screenshot" << std::endl;
		std::cout << "  --fps <n>      - Set maximum FPS" << std::endl;
		std::cout << "\nExample:" << std::endl;
		std::cout << "  " << argv[0] << " C:/Assets/WallpaperEngine C:/Wallpapers/1845706469 --fps 30" << std::endl;
		return 1;
	}

	const char* assetsPath = argv[1];
	const char* wallpaperPath = argv[2];

	// Parse options
	bool listOnly = false;
	bool interactiveMode = false;
	bool windowDemo = false;
	bool audioDemo = false;
	bool screenshotDemo = false;
	int maxFPS = 30;
	std::vector<std::pair<std::string, std::string>> propertiesToSet;

	for (int i = 3; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "--list") {
			listOnly = true;
		} else if (arg == "--interactive") {
			interactiveMode = true;
		} else if (arg == "--window") {
			windowDemo = true;
		} else if (arg == "--audio") {
			audioDemo = true;
		} else if (arg == "--screenshot") {
			screenshotDemo = true;
		} else if (arg == "--fps" && i + 1 < argc) {
			maxFPS = std::atoi(argv[++i]);
		} else if (arg == "--set" && i + 1 < argc) {
			std::string prop = argv[++i];
			const size_t eqPos = prop.find('=');
			if (eqPos != std::string::npos) {
				propertiesToSet.push_back({prop.substr(0, eqPos), prop.substr(eqPos + 1)});
			}
		}
	}

	//==========================================================================
	// Initialize Engine
	//==========================================================================

	std::cout << "\n=== Initializing Engine ===" << std::endl;
	WE_Engine* engine = WE_Create();
	if (!engine) {
		std::cerr << "Failed to create engine!" << std::endl;
		return 1;
	}
	std::cout << "Engine created successfully!" << std::endl;

	//==========================================================================
	// Configuration
	//==========================================================================

	std::cout << "\n=== Configuration ===" << std::endl;
	std::cout << "Assets path: " << assetsPath << std::endl;
	std::cout << "Wallpaper path: " << wallpaperPath << std::endl;

	if (!WE_SetAssetsPath(engine, assetsPath)) {
		std::cerr << "Failed to set assets path: " << WE_GetLastError(engine) << std::endl;
		WE_Destroy(engine);
		return 1;
	}
	std::cout << "Assets path set!" << std::endl;

	WE_SetMaxFPS(engine, maxFPS);
	std::cout << "Max FPS set to: " << maxFPS << std::endl;

	// Set scaling mode
	WE_SetScalingMode(engine, WE_SCALING_FILL);
	WE_SetClampMode(engine, WE_CLAMP_CLAMP);

	//==========================================================================
	// Load Wallpaper
	//==========================================================================

	std::cout << "\n=== Loading Wallpaper ===" << std::endl;
	if (!WE_LoadWallpaper(engine, wallpaperPath)) {
		std::cerr << "Failed to load wallpaper: " << WE_GetLastError(engine) << std::endl;
		WE_Destroy(engine);
		return 1;
	}
	std::cout << "Wallpaper loaded successfully!" << std::endl;

	//==========================================================================
	// List Properties
	//==========================================================================

	DemoListProperties(engine);

	if (listOnly) {
		std::cout << "\n--list specified, exiting..." << std::endl;
		WE_Destroy(engine);
		return 0;
	}

	//==========================================================================
	// Set Properties (if specified)
	//==========================================================================

	for (const auto& [name, value] : propertiesToSet) {
		DemoSetProperty(engine, name.c_str(), value.c_str());
	}

	//==========================================================================
	// Interactive Mode (if specified)
	//==========================================================================

	if (interactiveMode) {
		DemoInteractiveProperties(engine);
	}

	//==========================================================================
	// Start Playback
	//==========================================================================

	std::cout << "\n=== Starting Playback ===" << std::endl;
	if (!WE_Play(engine)) {
		std::cerr << "Failed to start playback: " << WE_GetLastError(engine) << std::endl;
		WE_Destroy(engine);
		return 1;
	}
	std::cout << "Playback started!" << std::endl;

	//==========================================================================
	// Run Demos
	//==========================================================================

	if (windowDemo) {
		DemoWindowControl(engine);
	}

	if (audioDemo) {
		DemoAudioControl(engine);
	}

	if (screenshotDemo) {
		DemoScreenshot(engine);
	}

	//==========================================================================
	// Keep Running
	//==========================================================================

	std::cout << "\n=== Running ===" << std::endl;
	std::cout << "Press Ctrl+C to stop..." << std::endl;

	// Keep running until interrupted
	while (WE_IsPlaying(engine)) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	//==========================================================================
	// Cleanup
	//==========================================================================

	std::cout << "\n=== Shutting Down ===" << std::endl;
	WE_Stop(engine);
	std::cout << "Stopped!" << std::endl;

	WE_Destroy(engine);
	std::cout << "Engine destroyed!" << std::endl;

	std::cout << "\n=== Demo Complete ===" << std::endl;
	return 0;
}
