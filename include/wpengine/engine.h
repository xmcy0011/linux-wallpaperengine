#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// Export macro - must be defined before including this header in the DLL build
#ifndef WE_API
#ifdef _WIN32
	#define WE_API __declspec(dllimport)
#else
	#define WE_API
#endif
#endif

// Opaque handle types
typedef struct WE_Engine WE_Engine;
typedef struct WE_PropertyList WE_PropertyList;

// Property types
typedef enum {
	WE_PROPERTY_UNKNOWN = 0,
	WE_PROPERTY_SLIDER,
	WE_PROPERTY_BOOLEAN,
	WE_PROPERTY_COLOR,
	WE_PROPERTY_COMBO,
	WE_PROPERTY_TEXT,
	WE_PROPERTY_TEXTINPUT,
	WE_PROPERTY_SCENE_TEXTURE,
	WE_PROPERTY_FILE
} WE_PropertyType;

// Texture scaling modes
typedef enum {
	WE_SCALING_DEFAULT = 0,
	WE_SCALING_STRETCH,
	WE_SCALING_FIT,
	WE_SCALING_FILL
} WE_ScalingMode;

// Texture clamping modes
typedef enum {
	WE_CLAMP_DEFAULT = 0,
	WE_CLAMP_CLAMP,
	WE_CLAMP_BORDER,
	WE_CLAMP_REPEAT
} WE_ClampMode;

/*==============================================================================
 * Initialization/Cleanup
 *============================================================================*/

/// @brief Create a new wallpaper engine instance
/// @return Handle to the engine, or NULL on failure
WE_API WE_Engine* WE_Create(void);

/// @brief Destroy a wallpaper engine instance
/// @param engine Engine handle
WE_API void WE_Destroy(WE_Engine* engine);

/*==============================================================================
 * Configuration
 *============================================================================*/

/// @brief Set the assets directory path (required before loading wallpapers)
/// @param engine Engine handle
/// @param path Path to the assets directory (e.g., Steam Wallpaper Engine assets)
/// @return true on success, false on failure
WE_API bool WE_SetAssetsPath(WE_Engine* engine, const char* path);

/// @brief Load a wallpaper from a file path
/// @param engine Engine handle
/// @param path Path to the wallpaper directory or project file
/// @return true on success, false on failure
WE_API bool WE_LoadWallpaper(WE_Engine* engine, const char* path);

/// @brief Set the maximum FPS for rendering
/// @param engine Engine handle
/// @param fps Maximum FPS (0 = unlimited)
WE_API void WE_SetMaxFPS(WE_Engine* engine, int fps);

/// @brief Set the texture scaling mode
/// @param engine Engine handle
/// @param mode Scaling mode
WE_API void WE_SetScalingMode(WE_Engine* engine, WE_ScalingMode mode);

/// @brief Set the texture clamping mode
/// @param engine Engine handle
/// @param mode Clamping mode
WE_API void WE_SetClampMode(WE_Engine* engine, WE_ClampMode mode);

/// @brief Enable or disable particles
/// @param engine Engine handle
/// @param enable true to enable, false to disable
WE_API void WE_SetParticlesEnabled(WE_Engine* engine, bool enable);

/*==============================================================================
 * Playback Control
 *============================================================================*/

/// @brief Start the wallpaper rendering (starts render thread)
/// @param engine Engine handle
/// @return true on success, false on failure
WE_API bool WE_Play(WE_Engine* engine);

/// @brief Pause the wallpaper rendering (keeps thread alive)
/// @param engine Engine handle
/// @return true on success, false on failure
WE_API bool WE_Pause(WE_Engine* engine);

/// @brief Stop the wallpaper rendering (stops render thread)
/// @param engine Engine handle
/// @return true on success, false on failure
WE_API bool WE_Stop(WE_Engine* engine);

/// @brief Check if the wallpaper is currently playing
/// @param engine Engine handle
/// @return true if playing, false otherwise
WE_API bool WE_IsPlaying(WE_Engine* engine);

/// @brief Check if the wallpaper is currently paused
/// @param engine Engine handle
/// @return true if paused, false otherwise
WE_API bool WE_IsPaused(WE_Engine* engine);

/*==============================================================================
 * Properties
 *============================================================================*/

/// @brief List all properties of the loaded wallpaper
/// @param engine Engine handle
/// @return Property list handle, or NULL on failure (call WE_PropertyList_Destroy when done)
WE_API WE_PropertyList* WE_ListProperties(WE_Engine* engine);

/// @brief Get the number of properties in the list
/// @param list Property list handle
/// @return Number of properties
WE_API int WE_PropertyList_GetCount(WE_PropertyList* list);

/// @brief Get the name of a property
/// @param list Property list handle
/// @param index Property index (0 to count-1)
/// @return Property name, or NULL if index is invalid
WE_API const char* WE_PropertyList_GetName(WE_PropertyList* list, int index);

/// @brief Get the display text of a property
/// @param list Property list handle
/// @param index Property index (0 to count-1)
/// @return Display text, or NULL if index is invalid
WE_API const char* WE_PropertyList_GetText(WE_PropertyList* list, int index);

/// @brief Get the type of a property
/// @param list Property list handle
/// @param index Property index (0 to count-1)
/// @return Property type
WE_API WE_PropertyType WE_PropertyList_GetType(WE_PropertyList* list, int index);

