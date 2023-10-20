#include "ChatPlexMod_MenuMusic/MMConfig.hpp"

#include <CP_SDK/ChatPlexSDK.hpp>

namespace ChatPlexMod_MenuMusic {

    CP_SDK_CONFIG_JSONCONFIG_INSTANCE_IMPL(MMConfig);

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Reset config to default
    void MMConfig::Reset_Impl()
    {
        MusicProvider = Data::MusicProviderType::E::GameMusic;

        ShowPlayer = true;

        StartSongFromBeginning          = false;
        StartANewMusicOnSceneChange     = true;
        LoopCurrentMusic                = false;
        UseOnlyCustomMenuSongsFolder    = false;
        PlaybackVolume                  = 0.5f;
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Write the document
    /// @param p_Document Target
    CP_SDK_JSON_SERIALIZE_IMPL(MMConfig)
    {
        CP_SDK_JSON_SERIALIZE_BOOL(Enabled);
        CP_SDK_JSON_SERIALIZE_ENUM(MusicProvider, Data::MusicProviderType);
        CP_SDK_JSON_SERIALIZE_BOOL(ShowPlayer);
        CP_SDK_JSON_SERIALIZE_BOOL(StartSongFromBeginning);
        CP_SDK_JSON_SERIALIZE_BOOL(StartANewMusicOnSceneChange);
        CP_SDK_JSON_SERIALIZE_BOOL(LoopCurrentMusic);
        CP_SDK_JSON_SERIALIZE_BOOL(UseOnlyCustomMenuSongsFolder);
        CP_SDK_JSON_SERIALIZE_FLOAT(PlaybackVolume);
    }
    /// @brief Read the document
    /// @param p_Document Source
    CP_SDK_JSON_UNSERIALIZE_IMPL(MMConfig)
    {
        CP_SDK_JSON_UNSERIALIZE_BOOL(Enabled);
        CP_SDK_JSON_UNSERIALIZE_ENUM(MusicProvider, Data::MusicProviderType, Data::MusicProviderType::E::GameMusic);
        CP_SDK_JSON_UNSERIALIZE_BOOL(ShowPlayer);
        CP_SDK_JSON_UNSERIALIZE_BOOL(StartSongFromBeginning);
        CP_SDK_JSON_UNSERIALIZE_BOOL(StartANewMusicOnSceneChange);
        CP_SDK_JSON_UNSERIALIZE_BOOL(LoopCurrentMusic);
        CP_SDK_JSON_UNSERIALIZE_BOOL(UseOnlyCustomMenuSongsFolder);
        CP_SDK_JSON_UNSERIALIZE_FLOAT(PlaybackVolume);
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Get relative config path
    std::filesystem::path MMConfig::GetRelativePath()
    {
        return std::filesystem::path("Q" + CP_SDK::Utils::U16StrToStr(CP_SDK::ChatPlexSDK::ProductName()) + "Plus") / "MenuMusic" / "Config";
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief On config init
    /// @param p_OnCreation On creation
    void MMConfig::OnInit(bool p_OnCreation)
    {

    }

}   ///< namespace ChatPlexMod_MenuMusic