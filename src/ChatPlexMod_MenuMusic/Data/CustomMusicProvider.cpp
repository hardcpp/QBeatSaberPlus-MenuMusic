#include "ChatPlexMod_MenuMusic/Data/CustomMusicProvider.hpp"
#include "ChatPlexMod_MenuMusic/Data/Music.hpp"
#include "ChatPlexMod_MenuMusic/Logger.hpp"

#include <filesystem>
#include <atomic>
#include <string>
#include <utility>
#include <vector>

#include <CP_SDK/Unity/MTCoroutineStarter.hpp>
#include <CP_SDK/Unity/MTThreadInvoker.hpp>

#include <UnityEngine/Random.hpp>

using namespace UnityEngine;

namespace ChatPlexMod_MenuMusic { namespace Data {

    namespace {

        struct CustomMusicScanResult
        {
            std::vector<std::shared_ptr<Music>> Musics;
            std::string                         Error;
            std::atomic_bool                    IsDone = false;
        };

    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    MusicProviderType::E CustomMusicProvider::Type()
    {
        return MusicProviderType::E::CustomMusic;
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
    /// @brief Shuffle music collection
    void CustomMusicProvider::Shuffle()
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
    custom_types::Helpers::Coroutine CustomMusicProvider::Coroutine_Load(Ptr p_Self)
    {
        auto l_Self = std::static_pointer_cast<CustomMusicProvider>(p_Self);

        co_yield nullptr;

        auto l_BaseDirectory = std::filesystem::path(
            CP_SDK::ChatPlexSDK::BasePath() + CP_SDK::ChatPlexSDK::ProductName() + "/MenuMusic/CustomMusic"
        );
        auto l_ScanResult = std::make_shared<CustomMusicScanResult>();

        /// Directory IO can block Quest storage; keep it off the Unity frame thread.
        CP_SDK::Unity::MTThreadInvoker::EnqueueOnThread([l_BaseDirectory, l_ScanResult, l_Self]() -> void
        {
            try
            {
                if (!std::filesystem::exists(l_BaseDirectory))
                    std::filesystem::create_directories(l_BaseDirectory);

                for (const auto& l_Entry : std::filesystem::directory_iterator(l_BaseDirectory))
                {
                    if (!l_Entry.is_regular_file())
                        continue;

                    auto l_Path      = l_Entry.path();
                    auto l_Extension = l_Path.extension();
                    if (   l_Extension != ".egg" && l_Extension != ".ogg"
                        && l_Extension != ".EGG" && l_Extension != ".OGG")
                        continue;

                    auto l_PathTest = l_Path;
                    std::filesystem::path l_CoverPath;

                    l_PathTest.replace_extension(".jpg");
                    if (std::filesystem::exists(l_PathTest))
                        l_CoverPath = l_PathTest;
                    else
                    {
                        l_PathTest = l_Path;
                        l_PathTest.replace_extension(".png");
                        if (std::filesystem::exists(l_PathTest))
                            l_CoverPath = l_PathTest;
                    }

                    l_ScanResult->Musics.emplace_back(std::make_shared<Music>(
                        l_Self,
                        CP_SDK::Utils::StrToU16Str(l_Path.string()),
                        CP_SDK::Utils::StrToU16Str(l_CoverPath.string()),
                        CP_SDK::Utils::StrToU16Str(l_Path.stem().string()),
                        u" ",
                        u""
                    ));
                }
            }
            catch (const std::exception& l_Exception)
            {
                l_ScanResult->Error = l_Exception.what();
            }

            l_ScanResult->IsDone.store(true, std::memory_order_release); });

        while (!l_ScanResult->IsDone.load(std::memory_order_acquire))
            co_yield nullptr;

        if (!l_ScanResult->Error.empty())
            Logger::Instance->Error(u"[ChatPlexMod_MenuMusic.Data][CustomMusicProvider.Coroutine_Load] Can't scan custom music: " + CP_SDK::Utils::StrToU16Str(l_ScanResult->Error));

        std::size_t l_ProcessedFiles = 0;
        l_Self->m_Musics.swap(l_ScanResult->Musics);
        l_Self->Shuffle();

        l_Self->m_IsLoading = false;
    }

}   ///< namespace Data
}   ///< namespace ChatPlexMod_MenuMusic
