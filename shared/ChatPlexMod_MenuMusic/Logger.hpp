#pragma once

#include <CP_SDK/Logging/ILogger.hpp>

namespace ChatPlexMod_MenuMusic {

    /// @brief Logger instance holder
    class Logger
    {
        public:
            /// @brief Logger instance
            static CP_SDK::Logging::ILogger* Instance;

    };

}   ///< namespace ChatPlexMod_MenuMusic