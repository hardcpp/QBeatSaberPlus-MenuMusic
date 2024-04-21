#pragma once

#include "Data/IMusicProvider.hpp"
#include "Data/Music.hpp"
#include "UI/PlayerFloatingPanel.hpp"
#include "UI/SettingsMainView.hpp"
#include "UI/SettingsLeftView.hpp"
#include "MMConfig.hpp"

#include <CP_SDK/Unity/MTCoroutineStarter.hpp>
#include <CP_SDK/ModuleBase.hpp>
#include <CP_SDK/ChatPlexSDK.hpp>

#include <GlobalNamespace/SongPreviewPlayer.hpp>
#include <System/Collections/IEnumerator.hpp>
#include <UnityEngine/Coroutine.hpp>
#include <UnityEngine/AudioClip.hpp>

#include <vector>

namespace ChatPlexMod_MenuMusic {

    using namespace GlobalNamespace;
    using namespace UnityEngine;
    using namespace System::Collections;

    /// @brief Menu Music Module
    class MenuMusic : public CP_SDK::ModuleBase<MenuMusic>
    {
        CP_SDK_MODULE_BASE_INSTANCE_DECL(MenuMusic);

        public:
            CP_SDK::EIModuleBaseType            Type()                      const override { return CP_SDK::EIModuleBaseType::Integrated;                               }
            std::u16string_view                 Name()                      const override { return u"Menu Music";                                                      }
            std::u16string_view                 FancyName()                 const override { return u"Menu Music";                                                      }
            std::u16string_view                 Description()               const override { return u"Replace boring ambient noise by music!";                          }
            std::u16string_view                 DocumentationURL()          const override { return u"https://github.com/hardcpp/BeatSaberPlus/wiki#menu-music";        }
            bool                                UseChatFeatures()           const override { return false;                                                              }
            bool                                IsEnabled()                 const override { return MMConfig::Instance()->Enabled;                                      }
            void                                IsEnabled(bool p_Enabled)         override { MMConfig::Instance()->Enabled = p_Enabled; MMConfig::Instance()->Save();   }
            CP_SDK::EIModuleBaseActivationType  ActivationType()            const override { return CP_SDK::EIModuleBaseActivationType::OnMenuSceneLoaded;              }

        private:
            CP_SDK::Utils::MonoPtr<UI::SettingsMainView>                        m_SettingsMainView;
            CP_SDK::Utils::MonoPtr<UI::SettingsLeftView>                        m_SettingsLeftView;

        private:
            CP_SDK::Utils::MonoPtr<CP_SDK::UI::Components::CFloatingPanel>      m_PlayerFloatingPanel;
            CP_SDK::Utils::MonoPtr<UI::PlayerFloatingPanel>                     m_PlayerFloatingPanelView;

            CP_SDK::Utils::MonoPtr<Coroutine>                                   m_CreateFloatingPlayerCoroutine;
            CP_SDK::Utils::MonoPtr<Coroutine>                                   m_WaitAndPlayNextSongCoroutine;

            bool                                        m_WantsToQuit;
            CP_SDK::Utils::MonoPtr<SongPreviewPlayer>   m_PreviewPlayer;
            CP_SDK::Utils::MonoPtr<AudioClip>           m_OriginalMenuMusic;
            float                                       m_OriginalAmbientVolumeScale;
            Data::Music::Ptr                            m_CurrentMusic;
            CP_SDK::Utils::MonoPtr<AudioClip>           m_CurrentMusicAudioClip;
            CP_SDK::Utils::MonoPtr<AudioClip>           m_BackupTimeClip;
            float                                       m_BackupTime;
            bool                                        m_IsPaused;

            Data::IMusicProvider::Ptr                   m_MusicProvider;
            int                                         m_CurrentSongIndex;
            CP_SDK::Utils::MonoPtr<Coroutine>           m_WaitUntillReadyCoroutine;
            CP_SDK::Misc::FastCancellationToken::Ptr    m_FastCancellationToken;

        public:
            /// @brief Constructor
            MenuMusic();

        protected:
            /// @brief Enable the Module
            void OnEnable() override;
            /// @brief Disable the Module
            void OnDisable() override;

        public:
            /// @brief Get Module settings UI
            t_InitialViews GetSettingsViewControllers() override;

        private:
            /// @brief When the active scene change
            /// @param p_Scene Scene type
            void ChatPlexSDK_OnGenericSceneChange(CP_SDK::EGenericScene p_Scene);
            /// @brief Application wants to quit
            bool Application_wantsToQuit();

        public:
            /// @brief Update the music provider
            void UpdateMusicProvider();
            /// @brief Update playback volume
            /// @param p_FromConfig From config?
            void UpdatePlaybackVolume(bool p_FromConfig);
            /// @brief Update player
            void UpdatePlayer();
            /// @brief Toggle pause status
            void TogglePause();

        private:
            /// @brief Create floating player window
            void CreateFloatingPlayer();
            /// @brief Create floating player window
            static custom_types::Helpers::Coroutine CreateFloatingPlayer_Coroutine();
            /// @brief Destroy floating player window
            void DestroyFloatingPlayer();

        public:
            /// @brief Start a previous music
            void StartPreviousMusic();
            /// @brief Start a new music
            /// @param p_Random            Pick a random song?
            /// @param p_OnSceneTransition On scene transition?
            void StartNewMusic(bool p_Random = false, bool p_OnSceneTransition = false);
            /// @brief Start a next music
            void StartNextMusic();

        private:
            /// @brief Load the next music
            /// @param p_OnSceneTransition Is on scene transition?
            void LoadNextMusic(bool p_OnSceneTransition);

        private:
            /// @brief Wait until music provider is ready
            /// @param p_Callback Callback action
            static custom_types::Helpers::Coroutine Coroutine_WaitUntilReady(CP_SDK::Utils::Action<> p_Callback);
            /// @brief Load the song into the preview player
            /// @param p_OnSceneTransition On scene transition?
            static custom_types::Helpers::Coroutine Coroutine_LoadAudioClip(bool p_OnSceneTransition, Data::Music::Ptr p_Music, CP_SDK::Utils::MonoPtr<AudioClip> p_AudioClip);
            /// @brief Wait and play next music
            /// @param p_WaitTime Time to wait
            static custom_types::Helpers::Coroutine Coroutine_WaitAndPlayNextMusic(float p_EndTime);

    };

}   ///< namespace ChatPlexMod_MenuMusic