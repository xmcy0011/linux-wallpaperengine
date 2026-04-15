#include <filesystem>
#include <fstream>
#include <memory>

#include "WallpaperEngine/FileSystem/Utf8Path.h"
#include "Directory.h"

#include "WallpaperEngine/Assets/AssetLoadException.h"

using namespace WallpaperEngine::FileSystem;
using namespace WallpaperEngine::FileSystem::Adapters;

namespace {
/** Avoid path::string() prefix checks (encoding / separator issues on Windows). */
bool resolvedPathUnderBase (const std::filesystem::path& base, const std::filesystem::path& resolved) {
    const auto rel = resolved.lexically_relative (base);
    if (rel.empty ()) {
	return resolved == base;
    }
    for (const auto& part : rel) {
	if (part == "..") {
	    return false;
	}
    }
    return true;
}
} // namespace

ReadStreamSharedPtr DirectoryAdapter::open (const std::filesystem::path& path) const {
    auto finalpath = std::filesystem::canonical (this->basepath / path);

    if (!resolvedPathUnderBase (this->basepath, finalpath)) {
	throw std::filesystem::filesystem_error ("Cannot find file", wstring2string (path), std::error_code ());
    }

    const auto status = std::filesystem::status (finalpath);

    if (!std::filesystem::exists (finalpath)) {
	throw std::filesystem::filesystem_error ("Cannot find file", wstring2string (path), std::error_code ());
    }

    if (!std::filesystem::is_regular_file (status)) {
	throw std::filesystem::filesystem_error ("Expected file but found a directory", wstring2string (path), std::error_code ());
    }

    return std::make_shared<std::ifstream> (finalpath);
}

bool DirectoryAdapter::exists (const std::filesystem::path& path) const {
    try {
	const auto finalpath = std::filesystem::canonical (this->basepath / path);

	if (!resolvedPathUnderBase (this->basepath, finalpath)) {
	    return false;
	}

	const auto status = std::filesystem::status (finalpath);

	if (!std::filesystem::exists (finalpath)) {
	    return false;
	}

	if (!std::filesystem::is_regular_file (status)) {
	    return false;
	}

	return true;
    } catch (std::filesystem::filesystem_error&) {
	return false;
    }
}

std::filesystem::path DirectoryAdapter::physicalPath (const std::filesystem::path& path) const {
    auto finalpath = std::filesystem::canonical (this->basepath / path);

    if (!resolvedPathUnderBase (this->basepath, finalpath)) {
	throw std::filesystem::filesystem_error ("Cannot find file", wstring2string (path), std::error_code ());
    }

    return finalpath;
}

bool DirectoryFactory::handlesMountpoint (const std::filesystem::path& path) const {
    const auto finalpath = std::filesystem::canonical (path);
    const auto status = std::filesystem::status (finalpath);

    return std::filesystem::exists (finalpath) && std::filesystem::is_directory (status);
}

AdapterSharedPtr DirectoryFactory::create (const std::filesystem::path& path) const {
    auto finalpath = std::filesystem::canonical (path);
    const auto status = std::filesystem::status (finalpath);

    if (!std::filesystem::exists (finalpath)) {
	throw std::filesystem::filesystem_error ("Cannot find directory", wstring2string (path), std::error_code ());
    }

    if (!std::filesystem::is_directory (status)) {
	throw std::filesystem::filesystem_error ("Expected directory but found a file", wstring2string (path), std::error_code ());
    }

    return std::make_unique<DirectoryAdapter> (finalpath);
}