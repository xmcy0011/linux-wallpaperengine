#pragma once

#include <filesystem>
#include <string>

namespace Steam::FileSystem {
std::filesystem::path workshopDirectory (int appID, const std::string& contentID);
std::filesystem::path appDirectory (const std::string& appDirectory, const std::string& path);
#if defined(_WIN32)
/** Wallpaper Engine "assets" folder (or install root); enables app/workshop resolution without Linux Steam paths. */
void setWindowsAssetRoot (const std::filesystem::path& assetsDirOrInstallRoot);
#endif
} // namespace Steam::FileSystem