#include "ChatPlexMod_MenuMusic/UI/SettingsMainView.hpp"
#include "ChatPlexMod_MenuMusic/MenuMusic.hpp"
#include "ChatPlexMod_MenuMusic/MMConfig.hpp"
#include "ChatPlexMod_MenuMusic/Logger.hpp"

#include <CP_SDK/UI/ValueFormatters.hpp>

using namespace CP_SDK::XUI;
using namespace UnityEngine;

namespace ChatPlexMod_MenuMusic::UI {

    CP_SDK_IL2CPP_INHERIT_INIT(SettingsMainView);

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Constructor
    CP_SDK_IL2CPP_DECLARE_CTOR_CHAIN_IMPL(SettingsMainView, CP_SDK::UI::ViewController)
    {
        m_PreventChanges = false;

        OnViewCreation      = {this, &SettingsMainView::OnViewCreation_Impl};
        OnViewActivation    = {this, &SettingsMainView::OnViewActivation_Impl};
        OnViewDeactivation  = {this, &SettingsMainView::OnViewDeactivation_Impl};
    }
    /// @brief Destructor
    CP_SDK_IL2CPP_DECLARE_DTOR_MONOBEHAVIOUR_CHAIN_IMPL(SettingsMainView, CP_SDK::UI::ViewController)
    {

    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief On view creation
    void SettingsMainView::OnViewCreation_Impl()
    {
        auto l_Config = MMConfig::Instance();

        Templates::FullRectLayout({
            Templates::TitleBar(u"Menu Music | Settings"),

            XUIHLayout::Make({
                XUIVLayout::Make({
                    XUIText::Make(u"Music provider")->AsShared(),
                    XUIDropdown::Make()
                        ->SetOptions(std::vector<std::u16string>(Data::MusicProviderType::S.begin(), Data::MusicProviderType::S.end()))
                        ->SetValue(Data::MusicProviderType::ToStr(l_Config->MusicProvider))
                        ->OnValueChanged([this](int, std::u16string_view) -> void { OnSettingChanged(); })
                        ->Bind(&m_MusicProvider)
                        ->AsShared(),

                    XUIText::Make(u"Show player interface")->AsShared(),
                    XUIToggle::Make()
                        ->SetValue(l_Config->ShowPlayer)
                        ->Bind(&m_ShowPlayerInterface)
                        ->AsShared(),

                    XUIText::Make(u"Playback volume")->Bind(&m_PlaybackVolumeLabel)->AsShared(),
                    XUISlider::Make()
                        ->SetMinValue(0.0f)->SetMaxValue(1.0f)->SetIncrements(0.01f)
                        ->SetFormatter(&CP_SDK::UI::ValueFormatters::Percentage)
                        ->SetValue(l_Config->PlaybackVolume)->Bind(&m_PlaybackVolume)
                        ->AsShared()
                })
                ->SetSpacing(1)
                ->SetWidth(60.0f)
                ->OnReady([](CP_SDK::UI::Components::CVLayout* x) -> void { x->HOrVLayoutGroup()->set_childAlignment(TextAnchor::UpperCenter); })
                ->ForEachDirect<XUIText>  ([this](XUIText*   x) -> void { x->SetAlign(TMPro::TextAlignmentOptions::Center);                    })
                ->ForEachDirect<XUIToggle>([this](XUIToggle* x) -> void { x->OnValueChanged([this](bool)  -> void { OnSettingChanged(); });    })
                ->ForEachDirect<XUISlider>([this](XUISlider* x) -> void { x->OnValueChanged([this](float) -> void { OnSettingChanged(); });    })
                ->AsShared(),

                XUIVLayout::Make({
                    XUIText::Make(u"Play songs from beginning")->AsShared(),
                    XUIToggle::Make()->SetValue(l_Config->StartSongFromBeginning)->Bind(&m_PlaySongsFromBeginning)->AsShared(),

                    XUIText::Make(u"Start a new song on scene change")->AsShared(),
                    XUIToggle::Make()->SetValue(l_Config->StartANewMusicOnSceneChange)->Bind(&m_StartANewMusicOnSceneChange)->AsShared(),

                    XUIText::Make(u"Loop current song")->AsShared(),
                    XUIToggle::Make()->SetValue(l_Config->LoopCurrentMusic)->Bind(&m_LoopCurrentMusic)->AsShared()
                })
                ->SetSpacing(1)
                ->SetWidth(60.0f)
                ->OnReady([](CP_SDK::UI::Components::CVLayout* x) -> void { x->HOrVLayoutGroup()->set_childAlignment(TextAnchor::UpperCenter); })
                ->ForEachDirect<XUIText>  ([this](XUIText*   x) -> void { x->SetAlign(TMPro::TextAlignmentOptions::Center);                 })
                ->ForEachDirect<XUIToggle>([this](XUIToggle* x) -> void { x->OnValueChanged([this](bool)  -> void { OnSettingChanged(); }); })
                ->AsShared()
            })
        })
        ->SetBackground(true, std::nullopt, true)
        ->BuildUI(get_transform());

        auto& l_Modules = CP_SDK::ChatPlexSDK::GetModules();
        if (std::count_if(l_Modules.begin(), l_Modules.end(), [](auto x) { return x->Name() == u"Audio Tweaker"; }))
        {
            m_PlaybackVolumeLabel->SetActive(false);
            m_PlaybackVolume->SetActive(false);
        }
    }
    /// @brief On view activation
    void SettingsMainView::OnViewActivation_Impl()
    {
        m_PreventChanges = true;
        m_PlaybackVolume->SetValue(MMConfig::Instance()->PlaybackVolume);
        m_PreventChanges = false;
    }
    /// @brief On view deactivation
    void SettingsMainView::OnViewDeactivation_Impl()
    {
        MMConfig::Instance()->Save();
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief When settings are changed
    void SettingsMainView::OnSettingChanged()
    {
        if (m_PreventChanges)
            return;

        auto l_Config           = MMConfig::Instance();
        auto l_OldMusicProvider = l_Config->MusicProvider;
        l_Config->MusicProvider                 = Data::MusicProviderType::ToEnum(m_MusicProvider->Element()->GetValue());
        l_Config->ShowPlayer                    = m_ShowPlayerInterface->Element()->GetValue();
        l_Config->PlaybackVolume                = m_PlaybackVolume->Element()->GetValue();

        l_Config->StartSongFromBeginning        = m_PlaySongsFromBeginning->Element()->GetValue();
        l_Config->StartANewMusicOnSceneChange   = m_StartANewMusicOnSceneChange->Element()->GetValue();
        l_Config->LoopCurrentMusic              = m_LoopCurrentMusic->Element()->GetValue();

        /// Update playback volume & player
        MenuMusic::Instance()->UpdatePlaybackVolume(true);
        MenuMusic::Instance()->UpdatePlayer();

        if (l_OldMusicProvider != l_Config->MusicProvider)
            MenuMusic::Instance()->UpdateMusicProvider();
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Update volume
    void SettingsMainView::UpdateVolume()
    {
        if (m_PlaybackVolume)
            m_PlaybackVolume->SetValue(MMConfig::Instance()->PlaybackVolume, false);
    }
    /// @brief Reset settings
    void SettingsMainView::OnResetButton()
    {
        m_PreventChanges = true;

        auto l_Config = MMConfig::Instance();
        m_MusicProvider                 ->SetValue(Data::MusicProviderType::ToStr(l_Config->MusicProvider));
        m_ShowPlayerInterface           ->SetValue(l_Config->ShowPlayer);
        m_PlaybackVolume                ->SetValue(l_Config->PlaybackVolume);

        m_PlaySongsFromBeginning        ->SetValue(l_Config->StartSongFromBeginning);
        m_StartANewMusicOnSceneChange   ->SetValue(l_Config->StartANewMusicOnSceneChange);
        m_LoopCurrentMusic              ->SetValue(l_Config->LoopCurrentMusic);

        m_PreventChanges = false;

        /// Update playback volume & player
        MenuMusic::Instance()->UpdatePlaybackVolume(true);
        MenuMusic::Instance()->UpdatePlayer();
        MenuMusic::Instance()->UpdateMusicProvider();
    }

}   ///< namespace ChatPlexMod_MenuMusic::UI