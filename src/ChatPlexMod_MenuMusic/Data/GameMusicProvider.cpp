#include "ChatPlexMod_MenuMusic/Data/GameMusicProvider.hpp"
#include "ChatPlexMod_MenuMusic/Data/Music.hpp"
#include "ChatPlexMod_MenuMusic/Logger.hpp"

#include <CP_SDK/Unity/MTCoroutineStarter.hpp>
#include <CP_SDK_BS/Game/LevelSelection.hpp>
#include <CP_SDK_BS/Game/Levels.hpp>
#include <songcore/shared/SongCore.hpp>

#include <filesystem>
#include <cctype>

#include <GlobalNamespace/BeatmapLevel.hpp>
#include <GlobalNamespace/FileSystemPreviewMediaData.hpp>
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
        BeatmapLevel* l_BeatmapLevel = nullptr;
        if (!p_Music || !CP_SDK_BS::Game::Levels::TryGetBeatmapLevelForLevelID(p_Music->GetCustomData(), &l_BeatmapLevel))
            return false;

        CP_SDK_BS::Game::LevelSelection::FilterToSpecificSong(l_BeatmapLevel);
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

        while (!SongCore::API::Loading::AreSongsLoaded())
            co_yield nullptr;

        auto l_Self = std::static_pointer_cast<GameMusicProvider>(p_Self);
        try
        {
            const auto l_LoadedSongs = SongCore::API::Loading::GetAllLevels();
            std::size_t l_ProcessedSongs = 0;
            for (auto& l_Current : l_LoadedSongs)
            {
                if (++l_ProcessedSongs % 32 == 0)
                    co_yield nullptr;

                auto l_FileSystemPreviewMediaData = il2cpp_utils::try_cast<FileSystemPreviewMediaData>(l_Current->previewMediaData);
                if (!l_FileSystemPreviewMediaData.has_value())
                    continue;

                auto l_Extension = std::filesystem::path(l_FileSystemPreviewMediaData.value()->_previewAudioClipPath).extension().string();
                std::transform(l_Extension.begin(), l_Extension.end(), l_Extension.begin(), ::tolower);

                if (l_Extension != ".egg" && l_Extension != ".ogg")
                    continue;

                l_Self->m_Musics.push_back(std::shared_ptr<Music>(new Music(
                    l_Self,
                    l_FileSystemPreviewMediaData.value()->_previewAudioClipPath,
                    l_FileSystemPreviewMediaData.value()->_coverSpritePath,
                    l_Current->songName,
                    l_Current->songAuthorName,
                    l_Current->levelID.operator std::__ndk1::u16string_view()
                )));
            }

            l_Self->Shuffle();

        }
        catch(const std::exception& p_Exception)
        {
            Logger::Instance->Error(u"[ChatPlexMod_MenuMusic.Data][GameMusicProvider.Coroutine_LoadGameSongs] Error:");
            Logger::Instance->Error(p_Exception);
        }

        /// Always release waiters, even if one malformed song caused an exception.
        l_Self->m_IsLoading = false;
    }

}   ///< namespace Data
}   ///< namespace ChatPlexMod_MenuMusic
