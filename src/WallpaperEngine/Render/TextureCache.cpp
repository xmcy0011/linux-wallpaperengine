#include "TextureCache.h"

#include "WallpaperEngine/FileSystem/Container.h"
#include "WallpaperEngine/FileSystem/Utf8Path.h"

#include "CTexture.h"
#include "WallpaperEngine/Assets/AssetLoadException.h"
#include "WallpaperEngine/Render/Helpers/ContextAware.h"

#include "WallpaperEngine/Data/Model/Project.h"
#include "WallpaperEngine/Data/Parsers/TextureParser.h"

using namespace WallpaperEngine::Render;
using namespace WallpaperEngine::FileSystem;
using namespace WallpaperEngine::Data::Parsers;

TextureCache::TextureCache (RenderContext& context) : Helpers::ContextAware (context) { }

std::shared_ptr<const TextureProvider> TextureCache::resolve (const std::string& filename) {
    if (const auto found = this->m_textureCache.find (filename); found != this->m_textureCache.end ()) {
	return found->second;
    }

    // search for the texture in all the different containers just in case
    for (const auto& project : this->getContext ().getApp ().getBackgrounds () | std::views::values) {
	try {
	    const auto contents = project->assetLocator->texture (filename);
	    auto stream = BinaryReader (contents);

	    // Create metadata loader lambda that captures the assetLocator
	    // so we need to construct the full path here
	    auto metadataLoader = [&project] (const std::string& metaFilename) -> std::string {
		// metaFilename is UTF-8; keep path operations in std::filesystem::path and convert explicitly when needed.
		std::filesystem::path fullPath = std::filesystem::path ("materials") / string2wstring (metaFilename);
		return project->assetLocator->readString (wstring2string (fullPath.wstring ()));
	    };

	    auto parsedTexture = TextureParser::parse (stream, filename, metadataLoader);
	    auto texture = std::make_shared<CTexture> (this->getContext (), std::move (parsedTexture));

	    this->store (filename, texture);

	    return texture;
	} catch (AssetLoadException&) {
	    // ignored, this happens if we're looking at the wrong background
	}
    }

    throw AssetLoadException ("Cannot find file", filename, std::error_code ());
}

void TextureCache::store (const std::string& name, std::shared_ptr<const TextureProvider> texture) {
    this->m_textureCache.insert_or_assign (name, texture);
}
