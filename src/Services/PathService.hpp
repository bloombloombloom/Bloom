#pragma once

#include <string>
#include <filesystem>

namespace Services
{
    class PathService
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
            return PathService::applicationDirPath() + "/../resources";
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
         * Returns the path to the current project's configuration file (bloom.yaml).
         *
         * @return
         */
        static std::string projectConfigPath() {
            return PathService::projectDirPath() + "/bloom.yaml";
        }

        /**
         * Returns the path to the current project's settings directory.
         *
         * @return
         */
        static std::string projectSettingsDirPath() {
            return PathService::projectDirPath() + "/.bloom";
        }

        /**
         * Returns the path to the directory containing Bloom's target description files.
         *
         * @return
         */
        static std::string targetDescriptionDirPath() {
            return PathService::resourcesDirPath() + "/TargetDescriptionFiles";
        }

        /**
         * Returns the path to the current project's settings file.
         *
         * @return
         */
        static std::string projectSettingsPath() {
            return PathService::projectSettingsDirPath() + "/settings.json";
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
        static std::string compiledResourcesPath() {
#ifdef BLOOM_COMPILED_RESOURCES_PATH_OVERRIDE
            return {BLOOM_COMPILED_RESOURCES_PATH_OVERRIDE};
#else
            return {":/compiled"};
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
        static std::string homeDomainName() {
#ifdef BLOOM_HOME_DOMAIN_NAME_OVERRIDE
            return {BLOOM_HOME_DOMAIN_NAME_OVERRIDE};
#else
            return {"https://bloom.oscillate.io"};
#endif
        }
    };
}
