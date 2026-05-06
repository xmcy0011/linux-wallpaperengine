#include "Utf8.h"

namespace WallpaperEngine::Render::Text {
std::u32string utf8ToUtf32 (const std::string& utf8) {
    std::u32string result;
    result.reserve (utf8.size ());

    size_t i = 0;
    while (i < utf8.size ()) {
        const unsigned char c = static_cast<unsigned char> (utf8[i]);
        char32_t cp = 0;
        size_t extra = 0;

        if (c <= 0x7F) {
            cp = c;
        } else if ((c & 0xE0) == 0xC0) {
            cp = c & 0x1F;
            extra = 1;
        } else if ((c & 0xF0) == 0xE0) {
            cp = c & 0x0F;
            extra = 2;
        } else if ((c & 0xF8) == 0xF0) {
            cp = c & 0x07;
            extra = 3;
        } else {
            result.push_back (U'?');
            ++i;
            continue;
        }

        if (i + extra >= utf8.size ()) {
            result.push_back (U'?');
            break;
        }

        bool valid = true;
        for (size_t j = 1; j <= extra; ++j) {
            const unsigned char cc = static_cast<unsigned char> (utf8[i + j]);
            if ((cc & 0xC0) != 0x80) {
                valid = false;
                break;
            }
            cp = (cp << 6) | (cc & 0x3F);
        }

        result.push_back (valid ? cp : U'?');
        i += extra + 1;
    }

    return result;
}
} // namespace WallpaperEngine::Render::Text
