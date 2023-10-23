#pragma once

#include <CP_SDK/UI/ViewController.hpp>
#include <CP_SDK/XUI/XUI.hpp>

namespace ChatPlexMod_MenuMusic::UI {

    namespace _u
    {
        using namespace CP_SDK::XUI;
        using namespace UnityEngine;
    }

    /// @brief Settings main view controller
    class SettingsMainView : public CP_SDK::UI::ViewController
    {
        CP_SDK_IL2CPP_INHERIT("ChatPlexMod_MenuMusic.UI", SettingsMainView, CP_SDK::UI::ViewController);
        CP_SDK_IL2CPP_DECLARE_CTOR_CHILD(SettingsMainView);
        CP_SDK_IL2CPP_DECLARE_DTOR_MONOBEHAVIOUR_CHILD(SettingsMainView);
        CP_SDK_UI_VIEW_CONTROLLER_INSTANCE();

        private:
            _u::XUIDropdown::Ptr    m_MusicProvider;
            _u::XUIToggle::Ptr      m_ShowPlayerInterface;
            _u::XUIText::Ptr        m_PlaybackVolumeLabel;
            _u::XUISlider::Ptr      m_PlaybackVolume;

            _u::XUIToggle::Ptr      m_PlaySongsFromBeginning;
            _u::XUIToggle::Ptr      m_StartANewMusicOnSceneChange;
            _u::XUIToggle::Ptr      m_LoopCurrentMusic;

        private:
            bool m_PreventChanges;

        private:
            /// @brief On view creation
            void OnViewCreation_Impl();
            /// @brief On view activation
            void OnViewActivation_Impl();
            /// @brief On view deactivation
            void OnViewDeactivation_Impl();

        private:
            /// @brief When settings are changed
            void OnSettingChanged();

        public:
            /// @brief Update volume
            void UpdateVolume();
            /// @brief Reset settings
            void OnResetButton();

    };

}   ///< namespace ChatPlexMod_MenuMusic::UI

CP_SDK_IL2CPP_INHERIT_HELPERS(ChatPlexMod_MenuMusic::UI::SettingsMainView);