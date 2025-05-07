#include "git_info.h"
#include "ChatPlexMod_MenuMusic/Logger.hpp"
#include "ChatPlexMod_MenuMusic/MenuMusic.hpp"

#include <CP_SDK/ChatPlexSDK.hpp>
#include <CP_SDK/Logging/PaperLogger.hpp>

#include <beatsaber-hook/shared/utils/il2cpp-functions.hpp>
#include <custom-types/shared/register.hpp>

static modloader::ModInfo s_ModInfo{"QBeatSaberPlus-MenuMusic", VERSION, GIT_COMMIT};

// Called at the early stages of game loading
extern "C" __attribute__((visibility("default"))) void setup(CModInfo* p_ModInfo)
{
    p_ModInfo->id           = s_ModInfo.id.c_str();
    p_ModInfo->version      = s_ModInfo.version.c_str();
    p_ModInfo->version_long = s_ModInfo.versionLong;

    auto l_Logger = new CP_SDK::Logging::PaperLogger(p_ModInfo->id);

    l_Logger->Error(u"QBeatSaberPlus-MenuMusic Setuping!");

    ChatPlexMod_MenuMusic::Logger::Instance = l_Logger;
    CP_SDK::ChatPlexSDK::RegisterModule(new ChatPlexMod_MenuMusic::MenuMusic());

    l_Logger->Error(u"QBeatSaberPlus-MenuMusic Completed setup!");
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

// Called later on in the game loading - a good time to install function hooks
extern "C" __attribute__((visibility("default"))) void late_load()
{
    il2cpp_functions::Init();

    ChatPlexMod_MenuMusic::Logger::Instance->Error(u"Registering custom types.");
    custom_types::Register::AutoRegister();
}