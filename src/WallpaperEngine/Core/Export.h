#pragma once

// DLL export/import macros for Windows
#if defined(_WIN32) || defined(_CYGWIN)
	#ifdef WALLPAPERENGINE_EXPORTS
		#define WE_API __declspec(dllexport)
	#else
		#ifdef WALLPAPERENGINE_STATIC
			#define WE_API
		#else
			#define WE_API __declspec(dllimport)
		#endif
	#endif
#else
	#if __GNUC__ >= 4
		#define WE_API __attribute__ ((visibility ("default")))
	#else
		#define WE_API
	#endif
#endif
