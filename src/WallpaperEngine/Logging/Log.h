#pragma once

#include <iostream>
#include <memory>
#include <ostream>
#include <sstream>
#include <vector>

#include "WallpaperEngine/FileSystem/Utf8Path.h"

namespace detail {
inline void appendToStream (std::ostream& os, const std::wstring& wstr) {
    os << WallpaperEngine::FileSystem::wstring2string (wstr);
}
inline void appendToStream (std::ostream& os, std::wstring_view wsv) { appendToStream (os, std::wstring (wsv)); }
inline void appendToStream (std::ostream& os, const wchar_t* wcs) {
    if (wcs == nullptr) {
        os << "(null)";
        return;
    }
    appendToStream (os, std::wstring_view (wcs));
}
inline void appendToStream (std::ostream& os, wchar_t wc) { appendToStream (os, std::wstring_view (&wc, 1)); }
inline void appendToStream (std::ostream& os, const std::filesystem::path& path) {
    os << WallpaperEngine::FileSystem::wstring2string (path.wstring ());
}
template <typename T> inline void appendToStream (std::ostream& os, T&& v) { os << std::forward<T> (v); }
} // namespace detail

namespace WallpaperEngine::Logging {
/**
 * Singleton class, simplifies logging for the whole app
 */
class Log {
public:
    Log ();

    void addOutput (std::ostream* stream);
    void addError (std::ostream* stream);

    template <typename... Data> void out (Data... data) {
        std::string str = this->buildBuffer (data...);

        // then send it to all the outputs configured
        for (const auto cur : this->mOutputs) {
            *cur << str << std::endl;
        }
    }

    template <typename... Data> void debug (Data... data) {
#if (!NDEBUG) && (!ERRORONLY)
        std::string str = this->buildBuffer (data...);

        // then send it to all the outputs configured
        for (const auto cur : this->mOutputs) {
            *cur << str << std::endl;
        }
#endif /* DEBUG */
    }

    template <typename... Data> void debugerror (Data... data) {
#if (!NDEBUG) && (ERRORONLY)
        std::string str = this->buildBuffer (data...);

        // then send it to all the outputs configured
        for (const auto cur : this->mOutputs) {
            *cur << str << std::endl;
        }
#endif /* DEBUG */
    }

    template <typename... Data> void error (Data... data) {
        std::string str = this->buildBuffer (data...);

        // then send it to all the outputs configured
        for (const auto cur : this->mErrors) {
            *cur << str << std::endl;
        }
    }

    template <class EX, typename... Data> [[noreturn]] void exception (Data... data) {
        std::string str = this->buildBuffer (data...);
        // then send it to all the outputs configured
        for (const auto cur : this->mErrors) {
            *cur << str << std::endl;
        }

        // now throw the exception
        throw EX (str);
    }

    template <typename... Data> [[noreturn]] void exception (Data... data) {
        this->exception<std::runtime_error> (data...);
    }

    static Log& get ();

private:
    template <typename... Data> std::string buildBuffer (Data... data) {
        // buffer the string first
        std::stringbuf buffer;
        std::ostream bufferStream (&buffer);
        (detail::appendToStream (bufferStream, std::forward<Data> (data)), ...);
        return buffer.str ();
    }

    std::vector<std::ostream*> mOutputs = {};
    std::vector<std::ostream*> mErrors = {};
    static std::unique_ptr<Log> sInstance;
};
} // namespace WallpaperEngine::Logging

#define sLog (WallpaperEngine::Logging::Log::get ())