/// @brief Get the current value of a property as a string
/// @param list Property list handle
/// @param index Property index (0 to count-1)
/// @return Property value, or NULL if index is invalid
WE_API const char* WE_PropertyList_GetValue(WE_PropertyList* list, int index);

/// @brief Get the minimum value of a slider property
/// @param list Property list handle
/// @param index Property index (0 to count-1)
/// @return Minimum value as string, or NULL if not a slider or index invalid
WE_API const char* WE_PropertyList_GetMinValue(WE_PropertyList* list, int index);

/// @brief Get the maximum value of a slider property
/// @param list Property list handle
/// @param index Property index (0 to count-1)
/// @return Maximum value as string, or NULL if not a slider or index invalid
WE_API const char* WE_PropertyList_GetMaxValue(WE_PropertyList* list, int index);

/// @brief Get the step value of a slider property
/// @param list Property list handle
/// @param index Property index (0 to count-1)
/// @return Step value as string, or NULL if not a slider or index invalid
WE_API const char* WE_PropertyList_GetStepValue(WE_PropertyList* list, int index);

/// @brief Get the number of options in a combo property
/// @param list Property list handle
/// @param index Property index (0 to count-1)
/// @return Number of options, or 0 if not a combo or index invalid
WE_API int WE_PropertyList_GetComboOptionCount(WE_PropertyList* list, int index);

/// @brief Get a combo option key at a given index
/// @param list Property list handle
/// @param index Property index (0 to count-1)
/// @param optionIndex Option index (0 to option count-1)
/// @return Option key, or NULL if not a combo or indices invalid
WE_API const char* WE_PropertyList_GetComboOptionKey(WE_PropertyList* list, int index, int optionIndex);

/// @brief Get a combo option value at a given index
/// @param list Property list handle
/// @param index Property index (0 to count-1)
/// @param optionIndex Option index (0 to option count-1)
/// @return Option value, or NULL if not a combo or indices invalid
WE_API const char* WE_PropertyList_GetComboOptionValue(WE_PropertyList* list, int index, int optionIndex);

/// @brief Destroy a property list
/// @param list Property list handle
WE_API void WE_PropertyList_Destroy(WE_PropertyList* list);

/// @brief Set a property value
/// @param engine Engine handle
/// @param name Property name
/// @param value New value (as string)
/// @return true on success, false on failure
WE_API bool WE_SetProperty(WE_Engine* engine, const char* name, const char* value);

/// @brief Get a property value
/// @param engine Engine handle
/// @param name Property name
/// @return Property value, or NULL if not found
WE_API const char* WE_GetProperty(WE_Engine* engine, const char* name);

/*==============================================================================
 * Window Control (for preview/multi-window setup)
 *============================================================================*/

/// @brief Set the parent window handle for embedding
/// @param engine Engine handle
/// @param hwnd Native window handle (HWND on Windows, Window on X11, etc.)
/// @note Must be called before WE_Play() to take effect
///       On Windows: pass the HWND of the parent window
///       On X11: pass the Window ID of the parent window
WE_API void WE_SetWindowHandle(WE_Engine* engine, void* hwnd);

/// @brief Get the current window handle
/// @param engine Engine handle
/// @return Native window handle, or NULL if not set
WE_API void* WE_GetWindowHandle(WE_Engine* engine);

/// @brief Show the wallpaper window
/// @param engine Engine handle
WE_API void WE_ShowWindow(WE_Engine* engine);

/// @brief Hide the wallpaper window
/// @param engine Engine handle
WE_API void WE_HideWindow(WE_Engine* engine);

/// @brief Resize and move the wallpaper window
/// @param engine Engine handle
/// @param x X position
/// @param y Y position
/// @param width Width
/// @param height Height
WE_API void WE_ResizeWindow(WE_Engine* engine, int x, int y, int width, int height);

/*==============================================================================
 * Audio Control
 *============================================================================*/

/// @brief Enable or disable audio
/// @param engine Engine handle
/// @param enable true to enable, false to disable
WE_API void WE_SetAudioEnabled(WE_Engine* engine, bool enable);

/// @brief Set the audio volume
/// @param engine Engine handle
/// @param volume Volume (0-128)
WE_API void WE_SetVolume(WE_Engine* engine, int volume);

/// @brief Mute or unmute the audio
/// @param engine Engine handle
/// @param mute true to mute, false to unmute
WE_API void WE_SetMuted(WE_Engine* engine, bool mute);

/// @brief Enable or disable audio auto-mute (mute when other apps play audio)
/// @param engine Engine handle
/// @param enable true to enable auto-mute, false to disable
WE_API void WE_SetAutoMute(WE_Engine* engine, bool enable);

/*==============================================================================
 * Screenshot
 *============================================================================*/

/// @brief Take a screenshot of the current wallpaper
/// @param engine Engine handle
/// @param path Output file path (supports .png, .jpg, .bmp)
/// @return true on success, false on failure
WE_API bool WE_TakeScreenshot(WE_Engine* engine, const char* path);

/*==============================================================================
 * Error Handling
 *============================================================================*/

/// @brief Get the last error message
/// @param engine Engine handle
/// @return Error message string, or NULL if no error
WE_API const char* WE_GetLastError(WE_Engine* engine);

/// @brief Clear the last error message
/// @param engine Engine handle
WE_API void WE_ClearError(WE_Engine* engine);

#ifdef __cplusplus
}
#endif
