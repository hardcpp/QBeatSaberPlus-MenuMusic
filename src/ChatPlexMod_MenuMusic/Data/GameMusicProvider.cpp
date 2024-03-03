#include "ChatPlexMod_MenuMusic/Data/GameMusicProvider.hpp"
#include "ChatPlexMod_MenuMusic/Data/Music.hpp"
#include "ChatPlexMod_MenuMusic/Logger.hpp"

#include <CP_SDK/Unity/MTCoroutineStarter.hpp>
#include <CP_SDK_BS/Game/LevelSelection.hpp>
#include <songloader/shared/API.hpp>

#include <filesystem>

#include <GlobalNamespace/CustomPreviewBeatmapLevel.hpp>
#include <UnityEngine/Random.hpp>

using namespace GlobalNamespace;
using namespace UnityEngine;

namespace ChatPlexMod_MenuMusic { namespace Data {

    MusicProviderType::E GameMusicProvider::Type()
    {
        return MusicProviderType::E::GameMusic;
    }
    bool GameMusicProvider::IsReady()
    {
        return !m_IsLoading;
    }
    bool GameMusicProvider::SupportPlayIt()
    {
        return true;
    }
    const std::vector<std::shared_ptr<Music>>& GameMusicProvider::Musics()
    {
        return m_Musics;
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Constructor
    GameMusicProvider::GameMusicProvider()
        : IMusicProvider(CP_SDK_PRIV_TAG_VAL())
    {
        m_IsLoading = true;
    }
    /// @brief Destructor
    GameMusicProvider::~GameMusicProvider()
    {
        m_Musics.clear();
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Init
    /// @param p_SelfPtr Self ptr
    void GameMusicProvider::Init(const Ptr& p_SelfPtr)
    {
        CP_SDK::Unity::MTCoroutineStarter::EnqueueFromThread(custom_types::Helpers::CoroutineHelper::New(Coroutine_LoadGameSongs(p_SelfPtr)));
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Per game implementation of the Play It button
    /// @param p_Music Target music
    bool GameMusicProvider::StartGameSpecificGamePlay(const std::shared_ptr<Music>& p_Music)
    {
        if (!p_Music || !RuntimeSongLoader::API::HasLoadedSongs())
            return false;

        auto l_CustomPreviewBeatmapLevel = RuntimeSongLoader::API::GetLevelById(p_Music->GetCustomData());
        if (!l_CustomPreviewBeatmapLevel)
            return false;

        CP_SDK_BS::Game::LevelSelection::FilterToSpecificSong(l_CustomPreviewBeatmapLevel.value());
        return true;
    }
    /// @brief Shuffle music collection
    void GameMusicProvider::Shuffle()
    {
        for (auto l_I = 0; l_I < m_Musics.size(); ++l_I)
        {
            auto l_Swapped  = m_Musics[l_I];
            auto l_NewIndex = Random::Range(l_I, m_Musics.size());

            m_Musics[l_I]           = m_Musics[l_NewIndex];
            m_Musics[l_NewIndex]    = l_Swapped;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Load game songs
    custom_types::Helpers::Coroutine GameMusicProvider::Coroutine_LoadGameSongs(Ptr p_Self)
    {
        co_yield nullptr;

        while (!RuntimeSongLoader::API::HasLoadedSongs())
            co_yield nullptr;

        auto l_Self = *reinterpret_cast<std::shared_ptr<GameMusicProvider>*>(&p_Self);
        try
        {
            const auto l_LoadedSongs = RuntimeSongLoader::API::GetLoadedSongs();
            for (auto& l_Current : l_LoadedSongs)
            {
                auto l_Extension = std::filesystem::path(l_Current->standardLevelInfoSaveData->songFilename).extension().string();
                std::transform(l_Extension.begin(), l_Extension.end(), l_Extension.begin(), ::tolower);

                if (l_Extension != ".egg" && l_Extension != ".ogg")
                    continue;

                std::u16string l_Prefix = l_Current->customLevelPath.operator std::__ndk1::u16string() + u"/";
                l_Self->m_Musics.push_back(std::shared_ptr<Music>(new Music(
                    l_Self,
                    l_Prefix + l_Current->standardLevelInfoSaveData->songFilename,
                    l_Prefix + l_Current->standardLevelInfoSaveData->coverImageFilename,
                    l_Current->songName,
                    l_Current->songAuthorName,
                    l_Current->levelID.operator std::__ndk1::string()
                )));
            }

            l_Self->Shuffle();

            l_Self->m_IsLoading = false;
        }
        catch(const std::exception& p_Exception)
        {
            Logger::Instance->Error(u"[ChatPlexMod_MenuMusic.Data][GameMusicProvider.Coroutine_LoadGameSongs] Error:");
            Logger::Instance->Error(p_Exception);
        }
    }

}   ///< namespace Data
}   ///< namespace ChatPlexMod_MenuMusic