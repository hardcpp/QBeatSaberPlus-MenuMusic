#include "ChatPlexMod_MenuMusic/Data/CustomMusicProvider.hpp"
#include "ChatPlexMod_MenuMusic/Data/Music.hpp"
#include "ChatPlexMod_MenuMusic/Logger.hpp"

#include <filesystem>

#include <CP_SDK/Unity/MTCoroutineStarter.hpp>

namespace ChatPlexMod_MenuMusic { namespace Data {

    MusicProviderType::E CustomMusicProvider::Type()
    {
        return MusicProviderType::E::GameMusic;
    }
    bool CustomMusicProvider::IsReady()
    {
        return !m_IsLoading;
    }
    bool CustomMusicProvider::SupportPlayIt()
    {
        return false;
    }
    const std::vector<std::shared_ptr<Music>>& CustomMusicProvider::Musics()
    {
        return m_Musics;
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Constructor
    CustomMusicProvider::CustomMusicProvider()
        : IMusicProvider(CP_SDK_PRIV_TAG_VAL())
    {
        m_IsLoading = true;
    }
    /// @brief Destructor
    CustomMusicProvider::~CustomMusicProvider()
    {
        m_Musics.clear();
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Init
    /// @param p_SelfPtr Self ptr
    void CustomMusicProvider::Init(const Ptr& p_SelfPtr)
    {
        CP_SDK::Unity::MTCoroutineStarter::EnqueueFromThread(custom_types::Helpers::CoroutineHelper::New(Coroutine_Load(p_SelfPtr)));
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Per game implementation of the Play It button
    /// @param p_Music Target music
    bool CustomMusicProvider::StartGameSpecificGamePlay(const std::shared_ptr<Music>& p_Music)
    {
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Load game songs
    custom_types::Helpers::Coroutine CustomMusicProvider::Coroutine_Load(Ptr p_Self)
    {
        auto l_Self = *reinterpret_cast<std::shared_ptr<CustomMusicProvider>*>(&p_Self);

        co_yield nullptr;

        try
        {
            auto l_BaseDirectory = CP_SDK::ChatPlexSDK::BasePath() + CP_SDK::ChatPlexSDK::ProductName() + "/MenuMusic/CustomMusic";
            auto l_Files         = std::vector<std::string>();

            if (!std::filesystem::exists(l_BaseDirectory))
                std::filesystem::create_directories(l_BaseDirectory);

            for (const auto& l_Entry : std::filesystem::directory_iterator(l_BaseDirectory))
            {
                if (!l_Entry.is_regular_file())
                    continue;

                auto l_Path         = l_Entry.path();
                auto l_Extension    = l_Path.extension();
                if (   l_Extension != ".egg" && l_Extension != ".ogg"
                    && l_Extension != ".EGG" && l_Extension != ".OGG")
                    continue;

                auto        l_PathTest  = l_Path;
                std::string l_CoverPath;

                if (std::filesystem::exists(l_PathTest.replace_extension(".jpg")))
                    l_CoverPath = l_PathTest.replace_extension(".jpg").string();
                else if (std::filesystem::exists(l_PathTest.replace_extension(".png")))
                    l_CoverPath = l_PathTest.replace_extension(".png").string();

                l_Self->m_Musics.push_back(std::shared_ptr<Music>(new Music(
                    l_Self,
                    CP_SDK::Utils::StrToU16Str(l_Path.string()),
                    CP_SDK::Utils::StrToU16Str(l_CoverPath),
                    CP_SDK::Utils::StrToU16Str(l_Path.stem().string()),
                    u" ",
                    ""
                )));
            }

        }
        catch (const std::exception& l_Exception)
        {
            Logger::Instance->Error(u"[ChatPlexMod_MenuMusic.Data][CustomMusicProvider.Coroutine_Load] Can't load audio! Exception:");
            Logger::Instance->Error(l_Exception);
        }

        l_Self->m_IsLoading = false;
    }

}   ///< namespace Data
}   ///< namespace ChatPlexMod_MenuMusic