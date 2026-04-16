#pragma once

#include "WallpaperEngine/FileSystem/Container.h"

namespace WallpaperEngine::Assets {
using namespace WallpaperEngine::FileSystem;
using namespace WallpaperEngine::Data::Model;
class AssetLocator {
public:
    explicit AssetLocator (ContainerUniquePtr filesystem);

    std::string vertexShader (const std::string& filename) const;
    std::string fragmentShader (const std::string& filename) const;
    std::string includeShader (const std::string& filename) const;
    ReadStreamSharedPtr texture (const std::string& filename) const;
    std::string readString (const std::string& filename) const;
    ReadStreamSharedPtr read (const std::string& path) const;
    std::filesystem::path physicalPath (const std::string& path) const;

private:
    std::string shader (const std::string& filename) const;

    ContainerUniquePtr m_filesystem;
};

using AssetLocatorUniquePtr = std::unique_ptr<AssetLocator>;
}