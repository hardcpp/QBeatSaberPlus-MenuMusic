#pragma once

#include <CP_SDK/UI/ViewController.hpp>
#include <CP_SDK/XUI/XUI.hpp>

namespace ChatPlexMod_MenuMusic::UI {

    /// @brief Settings left view controller
    class SettingsLeftView : public CP_SDK::UI::ViewController
    {
        CP_SDK_IL2CPP_INHERIT("ChatPlexMod_MenuMusic.UI", SettingsLeftView, CP_SDK::UI::ViewController);
        CP_SDK_IL2CPP_DECLARE_CTOR_CHILD(SettingsLeftView);
        CP_SDK_IL2CPP_DECLARE_DTOR_MONOBEHAVIOUR_CHILD(SettingsLeftView);
        CP_SDK_UI_VIEW_CONTROLLER_INSTANCE();

        private:
            /// @brief On view creation
            void OnViewCreation_Impl();

        private:
            /// @brief Reset button
            void OnResetButton();
            /// @brief Reload songs
            void OnReloadButton();

        private:
            /// @brief Documentation button
            void OnDocumentationButton();

    };

}   ///< namespace ChatPlexMod_MenuMusic::UI

CP_SDK_IL2CPP_INHERIT_HELPERS(ChatPlexMod_MenuMusic::UI::SettingsLeftView);