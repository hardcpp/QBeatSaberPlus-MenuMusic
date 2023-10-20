#include "ChatPlexMod_MenuMusic/Logger.hpp"
#include "ChatPlexMod_MenuMusic/MenuMusic.hpp"

#include <CP_SDK/ChatPlexSDK.hpp>
#include <CP_SDK/Logging/BMBFLogger.hpp>

#include <beatsaber-hook/shared/utils/il2cpp-functions.hpp>
#include <custom-types/shared/register.hpp>

static ModInfo s_ModInfo;

// Called at the early stages of game loading
extern "C" void setup(ModInfo & p_ModInfo)
{
    p_ModInfo.id        = "QBeatSaberPlus-MenuMusic";
    p_ModInfo.version   = VERSION;

    s_ModInfo = p_ModInfo;

    auto l_Logger = new CP_SDK::Logging::BMBFLogger(new Logger(p_ModInfo));

    l_Logger->Error(u"QBeatSaberPlus-MenuMusic Setuping!");

    ChatPlexMod_MenuMusic::Logger::Instance = l_Logger;
    CP_SDK::ChatPlexSDK::RegisterModule(new ChatPlexMod_MenuMusic::MenuMusic());

    l_Logger->Error(u"QBeatSaberPlus-MenuMusic Completed setup!");
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

// Called later on in the game loading - a good time to install function hooks
extern "C" void load()
{
    il2cpp_functions::Init();

    ChatPlexMod_MenuMusic::Logger::Instance->Error(u"Registering custom types.");
    custom_types::Register::AutoRegister();
}