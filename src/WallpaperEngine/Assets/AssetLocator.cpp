#include "AssetLocator.h"

#include "AssetLoadException.h"
#include "WallpaperEngine/FileSystem/Utf8Path.h"

using namespace WallpaperEngine::Assets;
using namespace WallpaperEngine::FileSystem;

AssetLocator::AssetLocator (ContainerUniquePtr filesystem) : m_filesystem (std::move (filesystem)) { }

std::string AssetLocator::shader (const std::string& filename) const {
    try {
        // Treat incoming std::string paths as UTF-8 on all platforms.
        // On Windows, std::filesystem::path(const char*) uses the active codepage and can garble UTF-8.
        std::filesystem::path shader = string2wstring (filename);

        // detect workshop shaders and check if there's a
        if (auto it = shader.begin (); it != shader.end () && *it++ == std::filesystem::path (string2wstring ("workshop"))) {
            if (it == shader.end ()) {
                // Not enough segments: "workshop/<id>/<file>"
                goto normalShader;
            }

            const std::filesystem::path workshopId = *it++;

            if (it == shader.end ()) {
                goto normalShader;
            }

            // We only rewrite if there are at least 3 segments total; the original code expects the shader file
            // to be the 3rd segment.
            if (++it != shader.end ()) {
                const std::filesystem::path& shaderfile = *it;

                try {
                    shader = std::filesystem::path (string2wstring ("zcompat")) / "scene"
                             / std::filesystem::path (string2wstring ("shaders")) / workshopId / shaderfile;
                    // replace the old path with the new one
                    std::string contents = this->m_filesystem->readString (shader);

                    sLog.out ("Replaced ", filename, " with compat ", shader);
                    return contents;
                } catch (std::filesystem::filesystem_error&) {
                    // these exceptions can be ignored because the replacement file might not exist
                }
            }
        }

normalShader:
        std::filesystem::path final = std::filesystem::path (string2wstring ("shaders")) / shader;
        return this->m_filesystem->readString (final);
    } catch (std::filesystem::filesystem_error& base) {
        throw AssetLoadException (base);
    }
}

std::string AssetLocator::fragmentShader (const std::string& filename) const {
    std::filesystem::path final = string2wstring (filename);
    final.replace_extension (std::filesystem::path (string2wstring ("frag")));
    return this->shader (wstring2string (final.wstring ()));
}

std::string AssetLocator::vertexShader (const std::string& filename) const {
    std::filesystem::path final = string2wstring (filename);
    final.replace_extension (std::filesystem::path (string2wstring ("vert")));
    return this->shader (wstring2string (final.wstring ()));
}

std::string AssetLocator::includeShader (const std::string& filename) const {
    std::filesystem::path final = string2wstring (filename);
    final.replace_extension (std::filesystem::path (string2wstring ("h")));
    return this->shader (wstring2string (final.wstring ()));
}

std::string AssetLocator::readString (const std::string& filename) const {
    try {
        return this->m_filesystem->readString (std::filesystem::path(string2wstring(filename)));
    } catch (std::filesystem::filesystem_error& base) {
        throw AssetLoadException (base);
    }
}

ReadStreamSharedPtr AssetLocator::texture (const std::string& filename) const {
    std::string texKey = filename;
    texKey += ".tex";
    const auto final = std::filesystem::path ("materials") / string2wstring (texKey);

    try {
        return this->m_filesystem->read (final);
    } catch (std::filesystem::filesystem_error& base) {
        throw AssetLoadException (base);
    }
}

ReadStreamSharedPtr AssetLocator::read (const std::string& path) const {
    try {
        return this->m_filesystem->read (std::filesystem::path(string2wstring(path)));
    } catch (std::filesystem::filesystem_error& base) {
        throw AssetLoadException (base);
    }
}

std::filesystem::path AssetLocator::physicalPath (const std::string& path) const {
    try {
        return this->m_filesystem->physicalPath (std::filesystem::path(string2wstring(path)));
    } catch (std::filesystem::filesystem_error& base) {
        throw AssetLoadException (base);
    }
}