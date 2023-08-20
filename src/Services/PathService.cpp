#include "PathService.hpp"

#include <unistd.h>
#include <array>
#include <filesystem>
#include <climits>

#include "src/Exceptions/Exception.hpp"

namespace Services
{
    std::string PathService::applicationDirPath() {
        auto pathCharArray = std::array<char, PATH_MAX>();

        if (::readlink("/proc/self/exe", pathCharArray.data(), PATH_MAX) < 0) {
            throw Exceptions::Exception("Failed to obtain application directory path.");
        }

        return std::filesystem::path(std::string(pathCharArray.begin(), pathCharArray.end())).parent_path();
    }
}
