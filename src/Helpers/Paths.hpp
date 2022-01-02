#pragma once

#include <string>
#include <filesystem>

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
        static std::string resourcesDirPath() {
            return Paths::applicationDirPath() + "/../resources/";
        }

        /**
         * Returns the path to the current project's directory (the working directory, from which Bloom was run)
         *
         * @return
         */
        static std::string projectDirPath() {
            return std::filesystem::current_path().string();
        }

        /**
         * Returns the path to the current project's configuration file (bloom.json).
         *
         * @return
         */
        static std::string projectConfigPath() {
            return std::filesystem::current_path().string() + "/bloom.json";
        }

        /**
         * Returns the path to the current project's settings directory.
         *
         * @return
         */
        static std::string projectSettingsDirPath() {
            return std::filesystem::current_path().string() + "/.bloom";
        }

        /**
         * Returns the path to the current project's settings file.
         *
         * @return
         */
        static std::string projectSettingsPath() {
            return std::filesystem::current_path().string() + "/.bloom/settings.json";
        }

        /**
         * Returns the path to Bloom's compiled resources.
         *
         * If the debug configuration is enabled, this function will return the absolute path to Bloom's source code,
         * meaning we won't use compiled resources for debug builds. This is useful for debugging and tweaking QT UI
         * files such as QT stylesheets and UI templates.
         *
         * @return
         */
        static inline std::string compiledResourcesPath() {
#ifdef BLOOM_COMPILED_RESOURCES_PATH_OVERRIDE
            return std::string(BLOOM_COMPILED_RESOURCES_PATH_OVERRIDE);
#else
            return std::string(":/compiled");
#endif
        }

        /**
         * Returns the domain name for the Bloom website.
         *
         * The domain name can be overridden via the BLOOM_HOME_DOMAIN_NAME_OVERRIDE compile definition.
         * See CMakeLists.txt
         *
         * @return
         */
        static inline std::string homeDomainName() {
#ifdef BLOOM_HOME_DOMAIN_NAME_OVERRIDE
            return std::string(BLOOM_HOME_DOMAIN_NAME_OVERRIDE);
#else
            return std::string("https://bloom.oscillate.io");
#endif
        }
    };
}
