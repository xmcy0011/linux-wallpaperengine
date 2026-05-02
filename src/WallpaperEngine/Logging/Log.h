#pragma once

#include <iostream>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
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

struct LogAtProxy;

/**
 * Singleton logger. Each line is prefixed with local time (ms), level, basename(__FILE__) and __LINE__.
 * Use the sLog macro so file/line are captured at the call site.
 */
class Log {
    friend struct LogAtProxy;

public:
    Log ();

    void addOutput (std::ostream* stream);
    void addError (std::ostream* stream);

    static Log& get ();

private:
    static std::string formatLogPrefix (const char* level, const char* file, int line);

    template <typename... Data> std::string buildBuffer (Data... data) {
        // buffer the string first
        std::stringbuf buffer;
        std::ostream bufferStream (&buffer);
        (detail::appendToStream (bufferStream, std::forward<Data> (data)), ...);
        return buffer.str ();
    }

    template <typename... Data>
    void emitLine (const char* level, bool useErrorStreams, const char* file, int line, Data&&... data) {
        const std::string prefix = formatLogPrefix (level, file, line);
        const std::string body = this->buildBuffer (std::forward<Data> (data)...);
        const std::string lineText = prefix + body;
        auto& streams = useErrorStreams ? this->mErrors : this->mOutputs;
        for (const auto cur : streams) {
            *cur << lineText << std::endl;
        }
    }

    template <class EX, typename... Data> [[noreturn]] void emitException (const char* file, int line, Data&&... data) {
        const std::string prefix = formatLogPrefix ("EXCEPTION", file, line);
        const std::string body = this->buildBuffer (std::forward<Data> (data)...);
        const std::string lineText = prefix + body;
        for (const auto cur : this->mErrors) {
            *cur << lineText << std::endl;
        }
        throw EX (body);
    }

    template <typename... Data> [[noreturn]] void emitException (const char* file, int line, Data&&... data) {
        this->emitException<std::runtime_error> (file, line, std::forward<Data> (data)...);
    }

    std::vector<std::ostream*> mOutputs = {};
    std::vector<std::ostream*> mErrors = {};
    static std::unique_ptr<Log> sInstance;
};

/**
 * Carries __FILE__ / __LINE__ for each log call; use via sLog macro.
 * addOutput / addError are forwarded to Log so init code can keep using sLog.addOutput(...).
 */
struct LogAtProxy {
    Log* log;
    const char* file;
    int line;

    void addOutput (std::ostream* stream) { this->log->addOutput (stream); }
    void addError (std::ostream* stream) { this->log->addError (stream); }

    template <typename... Data> void out (Data&&... data) {
        this->log->emitLine ("OUT", false, this->file, this->line, std::forward<Data> (data)...);
    }

    template <typename... Data> void debug (Data&&... data) {
#if (!NDEBUG) && (!ERRORONLY)
        this->log->emitLine ("DEBUG", false, this->file, this->line, std::forward<Data> (data)...);
#endif
    }

    template <typename... Data> void debugerror (Data&&... data) {
#if (!NDEBUG) && (ERRORONLY)
        this->log->emitLine ("DEBUGERR", true, this->file, this->line, std::forward<Data> (data)...);
#endif
    }

    template <typename... Data> void error (Data&&... data) {
        this->log->emitLine ("ERROR", true, this->file, this->line, std::forward<Data> (data)...);
    }

    template <class EX, typename... Data> [[noreturn]] void exception (Data&&... data) {
        this->log->emitException<EX> (this->file, this->line, std::forward<Data> (data)...);
    }

    template <typename... Data> [[noreturn]] void exception (Data&&... data) {
        this->log->emitException (this->file, this->line, std::forward<Data> (data)...);
    }
};

inline LogAtProxy makeLogAt (const char* file, int line) noexcept {
    return LogAtProxy {&Log::get (), file, line};
}

} // namespace WallpaperEngine::Logging

#define sLog (::WallpaperEngine::Logging::makeLogAt (__FILE__, __LINE__))
