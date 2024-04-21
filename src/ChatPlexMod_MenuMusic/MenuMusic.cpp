#include "ChatPlexMod_MenuMusic/MenuMusic.hpp"
#include "ChatPlexMod_MenuMusic/Data/CustomMusicProvider.hpp"
#include "ChatPlexMod_MenuMusic/Data/GameMusicProvider.hpp"
#include "ChatPlexMod_MenuMusic/Logger.hpp"

#include <CP_SDK/UI/FlowCoordinators/MainFlowCoordinator.hpp>

#include <GlobalNamespace/SongPreviewPlayer.hpp>
#include <System/Environment.hpp>
#include <System/Random.hpp>
#include <UnityEngine/Application.hpp>
#include <UnityEngine/AudioDataLoadState.hpp>
#include <UnityEngine/AudioSource.hpp>
#include <UnityEngine/Mathf.hpp>
#include <UnityEngine/Random.hpp>
#include <UnityEngine/Resources.hpp>
#include <UnityEngine/WaitForSeconds.hpp>
#include <UnityEngine/WaitForSecondsRealtime.hpp>

namespace ChatPlexMod_MenuMusic {

    using namespace GlobalNamespace;
    using namespace UnityEngine;
    using namespace System::Collections;

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    CP_SDK_MODULE_BASE_INSTANCE_IMPL(MenuMusic);

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// Constructor
    MenuMusic::MenuMusic()
    {
        m_OriginalAmbientVolumeScale    = 1.0f;
        m_BackupTime                    = 0.0f;
        m_IsPaused                      = false;

        m_CurrentSongIndex              = 0;

        m_FastCancellationToken         = CP_SDK::Misc::FastCancellationToken::Make();
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Enable the Module
    void MenuMusic::OnEnable()
    {
        /// Bind event
        CP_SDK::ChatPlexSDK::OnGenericSceneChange += {this, &MenuMusic::ChatPlexSDK_OnGenericSceneChange};
        /// Application.wantsToQuit                             += Application_wantsToQuit;

        /// Try to find existing preview player
        m_PreviewPlayer = Resources::FindObjectsOfTypeAll<SongPreviewPlayer*>()->FirstOrDefault();

        /// Backup original settings
        if (m_PreviewPlayer)
        {
            m_OriginalMenuMusic             = m_PreviewPlayer->____defaultAudioClip;
            m_OriginalAmbientVolumeScale    = m_PreviewPlayer->____ambientVolumeScale;
        }

        UpdateMusicProvider();

        /// Enable at start if in menu
        if (CP_SDK::ChatPlexSDK::ActiveGenericScene() == CP_SDK::EGenericScene::Menu)
            ChatPlexSDK_OnGenericSceneChange(CP_SDK::EGenericScene::Menu);
    }
    /// @brief Disable the Module
    void MenuMusic::OnDisable()
    {
        /// Unbind event
        //Application.wantsToQuit                             -= Application_wantsToQuit;
        CP_SDK::ChatPlexSDK::OnGenericSceneChange -= {this, &MenuMusic::ChatPlexSDK_OnGenericSceneChange};

        /// Stop wait and play next song coroutine
        if (m_WaitAndPlayNextSongCoroutine)
        {
            CP_SDK::Unity::MTCoroutineStarter::Stop(m_WaitAndPlayNextSongCoroutine.Ptr());
            m_WaitAndPlayNextSongCoroutine = nullptr;
        }

        /// Destroy floating window
        DestroyFloatingPlayer();

        CP_SDK::UI::UISystem::DestroyUI(&m_SettingsLeftView);
        CP_SDK::UI::UISystem::DestroyUI(&m_SettingsMainView);

        /// Restore original settings
        if (!m_WantsToQuit && m_PreviewPlayer && m_OriginalMenuMusic)
        {
            m_PreviewPlayer->____defaultAudioClip   = m_OriginalMenuMusic.Ptr();
            m_PreviewPlayer->____ambientVolumeScale = m_OriginalAmbientVolumeScale;
            m_PreviewPlayer->CrossfadeToDefault();
        }
        else if (m_WantsToQuit && m_PreviewPlayer)
            m_PreviewPlayer->PauseCurrentChannel();
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Get Module settings UI
    MenuMusic::t_InitialViews MenuMusic::GetSettingsViewControllers()
    {
        if (!m_SettingsMainView) m_SettingsMainView = CP_SDK::UI::UISystem::CreateViewController<UI::SettingsMainView*>();
        if (!m_SettingsLeftView) m_SettingsLeftView = CP_SDK::UI::UISystem::CreateViewController<UI::SettingsLeftView*>();

        return { m_SettingsMainView.Ptr(), m_SettingsLeftView.Ptr(), nullptr};
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief When the active scene change
    /// @param p_Scene Scene type
    void MenuMusic::ChatPlexSDK_OnGenericSceneChange(CP_SDK::EGenericScene p_Scene)
    {
        /// Skip if it's not the menu
        if (p_Scene != CP_SDK::EGenericScene::Menu)
        {
            if (m_PreviewPlayer && m_OriginalMenuMusic)
                m_PreviewPlayer->____defaultAudioClip = m_OriginalMenuMusic.Ptr(false);

            DestroyFloatingPlayer();
            return;
        }

        /// Create player window
        if (MMConfig::Instance()->ShowPlayer)
            CreateFloatingPlayer();

        m_PreviewPlayer->____ambientVolumeScale = 0.0f;
        m_PreviewPlayer->____volumeScale        = 0.0f;

        /// Start a new music
        if (MMConfig::Instance()->StartANewMusicOnSceneChange)
            StartNewMusic(false, true);
        else
            LoadNextMusic(true);
    }
    /// @brief Application wants to quit
    bool MenuMusic::Application_wantsToQuit()
    {
        m_WantsToQuit = true;
        if (m_PreviewPlayer)
            m_PreviewPlayer->PauseCurrentChannel();
        return true;
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Update the music provider
    void MenuMusic::UpdateMusicProvider()
    {
        switch (MMConfig::Instance()->MusicProvider)
        {
            case Data::MusicProviderType::E::CustomMusic:
                m_MusicProvider = std::make_shared<Data::CustomMusicProvider>();
                m_MusicProvider->Init(m_MusicProvider);
                break;

            default:
                m_MusicProvider = std::make_shared<Data::GameMusicProvider>();
                m_MusicProvider->Init(m_MusicProvider);
                break;
        }

        StartNewMusic();
    }
    /// @brief Update playback volume
    /// @param p_FromConfig From config?
    void MenuMusic::UpdatePlaybackVolume(bool p_FromConfig)
    {
        if (!m_PreviewPlayer)
            return;

        auto& l_Modules = CP_SDK::ChatPlexSDK::GetModules();
        if (std::count_if(l_Modules.begin(), l_Modules.end(), [](auto x) { return x->Name() == u"Audio Tweaker"; }))
        {
            auto l_ChannelsController = m_PreviewPlayer->____audioSourceControllers;
            if (l_ChannelsController && l_ChannelsController.size())
            {
                for (auto l_I = 0; l_I < l_ChannelsController->get_Length(); ++l_I)
                {
                    auto l_ChannelController = l_ChannelsController[l_I];
                    auto l_Channel           = l_ChannelController->___audioSource;
                    if (l_Channel->get_isPlaying() && l_Channel->get_clip() == m_CurrentMusicAudioClip)
                    {
                        m_PreviewPlayer->____ambientVolumeScale = 1.0f;
                        m_PreviewPlayer->____volumeScale        = 1.0f;
                    }
                }
            }
        }
        else
        {
            m_PreviewPlayer->____ambientVolumeScale = MMConfig::Instance()->PlaybackVolume;
            m_PreviewPlayer->____volumeScale        = MMConfig::Instance()->PlaybackVolume;
        }

        if (p_FromConfig && m_PlayerFloatingPanelView)
            m_PlayerFloatingPanelView->UpdateVolume();

        if (!p_FromConfig && m_SettingsMainView && m_SettingsMainView->CanBeUpdated())
            m_SettingsMainView->UpdateVolume();

        MMConfig::Instance()->Save();
    }
    /// Update player
    void MenuMusic::UpdatePlayer()
    {
        if (MMConfig::Instance()->ShowPlayer && !m_PlayerFloatingPanel)
            CreateFloatingPlayer();
        else if (!MMConfig::Instance()->ShowPlayer && m_PlayerFloatingPanel)
            DestroyFloatingPlayer();
    }
    /// Toggle pause status
    void MenuMusic::TogglePause()
    {
        m_IsPaused = !m_IsPaused;

        if (m_PlayerFloatingPanelView)
            m_PlayerFloatingPanelView->SetIsPaused(m_IsPaused);
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Create floating player window
    void MenuMusic::CreateFloatingPlayer()
    {
        if (m_PlayerFloatingPanel || m_CreateFloatingPlayerCoroutine)
            return;

        m_CreateFloatingPlayerCoroutine = CP_SDK::Unity::MTCoroutineStarter::Start(custom_types::Helpers::CoroutineHelper::New(
            CreateFloatingPlayer_Coroutine()
        ));
    }
    /// @brief Create floating player window
    custom_types::Helpers::Coroutine MenuMusic::CreateFloatingPlayer_Coroutine()
    {
        co_yield nullptr;

        if (m_Instance->m_PlayerFloatingPanel)
        {
            m_Instance->m_CreateFloatingPlayerCoroutine = nullptr;
            co_return;
        }

        auto l_ScreenContainer  = (GameObject*)nullptr;
        auto l_Waiter           = WaitForSecondsRealtime::New_ctor(0.25f)->i___System__Collections__IEnumerator();
        while (true)
        {
            if (CP_SDK::ChatPlexSDK::ActiveGenericScene() != CP_SDK::EGenericScene::Menu)
            {
                m_Instance->m_CreateFloatingPlayerCoroutine = nullptr;
                co_return;
            }

            l_ScreenContainer = Resources::FindObjectsOfTypeAll<GameObject*>()->FirstOrDefault([](GameObject* x) -> bool {
                return CP_SDK::Utils::IsUnityPtrValid(x) && x->get_activeInHierarchy() && x->get_name() == u"ScreenContainer";
            });

            if (l_ScreenContainer != nullptr && l_ScreenContainer)
                break;

            co_yield l_Waiter;
        }

        auto l_PlayerPosition = Vector3(-140.0f, 55.0f, 0.0f);
        try
        {
            m_Instance->m_PlayerFloatingPanel = CP_SDK::UI::UISystem::FloatingPanelFactory->Create(u"ChatPlexMod_MenuMusic", l_ScreenContainer->get_transform());
            m_Instance->m_PlayerFloatingPanel->SetSize(Vector2(80.0f, 20.0f));
            m_Instance->m_PlayerFloatingPanel->SetRadius(140.0f);
            m_Instance->m_PlayerFloatingPanel->SetTransformDirect(l_PlayerPosition, Vector3(0.0f, 0.0f, 0.0f));
            m_Instance->m_PlayerFloatingPanel->SetBackground(false);
            m_Instance->m_PlayerFloatingPanel->RTransform()->set_localScale(Vector3::get_one());

            m_Instance->m_PlayerFloatingPanelView = CP_SDK::UI::UISystem::CreateViewController<UI::PlayerFloatingPanel*>();
            m_Instance->m_PlayerFloatingPanel->SetViewController(m_Instance->m_PlayerFloatingPanelView.Ptr());
            m_Instance->m_PlayerFloatingPanel->SetGearIcon(CP_SDK::UI::Components::CFloatingPanel::ECorner::TopRight);
            m_Instance->m_PlayerFloatingPanel->OnGearIcon([](const CP_SDK::Utils::MonoPtr<CP_SDK::UI::Components::CFloatingPanel>&) -> void
            {
                auto l_Items = Instance()->GetSettingsViewControllers();
                CP_SDK::UI::FlowCoordinators::MainFlowCoordinator::Instance()->Present();
                CP_SDK::UI::FlowCoordinators::MainFlowCoordinator::Instance()->ChangeViewControllers(l_Items.Main, l_Items.Left, l_Items.Right);
            });

            m_Instance->m_PlayerFloatingPanelView->OnMusicChanged(m_Instance->m_CurrentMusic);
            m_Instance->m_PlayerFloatingPanelView->SetIsPaused(m_Instance->m_IsPaused);
        }
        catch (const std::exception& l_Exception)
        {
            Logger::Instance->Error(u"[MenuMusic] Failed to CreateFloatingPlayer");
            Logger::Instance->Error(l_Exception);
        }

        m_Instance->m_CreateFloatingPlayerCoroutine = nullptr;
    }
    /// @brief Destroy floating player window
    void MenuMusic::DestroyFloatingPlayer()
    {
        try
        {
            if (m_CreateFloatingPlayerCoroutine)
            {
                CP_SDK::Unity::MTCoroutineStarter::Stop(m_CreateFloatingPlayerCoroutine.Ptr());
                m_CreateFloatingPlayerCoroutine = nullptr;
            }

            CP_SDK::UI::UISystem::DestroyUI(&m_PlayerFloatingPanel, &m_PlayerFloatingPanelView);
        }
        catch (const std::exception& l_Exception)
        {
            Logger::Instance->Error(u"[MenuMusic] Failed to DestroyFloatingPlayer");
            Logger::Instance->Error(l_Exception);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Start a previous music
    void MenuMusic::StartPreviousMusic()
    {
        if (!m_MusicProvider->IsReady())
        {
            if (m_WaitUntillReadyCoroutine)
                CP_SDK::Unity::MTCoroutineStarter::Stop(m_WaitUntillReadyCoroutine.Ptr());

            m_WaitUntillReadyCoroutine = CP_SDK::Unity::MTCoroutineStarter::Start(custom_types::Helpers::CoroutineHelper::New(
                Coroutine_WaitUntilReady({this, &MenuMusic::StartPreviousMusic})
            ));
            return;
        }

        /// Decrement for next song
        m_CurrentSongIndex--;

        /// Handle overflow
        if (m_CurrentSongIndex < 0)                                 m_CurrentSongIndex = m_MusicProvider->Musics().size() - 1;
        if (m_CurrentSongIndex >= m_MusicProvider->Musics().size()) m_CurrentSongIndex = 0;

        /// Load and play audio clip
        LoadNextMusic(false);
    }
    /// @brief Start a new music
    /// @param p_Random            Pick a random song?
    /// @param p_OnSceneTransition On scene transition?
    void MenuMusic::StartNewMusic(bool p_Random, bool p_OnSceneTransition)
    {
        if (!m_MusicProvider->IsReady())
        {
            if (m_WaitUntillReadyCoroutine)
                CP_SDK::Unity::MTCoroutineStarter::Stop(m_WaitUntillReadyCoroutine.Ptr());

            m_WaitUntillReadyCoroutine = CP_SDK::Unity::MTCoroutineStarter::Start(custom_types::Helpers::CoroutineHelper::New(
                Coroutine_WaitUntilReady([this, p_Random, p_OnSceneTransition]() -> void { StartNewMusic(p_Random, p_OnSceneTransition);  }))
            );
            return;
        }

        if (p_Random)
            m_MusicProvider->Shuffle();

        m_CurrentSongIndex++;

        /// Load and play audio clip
        LoadNextMusic(p_OnSceneTransition);
    }
    /// @brief Start a next music
    void MenuMusic::StartNextMusic()
    {
        if (!m_MusicProvider->IsReady())
        {
            if (m_WaitUntillReadyCoroutine)
                CP_SDK::Unity::MTCoroutineStarter::Stop(m_WaitUntillReadyCoroutine.Ptr());

            m_WaitUntillReadyCoroutine = CP_SDK::Unity::MTCoroutineStarter::Start(custom_types::Helpers::CoroutineHelper::New(
                Coroutine_WaitUntilReady({this, &MenuMusic::StartNextMusic})
            ));
            return;
        }

        /// Increment for next song
        m_CurrentSongIndex++;

        /// Load and play audio clip
        LoadNextMusic(false);
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Load the next music
    /// @param p_OnSceneTransition Is on scene transition?
    void MenuMusic::LoadNextMusic(bool p_OnSceneTransition)
    {
        if (m_MusicProvider->Musics().empty())
            return;

        /// Handle overflow
        if (m_CurrentSongIndex < 0)                                 m_CurrentSongIndex = m_MusicProvider->Musics().size() - 1;
        if (m_CurrentSongIndex >= m_MusicProvider->Musics().size()) m_CurrentSongIndex = 0;

        auto& l_MusicToLoad = m_MusicProvider->Musics()[m_CurrentSongIndex];

        m_FastCancellationToken->Cancel();
        l_MusicToLoad->GetAudioAsync(
            m_FastCancellationToken,
            [=](const CP_SDK::Utils::MonoPtr<AudioClip>& p_AudioClip) -> void {
                CP_SDK::Unity::MTCoroutineStarter::EnqueueFromThread(custom_types::Helpers::CoroutineHelper::New(
                    Coroutine_LoadAudioClip(p_OnSceneTransition, l_MusicToLoad, p_AudioClip))
                );
            },
            {this, &MenuMusic::StartNextMusic}
        );
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Wait until music provider is ready
    /// @param p_Callback Callback action
    custom_types::Helpers::Coroutine MenuMusic::Coroutine_WaitUntilReady(CP_SDK::Utils::Action<> p_Callback)
    {
        co_yield nullptr;

        while (!m_Instance->m_MusicProvider || !m_Instance->m_MusicProvider->IsReady())
            co_yield nullptr;

        p_Callback();
    }
    /// @brief Load the song into the preview player
    /// @param p_OnSceneTransition On scene transition?
    custom_types::Helpers::Coroutine MenuMusic::Coroutine_LoadAudioClip(bool p_OnSceneTransition, Data::Music::Ptr p_Music, CP_SDK::Utils::MonoPtr<AudioClip> p_AudioClip)
    {
        if (m_Instance->m_WaitAndPlayNextSongCoroutine)
        {
            CP_SDK::Unity::MTCoroutineStarter::Stop(m_Instance->m_WaitAndPlayNextSongCoroutine.Ptr());
            m_Instance->m_WaitAndPlayNextSongCoroutine = nullptr;
        }

        co_yield nullptr;

        /// Skip if it's not the menu
        if (CP_SDK::ChatPlexSDK::ActiveGenericScene() != CP_SDK::EGenericScene::Menu)
            co_return;

        while (!m_Instance->m_PreviewPlayer)
        {
            co_yield nullptr;
            m_Instance->m_PreviewPlayer = Resources::FindObjectsOfTypeAll<SongPreviewPlayer*>()->First();
        }

        if (p_OnSceneTransition)
        {
            if (m_Instance->m_PreviewPlayer)
                m_Instance->m_PreviewPlayer->FadeOut(m_Instance->m_PreviewPlayer->____crossFadeToDefaultSpeed);

            co_yield WaitForSecondsRealtime::New_ctor(2.0f)->i___System__Collections__IEnumerator();
        }

        /// Skip if it's not the menu
        if (CP_SDK::ChatPlexSDK::ActiveGenericScene() != CP_SDK::EGenericScene::Menu)
            co_return;

        m_Instance->m_CurrentMusic          = p_Music;
        m_Instance->m_CurrentMusicAudioClip = p_AudioClip;

        if (m_Instance->m_PreviewPlayer && m_Instance->m_CurrentMusicAudioClip)
        {
            /// Wait that the song is loaded in background
            while (m_Instance->m_CurrentMusicAudioClip->get_loadState() != AudioDataLoadState::Loaded
                && m_Instance->m_CurrentMusicAudioClip->get_loadState() != AudioDataLoadState::Failed)
            {
                co_yield nullptr;
            }

            /// Check if we changed scene during loading
            if (!m_Instance->m_PreviewPlayer || CP_SDK::ChatPlexSDK::ActiveGenericScene() != CP_SDK::EGenericScene::Menu)
                co_return;

            if (m_Instance->m_CurrentMusicAudioClip->get_loadState() == AudioDataLoadState::Loaded)
            {
                bool l_Failed = false;

                try
                {
                    if (m_Instance->m_WaitAndPlayNextSongCoroutine)
                    {
                        CP_SDK::Unity::MTCoroutineStarter::Stop(m_Instance->m_WaitAndPlayNextSongCoroutine.Ptr());
                        m_Instance->m_WaitAndPlayNextSongCoroutine = nullptr;
                    }

                    m_Instance->m_PreviewPlayer->____defaultAudioClip = m_Instance->m_CurrentMusicAudioClip.Ptr();

                    auto  l_Volume  = MMConfig::Instance()->PlaybackVolume;
                    auto& l_Modules = CP_SDK::ChatPlexSDK::GetModules();
                    if (std::count_if(l_Modules.begin(), l_Modules.end(), [](auto x) { return x->Name() == u"Audio Tweaker"; }))
                        l_Volume = 1.0f;

                    m_Instance->m_PreviewPlayer->____ambientVolumeScale = l_Volume;
                    m_Instance->m_PreviewPlayer->____volumeScale        = l_Volume;

                    float l_StartTime = (MMConfig::Instance()->StartSongFromBeginning || m_Instance->m_CurrentMusicAudioClip->get_length() < 60)
                                        ?
                                            0.0f
                                        :
                                            std::max<float>(UnityEngine::Random::Range(m_Instance->m_CurrentMusicAudioClip->get_length() * 0.2f, m_Instance->m_CurrentMusicAudioClip->get_length() * 0.8f), 0.0f);

                    m_Instance->m_PreviewPlayer->CrossfadeTo(m_Instance->m_CurrentMusicAudioClip.Ptr(), l_Volume, l_StartTime, -1.0f, false, nullptr);

                    m_Instance->m_BackupTimeClip    = m_Instance->m_CurrentMusicAudioClip;
                    m_Instance->m_BackupTime        = l_StartTime;

                    if (m_Instance->m_PlayerFloatingPanelView)
                        m_Instance->m_PlayerFloatingPanelView->OnMusicChanged(p_Music);

                    m_Instance->m_WaitAndPlayNextSongCoroutine = CP_SDK::Unity::MTCoroutineStarter::Start(custom_types::Helpers::CoroutineHelper::New(
                        Coroutine_WaitAndPlayNextMusic(m_Instance->m_CurrentMusicAudioClip->get_length())
                    ));
                }
                catch (const std::exception& p_Exception)
                {
                    Logger::Instance->Error(u"Can't play audio! Exception: ");
                    Logger::Instance->Error(p_Exception);

                    l_Failed = true;
                }

                if (l_Failed)
                {
                    /// Wait until next try
                    co_yield WaitForSecondsRealtime::New_ctor(2.0f)->i___System__Collections__IEnumerator();

                    /// Try next music if loading failed
                    if (CP_SDK::ChatPlexSDK::ActiveGenericScene() == CP_SDK::EGenericScene::Menu)
                        m_Instance->StartNextMusic();

                    co_return;
                }
            }
            /// Try next music if loading failed
            else
                m_Instance->StartNextMusic();
        }
    }
    /// @brief Wait and play next music
    /// @param p_WaitTime Time to wait
    custom_types::Helpers::Coroutine MenuMusic::Coroutine_WaitAndPlayNextMusic(float p_EndTime)
    {
        co_yield nullptr;

        auto l_Interval = 1.0f / 16.0f;
        auto l_Waiter   = reinterpret_cast<IEnumerator*>(WaitForSeconds::New_ctor(l_Interval));

        do
        {
            /// Skip if it's not the menu
            if (CP_SDK::ChatPlexSDK::ActiveGenericScene() != CP_SDK::EGenericScene::Menu || !m_Instance->m_PreviewPlayer)
            {
                m_Instance->m_WaitAndPlayNextSongCoroutine = nullptr;
                co_return;
            }

            auto l_ChannelsController = m_Instance->m_PreviewPlayer->____audioSourceControllers;
            if (l_ChannelsController.size() > 0)
            {
                for (auto l_ChannelController : l_ChannelsController)
                {
                    auto l_Channel = l_ChannelController->___audioSource;

                    if (   !m_Instance->m_IsPaused
                        && !l_Channel->get_isPlaying()
                        && l_Channel->get_clip() == m_Instance->m_CurrentMusicAudioClip
                        && l_ChannelsController->IndexOf(l_ChannelController) == m_Instance->m_PreviewPlayer->____activeChannel)
                    {
                        l_Channel->UnPause();
                    }

                    if (l_Channel->get_isPlaying() && l_Channel->get_clip() == m_Instance->m_CurrentMusicAudioClip)
                    {
                        if (!m_Instance->m_BackupTimeClip || m_Instance->m_BackupTimeClip != m_Instance->m_CurrentMusicAudioClip)
                        {
                            m_Instance->m_BackupTimeClip = m_Instance->m_CurrentMusicAudioClip;
                            m_Instance->m_BackupTime     = l_Channel->get_time();
                        }
                        else if (std::abs(l_Channel->get_time() - m_Instance->m_BackupTime) > 1.0f)
                            l_Channel->set_time(m_Instance->m_BackupTime);
                        else
                            m_Instance->m_BackupTime = l_Channel->get_time();

                        if (m_Instance->m_IsPaused)
                            l_Channel->Pause();
                        else
                        {
                            auto  l_Volume  = MMConfig::Instance()->PlaybackVolume;
                            auto& l_Modules = CP_SDK::ChatPlexSDK::GetModules();
                            if (std::count_if(l_Modules.begin(), l_Modules.end(), [](auto x) { return x->Name() == u"Audio Tweaker"; }))
                                l_Volume = 1.0f;

                            m_Instance->m_PreviewPlayer->____ambientVolumeScale = l_Volume;
                            m_Instance->m_PreviewPlayer->____volumeScale        = l_Volume;
                        }

                        if (Mathf::Abs(p_EndTime - l_Channel->get_time()) < (MMConfig::Instance()->LoopCurrentMusic ? l_Interval : 3.0f))
                        {
                            m_Instance->m_WaitAndPlayNextSongCoroutine = nullptr;

                            if (MMConfig::Instance()->LoopCurrentMusic && l_Channel->get_clip()->get_length() >= 10.0f)
                            {
                                l_Channel->set_time(0.0f);
                                m_Instance->m_BackupTime = 0.0f;
                            }
                            else
                            {
                                m_Instance->StartNextMusic();
                                co_return;
                            }
                        }
                    }
                }
            }

            co_yield l_Waiter;

        } while (true);
    }

}   ///< namespace ChatPlexMod_MenuMusic