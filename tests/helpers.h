#include <string>

namespace transport_catalogue::details {

    inline std::string normalize_line_endings(const std::string& s) {
        std::string result;
        for (size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '\r') {
                if (i + 1 < s.size() && s[i+1] == '\n') {
                    ++i; // skip '\r', keep '\n'
                }
                result += '\n';
            } else {
                result += s[i];
            }
        }
        return result;
    }

} // namespace transport_catalogue::details
