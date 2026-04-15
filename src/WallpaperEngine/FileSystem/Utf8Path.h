#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <codecvt>

namespace WallpaperEngine::FileSystem {

inline std::string wstring2string (const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

inline std::wstring string2wstring (const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}

} // namespace WallpaperEngine::FileSystem
