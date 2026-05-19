#include "WallpaperEngine/Engine/Engine.h"
#include "engine.h"

#include <cstring>
#include <map>
#include <string>

using namespace WallpaperEngine::Engine;

/*==============================================================================
 * Internal Helpers
 *============================================================================*/

namespace {

// Convert WE_PropertyType to PropertyType
PropertyType ConvertPropertyType(WE_PropertyType type) {
	switch (type) {
		case WE_PROPERTY_SLIDER: return PropertyType::Slider;
		case WE_PROPERTY_BOOLEAN: return PropertyType::Boolean;
		case WE_PROPERTY_COLOR: return PropertyType::Color;
		case WE_PROPERTY_COMBO: return PropertyType::Combo;
		case WE_PROPERTY_TEXT: return PropertyType::Text;
		case WE_PROPERTY_TEXTINPUT: return PropertyType::TextInput;
		case WE_PROPERTY_SCENE_TEXTURE: return PropertyType::SceneTexture;
		case WE_PROPERTY_FILE: return PropertyType::File;
		default: return PropertyType::Unknown;
	}
}

// Convert PropertyType to WE_PropertyType
WE_PropertyType ConvertPropertyType(PropertyType type) {
	switch (type) {
		case PropertyType::Slider: return WE_PROPERTY_SLIDER;
		case PropertyType::Boolean: return WE_PROPERTY_BOOLEAN;
		case PropertyType::Color: return WE_PROPERTY_COLOR;
		case PropertyType::Combo: return WE_PROPERTY_COMBO;
		case PropertyType::Text: return WE_PROPERTY_TEXT;
		case PropertyType::TextInput: return WE_PROPERTY_TEXTINPUT;
		case PropertyType::SceneTexture: return WE_PROPERTY_SCENE_TEXTURE;
		case PropertyType::File: return WE_PROPERTY_FILE;
		default: return WE_PROPERTY_UNKNOWN;
	}
}

} // anonymous namespace

/*==============================================================================
 * Initialization/Cleanup
 *============================================================================*/

extern "C" WE_API WE_Engine* WE_Create(void) {
	try {
		return reinterpret_cast<WE_Engine*>(new Engine());
	} catch (const std::exception& e) {
		return nullptr;
	}
}

extern "C" WE_API void WE_Destroy(WE_Engine* engine) {
	if (engine) {
		delete reinterpret_cast<Engine*>(engine);
	}
}

/*==============================================================================
 * Configuration
 *============================================================================*/

extern "C" WE_API bool WE_SetAssetsPath(WE_Engine* engine, const char* path) {
	if (!engine || !path) return false;
	auto* e = reinterpret_cast<Engine*>(engine);
	return e->SetAssetsPath(path);
}

extern "C" WE_API bool WE_LoadWallpaper(WE_Engine* engine, const char* path) {
	if (!engine || !path) return false;
	auto* e = reinterpret_cast<Engine*>(engine);
	return e->LoadWallpaper(path);
}

extern "C" WE_API void WE_SetMaxFPS(WE_Engine* engine, int fps) {
	if (!engine) return;
	auto* e = reinterpret_cast<Engine*>(engine);
	e->SetMaxFPS(fps);
}

extern "C" WE_API void WE_SetScalingMode(WE_Engine* engine, WE_ScalingMode mode) {
	if (!engine) return;
	auto* e = reinterpret_cast<Engine*>(engine);
	using ScalingMode = WallpaperEngine::Render::WallpaperState::TextureUVsScaling;
	switch (mode) {
		case WE_SCALING_DEFAULT:
			e->SetScalingMode(ScalingMode::DefaultUVs);
			break;
		case WE_SCALING_STRETCH:
			e->SetScalingMode(ScalingMode::StretchUVs);
			break;
		case WE_SCALING_FIT:
			e->SetScalingMode(ScalingMode::ZoomFitUVs);
			break;
		case WE_SCALING_FILL:
			e->SetScalingMode(ScalingMode::ZoomFillUVs);
			break;
	}
}

extern "C" WE_API void WE_SetClampMode(WE_Engine* engine, WE_ClampMode mode) {
	if (!engine) return;
	auto* e = reinterpret_cast<Engine*>(engine);
	using namespace WallpaperEngine::Data::Assets;
	switch (mode) {
		case WE_CLAMP_DEFAULT:
			e->SetClampMode(TextureFlags_ClampUVs);
			break;
		case WE_CLAMP_CLAMP:
			e->SetClampMode(TextureFlags_ClampUVs);
			break;
		case WE_CLAMP_BORDER:
			e->SetClampMode(TextureFlags_BorderUVs);
			break;
		case WE_CLAMP_REPEAT:
			e->SetClampMode(TextureFlags_RepeatUVs);
			break;
	}
}

