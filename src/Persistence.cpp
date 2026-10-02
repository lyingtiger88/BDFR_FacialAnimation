#include "bdfr/core/Persistence.h"
#include "bdfr/core/JsonCodec.h"

#include <fstream>
#include <sstream>
#include <system_error>

namespace bdfr {

bool Persistence::saveTextAtomic(const std::filesystem::path& path,
                                 const std::string& content,
                                 std::string* error) {
    if (path.empty()) {
        if (error) *error = "empty persistence path";
        return false;
    }

    std::error_code ec;
    if (path.has_parent_path())
        std::filesystem::create_directories(path.parent_path(), ec);
    if (ec) {
        if (error) *error = "failed to create parent directories: " + ec.message();
        return false;
    }

    const std::filesystem::path temp = path.string() + ".tmp";
    {
        std::ofstream out(temp, std::ios::binary | std::ios::trunc);
        if (!out) {
            if (error) *error = "failed to open temporary file for writing";
            return false;
        }
        out.write(content.data(), static_cast<std::streamsize>(content.size()));
        out.flush();
        if (!out) {
            if (error) *error = "failed while writing temporary file";
            return false;
        }
    }

    std::filesystem::rename(temp, path, ec);
    if (ec) {
        std::error_code removeError;
        std::filesystem::remove(path, removeError);
        ec.clear();
        std::filesystem::rename(temp, path, ec);
    }

    if (ec) {
        std::filesystem::remove(temp);
        if (error) *error = "failed to replace destination file: " + ec.message();
        return false;
    }
    return true;
}

bool Persistence::loadText(const std::filesystem::path& path,
                           std::string& content,
                           std::string* error) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        if (error) *error = "failed to open file for reading";
        return false;
    }

    std::ostringstream buffer;
    buffer << in.rdbuf();
    if (!in.good() && !in.eof()) {
        if (error) *error = "failed while reading file";
        return false;
    }
    content = buffer.str();
    return true;
}

bool Persistence::saveProject(const std::filesystem::path& path,
                              const Project& project,
                              std::string* error) {
    return saveTextAtomic(path, JsonCodec::encodeProject(project), error);
}

bool Persistence::loadProject(const std::filesystem::path& path,
                              Project& project,
                              std::string* error) {
    std::string text;
    if (!loadText(path, text, error)) return false;
    return JsonCodec::decodeProject(text, project, error);
}

} // namespace bdfr
