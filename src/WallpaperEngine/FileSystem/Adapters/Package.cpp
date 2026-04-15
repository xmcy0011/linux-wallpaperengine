#include <fstream>
#include <memory>

#include "Package.h"

#include "WallpaperEngine/FileSystem/Utf8Path.h"
#include "WallpaperEngine/Data/Parsers/PackageParser.h"
#include "WallpaperEngine/Data/Utils/BinaryReader.h"
#include "WallpaperEngine/Data/Utils/MemoryStream.h"

#include <algorithm>

using namespace WallpaperEngine::FileSystem;
using namespace WallpaperEngine::FileSystem::Adapters;

ReadStreamSharedPtr PackageAdapter::open (const std::filesystem::path& path) const {
    const std::string pathKey = wstring2string (path);
    // find the file entry
    const auto it = std::ranges::find_if (this->package->files, [&pathKey] (const auto& file) {
	return file->filename == pathKey;
    });

    if (it == this->package->files.end ()) {
	throw std::filesystem::filesystem_error ("Cannot find file", wstring2string (path), std::error_code ());
    }

    // read file into memory
    auto buffer = std::make_unique<char[]> (it->get ()->length);

    // go to the file's position and read into the buffer
    this->package->file->base ().seekg (it->get ()->offset + this->package->baseOffset, std::ios::beg);
    this->package->file->next (buffer.get (), it->get ()->length);

    // create a memory stream and return that
    return std::make_shared<MemoryStream> (std::move (buffer), it->get ()->length);
}

bool PackageAdapter::exists (const std::filesystem::path& path) const {
    const std::string pathKey = wstring2string (path);
    for (const auto& file : this->package->files) {
	if (file->filename == pathKey) {
	    return true;
	}
    }

    return false;
}

std::filesystem::path PackageAdapter::physicalPath (const std::filesystem::path& path) const {
    throw std::filesystem::filesystem_error ("Package adapter does not support realpath", wstring2string (path), std::error_code ());
}

bool PackageFactory::handlesMountpoint (const std::filesystem::path& path) const {
    const auto finalpath = std::filesystem::canonical (path);
    const auto status = std::filesystem::status (finalpath);

    return std::filesystem::exists (finalpath) && std::filesystem::is_regular_file (status)
	&& finalpath.extension () == ".pkg";
}

AdapterSharedPtr PackageFactory::create (const std::filesystem::path& path) const {
    const auto stream = std::make_shared<std::ifstream> (path, std::ios::binary);
    auto package = Data::Parsers::PackageParser::parse (stream);

    return std::make_unique<PackageAdapter> (std::move (package));
}