extern "C" WE_API void WE_SetParticlesEnabled(WE_Engine* engine, bool enable) {
	if (!engine) return;
	auto* e = reinterpret_cast<Engine*>(engine);
	e->SetParticlesEnabled(enable);
}

/*==============================================================================
 * Playback Control
 *============================================================================*/

extern "C" WE_API bool WE_Play(WE_Engine* engine) {
	if (!engine) return false;
	auto* e = reinterpret_cast<Engine*>(engine);
	return e->Play();
}

extern "C" WE_API bool WE_Pause(WE_Engine* engine) {
	if (!engine) return false;
	auto* e = reinterpret_cast<Engine*>(engine);
	return e->Pause();
}

extern "C" WE_API bool WE_Stop(WE_Engine* engine) {
	if (!engine) return false;
	auto* e = reinterpret_cast<Engine*>(engine);
	return e->Stop();
}

extern "C" WE_API bool WE_IsPlaying(WE_Engine* engine) {
	if (!engine) return false;
	auto* e = reinterpret_cast<Engine*>(engine);
	return e->IsPlaying();
}

extern "C" WE_API bool WE_IsPaused(WE_Engine* engine) {
	if (!engine) return false;
	auto* e = reinterpret_cast<Engine*>(engine);
	return e->IsPaused();
}

/*==============================================================================
 * Properties
 *============================================================================*/

extern "C" WE_API WE_PropertyList* WE_ListProperties(WE_Engine* engine) {
	if (!engine) return nullptr;
	auto* e = reinterpret_cast<Engine*>(engine);
	auto* list = e->ListProperties().release();
	return reinterpret_cast<WE_PropertyList*>(list);
}

extern "C" WE_API int WE_PropertyList_GetCount(WE_PropertyList* list) {
	if (!list) return 0;
	auto* l = reinterpret_cast<PropertyList*>(list);
	return static_cast<int>(l->properties.size());
}

extern "C" WE_API const char* WE_PropertyList_GetName(WE_PropertyList* list, int index) {
	if (!list || index < 0) return nullptr;
	auto* l = reinterpret_cast<PropertyList*>(list);
	if (index >= static_cast<int>(l->properties.size())) return nullptr;
	return l->properties[index].name.c_str();
}

extern "C" WE_API const char* WE_PropertyList_GetText(WE_PropertyList* list, int index) {
	if (!list || index < 0) return nullptr;
	auto* l = reinterpret_cast<PropertyList*>(list);
	if (index >= static_cast<int>(l->properties.size())) return nullptr;
	return l->properties[index].text.c_str();
}

extern "C" WE_API WE_PropertyType WE_PropertyList_GetType(WE_PropertyList* list, int index) {
	if (!list || index < 0) return WE_PROPERTY_UNKNOWN;
	auto* l = reinterpret_cast<PropertyList*>(list);
	if (index >= static_cast<int>(l->properties.size())) return WE_PROPERTY_UNKNOWN;
	return ConvertPropertyType(l->properties[index].type);
}

extern "C" WE_API const char* WE_PropertyList_GetValue(WE_PropertyList* list, int index) {
	if (!list || index < 0) return nullptr;
	auto* l = reinterpret_cast<PropertyList*>(list);
	if (index >= static_cast<int>(l->properties.size())) return nullptr;
	return l->properties[index].value.c_str();
}

extern "C" WE_API const char* WE_PropertyList_GetMinValue(WE_PropertyList* list, int index) {
	if (!list || index < 0) return nullptr;
	auto* l = reinterpret_cast<PropertyList*>(list);
	if (index >= static_cast<int>(l->properties.size())) return nullptr;
	if (l->properties[index].type != PropertyType::Slider) return nullptr;
	return l->properties[index].minValue.c_str();
}

extern "C" WE_API const char* WE_PropertyList_GetMaxValue(WE_PropertyList* list, int index) {
	if (!list || index < 0) return nullptr;
	auto* l = reinterpret_cast<PropertyList*>(list);
	if (index >= static_cast<int>(l->properties.size())) return nullptr;
	if (l->properties[index].type != PropertyType::Slider) return nullptr;
	return l->properties[index].maxValue.c_str();
}

extern "C" WE_API const char* WE_PropertyList_GetStepValue(WE_PropertyList* list, int index) {
	if (!list || index < 0) return nullptr;
	auto* l = reinterpret_cast<PropertyList*>(list);
	if (index >= static_cast<int>(l->properties.size())) return nullptr;
	if (l->properties[index].type != PropertyType::Slider) return nullptr;
	return l->properties[index].step.c_str();
}

