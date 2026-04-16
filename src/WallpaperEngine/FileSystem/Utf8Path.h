#pragma once

#include <filesystem>
#include <string>
#include <string_view>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#endif

namespace WallpaperEngine::FileSystem {

inline std::string wstring2string (const std::wstring& wstr) {
#ifdef _WIN32
    if (wstr.empty ()) {
        return {};
    }

    const int size = WideCharToMultiByte (CP_UTF8, 0, wstr.c_str (), static_cast<int> (wstr.size ()), nullptr, 0, nullptr, nullptr);
    if (size <= 0) {
        return {};
    }

    std::string str (static_cast<size_t> (size), '\0');
    WideCharToMultiByte (CP_UTF8, 0, wstr.c_str (), static_cast<int> (wstr.size ()), str.data (), size, nullptr, nullptr);
    return str;
#else
    const std::filesystem::path path (wstr);
#if defined(__cpp_char8_t)
    const auto u8 = path.u8string ();
    return std::string (u8.begin (), u8.end ());
#else
    return path.u8string ();
#endif
#endif
}

inline std::wstring string2wstring (const std::string& str) {
#ifdef _WIN32
    if (str.empty ()) {
        return {};
    }

    const int size = MultiByteToWideChar (CP_UTF8, 0, str.c_str (), static_cast<int> (str.size ()), nullptr, 0);
    if (size <= 0) {
        return {};
    }

    std::wstring wstr (static_cast<size_t> (size), L'\0');
    MultiByteToWideChar (CP_UTF8, 0, str.c_str (), static_cast<int> (str.size ()), wstr.data (), size);
    return wstr;
#else
    return std::filesystem::u8path (str).wstring ();
#endif
}

} // namespace WallpaperEngine::FileSystem
