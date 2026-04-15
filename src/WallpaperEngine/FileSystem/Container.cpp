#include <map>
#include <memory>

#include "Container.h"

#include "WallpaperEngine/FileSystem/Utf8Path.h"
#include "Adapters/Directory.h"
#include "Adapters/Package.h"
#include "Adapters/Types.h"
#include "Adapters/Virtual.h"

using namespace WallpaperEngine::FileSystem;
using namespace WallpaperEngine::FileSystem::Adapters;

/**
 * Normalizes a file path to get rid of relative stuff as most as possible
 * This is not a security measure but helps keep adapters that do not really have an actual filesystem
 * behind to trust the input data without much validation
 * @see https://en.cppreference.com/w/cpp/filesystem/path/lexically_normal
 */
std::filesystem::path normalize_path (const std::filesystem::path& input_path) {
    return input_path.lexically_normal ();
}

/** Mount roots are POSIX-style; compare UTF-8 generic keys (MSVC generic_string() uses the ANSI code page). */
static bool path_has_prefix (const std::filesystem::path& path, const std::filesystem::path& root) {
    const std::string p = pathToUtf8Generic (path);
    const std::string r = pathToUtf8Generic (root);
    return p.size () >= r.size () && p.compare (0, r.size (), r) == 0;
}

Container::Container () {
    // register all available factories
    this->m_factories.push_back (std::make_unique<VirtualFactory> ());
    this->m_factories.push_back (std::make_unique<PackageFactory> ());
    this->m_factories.push_back (std::make_unique<DirectoryFactory> ());

    this->m_vfs = std::make_shared<VirtualAdapter> ();
    this->m_mountpoints.emplace_back ("/", this->m_vfs);
}

ReadStreamSharedPtr Container::read (const std::filesystem::path& path) const {
    const auto resolved = this->resolveAdapterForFile (path);
    return resolved.adapter.open (resolved.pathWithinMount);
}

std::string Container::readString (const std::filesystem::path& path) const {
    const auto stream = this->read (path);
    std::stringstream buffer;
    buffer << stream->rdbuf ();
    return buffer.str ();
}

std::filesystem::path Container::physicalPath (const std::filesystem::path& path) const {
    const auto resolved = this->resolveAdapterForFile (path);
    return resolved.adapter.physicalPath (resolved.pathWithinMount);
}

AdapterSharedPtr Container::mount (const std::filesystem::path& path, const std::filesystem::path& mountPoint) {
    // check if any adapter can handle the path
    for (const auto& factory : this->m_factories) {
	if (factory->handlesMountpoint (path) == false) {
	    continue;
	}

	return this->m_mountpoints.emplace_back (mountPoint, factory->create (path)).second;
    }

    throw std::filesystem::filesystem_error (
	"The specified mount cannot be handled by any of the filesystem adapters", path, std::error_code ()
    );
}

VirtualAdapter& Container::getVFS () const { return *this->m_vfs; }

Container::MountResolution Container::resolveAdapterForFile (const std::filesystem::path& path, const bool triedVfsRootSlash) const {
    const auto normalized = normalize_path (path);
    const std::string norm = pathToUtf8Generic (normalized);

    for (const auto& [root, adapter] : this->m_mountpoints) {
	if (!path_has_prefix (normalized, root)) {
	    continue;
	}

	const std::string rootStr = pathToUtf8Generic (root);
	std::string_view tail (norm);
	if (tail.size () < rootStr.size ()) {
	    continue;
	}
	tail.remove_prefix (rootStr.size ());
	while (!tail.empty () && (tail.front () == '/' || tail.front () == '\\')) {
	    tail.remove_prefix (1);
	}
	if (tail.empty ()) {
	    continue;
	}

	const std::filesystem::path relPath = pathFromUtf8 (tail);
	if (adapter->exists (relPath) == false) {
	    continue;
	}

	return MountResolution { *adapter, relPath };
    }

    // Map a relative logical path to the VFS root once. On Windows, std::filesystem::path("/") /
    // normalized can collapse to a native absolute path or normalize in ways that never match the
    // POSIX "/" mount while still differing from `norm`, which can recurse until stack overflow.
#if defined(_WIN32)
    const bool tryVfsRootSlash = !normalized.is_absolute ();
#else
    const bool tryVfsRootSlash = true;
#endif
    if (!triedVfsRootSlash && !norm.empty () && tryVfsRootSlash
	&& !path_has_prefix (normalized, std::filesystem::path ("/"))) {
	const auto rooted = normalize_path (std::filesystem::path ("/") / normalized);
	const std::string rootedStr = pathToUtf8Generic (rooted);
	if (rootedStr != norm) {
	    return this->resolveAdapterForFile (rooted, true);
	}
    }

    throw std::filesystem::filesystem_error (
	"Cannot find requested file in any of the mountpoints", path, std::error_code ()
    );
}
