#include "Log.h"

#include <cassert>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <memory>
#include <sstream>

using namespace WallpaperEngine::Logging;

namespace {
const char* fileBasename (const char* path) {
    if (path == nullptr || *path == '\0') {
        return "?";
    }
    const char* last = path;
    for (const char* p = path; *p != '\0'; ++p) {
        if (*p == '/' || *p == '\\') {
            last = p + 1;
        }
    }
    return last;
}
} // namespace

Log::Log () { assert (this->sInstance == nullptr); }

Log& Log::get () {
    if (sInstance == nullptr) {
	sInstance = std::make_unique<Log> ();
    }

    return *sInstance;
}

void Log::addOutput (std::ostream* stream) { this->mOutputs.push_back (stream); }

void Log::addError (std::ostream* stream) { this->mErrors.push_back (stream); }

std::string Log::formatLogPrefix (const char* level, const char* file, int line) {
    const auto now = std::chrono::system_clock::now ();
    const std::time_t t = std::chrono::system_clock::to_time_t (now);
    const auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds> (now.time_since_epoch ()) % 1000;

    std::tm tmBuf {};
#if defined(_WIN32)
    localtime_s (&tmBuf, &t);
#else
    localtime_r (&t, &tmBuf);
#endif

    std::ostringstream oss;
    oss << '[' << std::put_time (&tmBuf, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill ('0') << std::setw (3)
        << ms.count () << "] [" << level << "] " << fileBasename (file) << ':' << line << ' ';
    return oss.str ();
}

std::unique_ptr<Log> Log::sInstance = nullptr;
