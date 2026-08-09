#pragma once

#include <CP_SDK/Utils/Il2cpp.hpp>

#include <optional>

#include <UnityEngine/Color.hpp>


namespace ChatPlexMod_MenuMusic {

    namespace _u
    {
        using namespace UnityEngine;
    }

    class ModulePresence
    {
        CP_SDK_NO_DEF_CTORS(ModulePresence);

        private:
            static std::optional<bool> m_ChatRequest;

        public:
            static bool ChatRequest();

    };

}   ///< namespace ChatPlexMod_MenuMusic