extern "C" WE_API int WE_PropertyList_GetComboOptionCount(WE_PropertyList* list, int index) {
	if (!list || index < 0) return 0;
	auto* l = reinterpret_cast<PropertyList*>(list);
	if (index >= static_cast<int>(l->properties.size())) return 0;
	if (l->properties[index].type != PropertyType::Combo) return 0;
	return static_cast<int>(l->properties[index].comboOptions.size());
}

extern "C" WE_API const char* WE_PropertyList_GetComboOptionKey(WE_PropertyList* list, int index, int optionIndex) {
	if (!list || index < 0 || optionIndex < 0) return nullptr;
	auto* l = reinterpret_cast<PropertyList*>(list);
	if (index >= static_cast<int>(l->properties.size())) return nullptr;
	if (l->properties[index].type != PropertyType::Combo) return nullptr;

	const auto& options = l->properties[index].comboOptions;
	if (optionIndex >= static_cast<int>(options.size())) return nullptr;

	auto it = options.begin();
	std::advance(it, optionIndex);
	return it->first.c_str();
}

extern "C" WE_API const char* WE_PropertyList_GetComboOptionValue(WE_PropertyList* list, int index, int optionIndex) {
	if (!list || index < 0 || optionIndex < 0) return nullptr;
	auto* l = reinterpret_cast<PropertyList*>(list);
	if (index >= static_cast<int>(l->properties.size())) return nullptr;
	if (l->properties[index].type != PropertyType::Combo) return nullptr;

	const auto& options = l->properties[index].comboOptions;
	if (optionIndex >= static_cast<int>(options.size())) return nullptr;

	auto it = options.begin();
	std::advance(it, optionIndex);
	return it->second.c_str();
}

extern "C" WE_API void WE_PropertyList_Destroy(WE_PropertyList* list) {
	if (list) {
		delete reinterpret_cast<PropertyList*>(list);
	}
}

extern "C" WE_API bool WE_SetProperty(WE_Engine* engine, const char* name, const char* value) {
	if (!engine || !name || !value) return false;
	auto* e = reinterpret_cast<Engine*>(engine);
	return e->SetProperty(name, value);
}

extern "C" WE_API const char* WE_GetProperty(WE_Engine* engine, const char* name) {
	if (!engine || !name) return nullptr;
	auto* e = reinterpret_cast<Engine*>(engine);
	static thread_local std::string value;
	if (auto result = e->GetProperty(name)) {
		value = *result;
		return value.c_str();
	}
	return nullptr;
}

/*==============================================================================
 * Window Control
 *============================================================================*/

extern "C" WE_API void WE_ShowWindow(WE_Engine* engine) {
	if (!engine) return;
	auto* e = reinterpret_cast<Engine*>(engine);
	e->ShowWindow();
}

extern "C" WE_API void WE_HideWindow(WE_Engine* engine) {
	if (!engine) return;
	auto* e = reinterpret_cast<Engine*>(engine);
	e->HideWindow();
}

extern "C" WE_API void WE_ResizeWindow(WE_Engine* engine, int x, int y, int width, int height) {
	if (!engine) return;
	auto* e = reinterpret_cast<Engine*>(engine);
	e->ResizeWindow(x, y, width, height);
}

/*==============================================================================
 * Audio Control
 *============================================================================*/

extern "C" WE_API void WE_SetAudioEnabled(WE_Engine* engine, bool enable) {
	if (!engine) return;
	auto* e = reinterpret_cast<Engine*>(engine);
	e->SetAudioEnabled(enable);
}

extern "C" WE_API void WE_SetVolume(WE_Engine* engine, int volume) {
	if (!engine) return;
	auto* e = reinterpret_cast<Engine*>(engine);
	e->SetVolume(volume);
}

extern "C" WE_API void WE_SetMuted(WE_Engine* engine, bool mute) {
	if (!engine) return;
	auto* e = reinterpret_cast<Engine*>(engine);
	e->SetMuted(mute);
}

extern "C" WE_API void WE_SetAutoMute(WE_Engine* engine, bool enable) {
	if (!engine) return;
	auto* e = reinterpret_cast<Engine*>(engine);
	e->SetAutoMute(enable);
}

/*==============================================================================
 * Screenshot
 *============================================================================*/

extern "C" WE_API bool WE_TakeScreenshot(WE_Engine* engine, const char* path) {
	if (!engine || !path) return false;
	auto* e = reinterpret_cast<Engine*>(engine);
	return e->TakeScreenshot(path);
}

/*==============================================================================
 * Error Handling
 *============================================================================*/

extern "C" WE_API const char* WE_GetLastError(WE_Engine* engine) {
	if (!engine) return "Invalid engine handle";
	auto* e = reinterpret_cast<Engine*>(engine);
	return e->GetLastError().c_str();
}

extern "C" WE_API void WE_ClearError(WE_Engine* engine) {
	if (!engine) return;
	auto* e = reinterpret_cast<Engine*>(engine);
	e->ClearError();
}
