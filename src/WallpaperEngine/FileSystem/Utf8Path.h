#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace WallpaperEngine::FileSystem {

/// Interprets @p utf8 as a UTF-8 encoded path (JSON / project / CLI).
inline std::filesystem::path pathFromUtf8 (const std::string& utf8) {
    return std::filesystem::path (std::u8string_view (
        reinterpret_cast<const char8_t*>(utf8.data ()), utf8.size ()));
}

inline std::filesystem::path pathFromUtf8 (std::string_view utf8) {
    return std::filesystem::path (std::u8string_view (
        reinterpret_cast<const char8_t*>(utf8.data ()), utf8.size ()));
}

/// UTF-8 logical key with '/' separators (matches .pkg entries and VFS keys).
inline std::string pathToUtf8Generic (const std::filesystem::path& path) {
    const std::u8string u8 = path.generic_u8string ();
    return std::string (reinterpret_cast<const char*>(u8.data ()), u8.size ());
}

} // namespace WallpaperEngine::FileSystem
