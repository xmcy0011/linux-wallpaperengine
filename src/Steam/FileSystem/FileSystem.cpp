#include "FileSystem.h"
#include "WallpaperEngine/Logging/Log.h"
#include <algorithm>
#include <filesystem>
#include <vector>

#if !defined(_WIN32)
#include <sys/stat.h>
#endif

#if !defined(_WIN32)
// Linux / Steam Deck style layouts (relative to $HOME)
std::vector<std::string> appDirectoryPaths = {
    "Steam/steam/steamapps/common",
    ".local/share/Steam/steamapps/common",
    ".var/app/com.valvesoftware.Steam/.local/share/Steam/steamapps/common",
    "snap/steam/common/.local/share/Steam/steamapps/common",
};

std::vector<std::string> workshopDirectoryPaths = {
    ".local/share/Steam/steamapps/workshop/content",
    ".steam/steam/steamapps/workshop/content",
    ".var/app/com.valvesoftware.Steam/.local/share/Steam/steamapps/workshop/content",
    "snap/steam/common/.local/share/Steam/steamapps/workshop/content",
};
#endif

#if !defined(_WIN32)
static std::filesystem::path detectHomepath () {
    char* home = getenv ("HOME");

    if (home == nullptr) {
	sLog.exception ("Cannot find home directory for the current user");
    }

    const std::filesystem::path path = home;

    if (!std::filesystem::is_directory (path)) {
	sLog.exception ("Cannot find home directory for current user, ", home, " is not a directory");
    }

    return path;
}
#endif

#if defined(_WIN32)
#include <optional>

static std::optional<std::filesystem::path> g_windowsAssetsDir;

void Steam::FileSystem::setWindowsAssetRoot (const std::filesystem::path& assetsDirOrInstallRoot) {
    if (assetsDirOrInstallRoot.empty ()) {
	g_windowsAssetsDir.reset ();
	return;
    }
    std::error_code ec;
    const auto c = std::filesystem::weakly_canonical (assetsDirOrInstallRoot, ec);
    g_windowsAssetsDir = ec ? assetsDirOrInstallRoot.lexically_normal () : c;
}

/** Directory that contains the "assets" subfolder (…/wallpaper_engine), or the path passed as install root. */
static std::filesystem::path wallpaperEngineInstallRoot () {
    if (!g_windowsAssetsDir.has_value () || g_windowsAssetsDir->empty ()) {
	return {};
    }
    const auto& p = *g_windowsAssetsDir;
    if (std::filesystem::is_directory (p / "assets")) {
	return p.lexically_normal ();
    }
    if (p.filename () == "assets" && std::filesystem::is_directory (p)) {
	return p.parent_path ().lexically_normal ();
    }
    return p.lexically_normal ();
}

static void push_unique_root (std::vector<std::filesystem::path>& roots, const std::filesystem::path& p) {
    if (p.empty ())
	    return;
    const auto n = p.lexically_normal ();
    if (std::find (roots.begin (), roots.end (), n) != roots.end ())
	    return;
    roots.push_back (n);
}

/** Typical Steam install roots on Windows (steamapps lives under these). */
static std::vector<std::filesystem::path> steamRootsWindows () {
    std::vector<std::filesystem::path> roots;
	push_unique_root (roots, "D:/Program Files (x86)/Steam");
    return roots;
}
#endif

std::filesystem::path Steam::FileSystem::workshopDirectory (int appID, const std::string& contentID) {
#if defined(_WIN32)
    if (const auto install = wallpaperEngineInstallRoot (); !install.empty ()) {
        // …/steamapps/common/wallpaper_engine -> parents to steamapps, then workshop/content/<appId>/<id>
        const auto steamapps = install.parent_path ().parent_path ();
        const auto currentpath = steamapps / "workshop" / "content" / std::to_string (appID) / contentID;

        if (std::filesystem::exists (currentpath) && std::filesystem::is_directory (currentpath)) {
            return currentpath;
        }
    }

    for (const auto& root : steamRootsWindows ()) {
	const auto currentpath
	    = root / "steamapps" / "workshop" / "content" / std::to_string (appID) / contentID;

	if (!std::filesystem::exists (currentpath) || !std::filesystem::is_directory (currentpath)) {
	    continue;
	}

	return currentpath;
    }

    sLog.exception ("Cannot find workshop directory for steam app ", appID, " and content ", contentID);
#else
    auto homepath = detectHomepath ();

    for (const auto& current : workshopDirectoryPaths) {
	auto currentpath = std::filesystem::path (homepath) / current / std::to_string (appID) / contentID;

	if (!std::filesystem::exists (currentpath) || !std::filesystem::is_directory (currentpath)) {
	    continue;
	}

	return currentpath;
    }

    sLog.exception ("Cannot find workshop directory for steam app ", appID, " and content ", contentID);
#endif
}

std::filesystem::path Steam::FileSystem::appDirectory (const std::string& appDirectory, const std::string& path) {
#if defined(_WIN32)
    if (const auto install = wallpaperEngineInstallRoot (); !install.empty ()) {
        const std::filesystem::path currentpath = path.empty () ? install : (install / path);

        if (std::filesystem::is_directory (currentpath)) {
            return currentpath;
        }
    }

    for (const auto& root : steamRootsWindows ()) {
	auto currentpath = root / "steamapps" / "common" / appDirectory / path;

	if (!std::filesystem::exists (currentpath) || !std::filesystem::is_directory (currentpath)) {
	    continue;
	}

	return currentpath;
    }

    sLog.exception ("Cannot find directory for steam app ", appDirectory, ": ", path);
#else
    auto homepath = detectHomepath ();

    for (const auto& current : appDirectoryPaths) {
	auto currentpath = std::filesystem::path (homepath) / current / appDirectory / path;

	if (!std::filesystem::exists (currentpath) || !std::filesystem::is_directory (currentpath)) {
	    continue;
	}

	return currentpath;
    }

    sLog.exception ("Cannot find directory for steam app ", appDirectory, ": ", path);
#endif
}
