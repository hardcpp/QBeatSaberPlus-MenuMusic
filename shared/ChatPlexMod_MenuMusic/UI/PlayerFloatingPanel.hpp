#pragma once

#include "ChatPlexMod_MenuMusic/Data/Music.hpp"

#include <CP_SDK/Misc/FastCancellationToken.hpp>
#include <CP_SDK/UI/ViewController.hpp>
#include <CP_SDK/XUI/XUI.hpp>

namespace ChatPlexMod_MenuMusic::UI {

    namespace _u
    {
        using namespace CP_SDK::XUI;
        using namespace UnityEngine;
    }

    /// @brief Player floating panel view
    class PlayerFloatingPanel : public CP_SDK::UI::ViewController
    {
        CP_SDK_IL2CPP_INHERIT("ChatPlexMod_MenuMusic.UI", PlayerFloatingPanel, CP_SDK::UI::ViewController);
        CP_SDK_IL2CPP_DECLARE_CTOR_CHILD(PlayerFloatingPanel);
        CP_SDK_IL2CPP_DECLARE_DTOR_MONOBEHAVIOUR_CHILD(PlayerFloatingPanel);
        CP_SDK_UI_VIEW_CONTROLLER_INSTANCE();

        private:
            _u::XUIHLayout::Ptr                         m_MusicBackground;
            _u::XUIHLayout::Ptr                         m_MusicCover;
            _u::XUIText::Ptr                            m_SongTitle;
            _u::XUIText::Ptr                            m_SongArtist;
            _u::XUIIconButton::Ptr                      m_PlayPauseButton;
            _u::XUISlider::Ptr                          m_Volume;
            _u::XUIPrimaryButton::Ptr                   m_PlayItButton;

            CP_SDK::Misc::FastCancellationToken::Ptr    m_CancellationToken;
            CP_SDK::Utils::MonoPtr<_u::Sprite>          m_PlaySprite;
            CP_SDK::Utils::MonoPtr<_u::Sprite>          m_PauseSprite;

            Data::Music::Ptr                            m_CurrentMusic;

        private:
            /// @brief On view creation
            void OnViewCreation_Impl();
            /// @brief On view activation
            void OnViewActivation_Impl();

        private:
            /// @brief When settings are changed
            void OnSettingChanged();

        public:
            /// @brief Set current played music
            /// @param p_Music Current song name
            void OnMusicChanged(const Data::Music::Ptr& p_Music);
            /// @brief Is the music paused
            /// @param p_IsPaused
            void SetIsPaused(bool p_IsPaused);

        public:
            /// @brief Update volume
            void UpdateVolume();

        private:
            /// @brief Reset button
            void OnPrevPressed();
            /// @brief Reload songs
            void OnRandPressed();
            /// @brief When the playlist button is pressed
            void OnPlaylistPressed();
            /// @brief When the random button is pressed
            void OnPlayPausePressed();
            /// @brief When the next button is pressed
            void OnNextPressed();

        private:
            /// @brief On play the map pressed
            void OnPlayItPressed();

    };

}   ///< namespace ChatPlexMod_MenuMusic::UI

CP_SDK_IL2CPP_INHERIT_HELPERS(ChatPlexMod_MenuMusic::UI::PlayerFloatingPanel);