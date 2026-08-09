#include "ChatPlexMod_MenuMusic/ModulePresence.hpp"
#include "ChatPlexMod_MenuMusic/Logger.hpp"

#include <CP_SDK/ChatPlexSDK.hpp>
#include <CP_SDK/ModuleBase.hpp>

using namespace UnityEngine;

namespace ChatPlexMod_MenuMusic {

    std::optional<bool> ModulePresence::m_ChatRequest;

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    bool ModulePresence::ChatRequest()
    {
        if (!m_ChatRequest.has_value())
        {
            auto& l_Modules = CP_SDK::ChatPlexSDK::GetModules();
            if (std::count_if(l_Modules.begin(), l_Modules.end(), [](auto x) { return x->Name() == u"Chat Request"; }))
                m_ChatRequest = true;
            else
                m_ChatRequest = false;
        }

        return m_ChatRequest.value();
    }

} ///< namespace ChatPlexMod_MenuMusic