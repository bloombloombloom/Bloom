#pragma once

#include <string>

namespace Bloom
{
    class Paths
    {
    public:
        /**
         * Returns the path to the directory in which the Bloom binary resides.
         *
         * @return
         */
        static std::string applicationDirPath();

        /**
         * Returns the path to the Resources directory, located in the application directory.
         *
         * @return
         */
        static inline std::string resourcesDirPath() {
            return Paths::applicationDirPath() + "/../resources/";
        }

        /**
         * Returns the path to Bloom's compiled resources.
         *
         * If the debug configuration is enabled, this function will return the absolute path to Bloom's source code,
         * meaning we won't use compiled resources for debug builds. This is useful for debugging and tweaking QT UI
         * files such as QT stylesheets and UI templates.
         * @return
         */
        static inline std::string compiledResourcesPath() {
#ifdef BLOOM_COMPILED_RESOURCES_PATH_OVERRIDE
            return std::string(BLOOM_COMPILED_RESOURCES_PATH_OVERRIDE);
#else
            return std::string(":/compiled");
#endif
        }
    };
}
