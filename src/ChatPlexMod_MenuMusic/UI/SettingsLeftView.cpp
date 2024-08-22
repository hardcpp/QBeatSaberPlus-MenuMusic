#include "ChatPlexMod_MenuMusic/UI/SettingsLeftView.hpp"
#include "ChatPlexMod_MenuMusic/UI/SettingsMainView.hpp"
#include "ChatPlexMod_MenuMusic/MenuMusic.hpp"

using namespace CP_SDK::XUI;
using namespace UnityEngine;

namespace ChatPlexMod_MenuMusic::UI {

    constexpr const std::u16string_view s_InformationStr =
               u"<b>Thanks to <b>Lunikc</b> for original idea</b>"
         u"\n" u"Custom music folder is: QBeatSaberPlus/MenuMusix/CustomMusic";

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    CP_SDK_IL2CPP_INHERIT_INIT(SettingsLeftView);

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Constructor
    CP_SDK_IL2CPP_DECLARE_CTOR_IMPL(SettingsLeftView)
    {
        OnViewCreation = {this, &SettingsLeftView::OnViewCreation_Impl};
    }
    /// @brief Destructor
    CP_SDK_IL2CPP_DECLARE_DTOR_MONOBEHAVIOUR_IMPL(SettingsLeftView)
    {

    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief On view creation
    void SettingsLeftView::OnViewCreation_Impl()
    {
        Templates::FullRectLayout({
            Templates::TitleBar(u"Information / Credits"),

            Templates::ScrollableInfos(50.0f, {
                XUIText::Make(s_InformationStr)
                    ->SetAlign(TMPro::TextAlignmentOptions::Left)
                    ->AsShared()
            }),

            Templates::ExpandedButtonsLine({
                XUIPrimaryButton::Make(u"Reset",  {this, &SettingsLeftView::OnResetButton})->AsShared(),
                XUIPrimaryButton::Make(u"Reload", {this, &SettingsLeftView::OnReloadButton})->AsShared()
            }),
            Templates::ExpandedButtonsLine({
                XUISecondaryButton::Make(u"Documentation", {this, &SettingsLeftView::OnDocumentationButton})->AsShared()
            })
        })
        ->SetBackground(true, std::nullopt, true)
        ->BuildUI(get_transform());
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Reset button
    void SettingsLeftView::OnResetButton()
    {
        ShowConfirmationModal(
            u"<color=yellow><b>Do you really want to reset\nall Menu Music settings?",
            [this](bool p_Confirm) -> void
            {
                if (!p_Confirm)
                    return;

                MMConfig::Instance()->Reset();
                MMConfig::Instance()->Enabled = true;
                MMConfig::Instance()->Save();

                SettingsMainView::Instance()->OnResetButton();

                OnReloadButton();
            }
        );
    }
    /// @brief Reload songs
    void SettingsLeftView::OnReloadButton()
    {
        MenuMusic::Instance()->UpdateMusicProvider();

        ShowMessageModal(u"Musics were reload!");
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Documentation button
    void SettingsLeftView::OnDocumentationButton()
    {
        ShowMessageModal(u"URL opened in your web browser.");
        CP_SDK::ChatPlexSDK::OpenURL(MenuMusic::Instance()->DocumentationURL());
    }

}   ///< namespace ChatPlexMod_MenuMusic::UI