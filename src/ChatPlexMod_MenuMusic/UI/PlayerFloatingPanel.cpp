#include "ChatPlexMod_MenuMusic/UI/PlayerFloatingPanel.hpp"
#include "ChatPlexMod_MenuMusic/Utils/ArtProvider.hpp"
#include "ChatPlexMod_MenuMusic/MenuMusic.hpp"
#include "ChatPlexMod_MenuMusic/MMConfig.hpp"
#include "ChatPlexMod_MenuMusic/Logger.hpp"

#include "assets.hpp"

#include <CP_SDK/UI/ValueFormatters.hpp>
#include <CP_SDK/Unity/Extensions/ColorU.hpp>
#include <CP_SDK/Unity/Operators.hpp>
#include <CP_SDK/Unity/SpriteU.hpp>

using namespace CP_SDK::Unity::Extensions;
using namespace CP_SDK::XUI;
using namespace UnityEngine;

namespace ChatPlexMod_MenuMusic::UI {

    CP_SDK_IL2CPP_INHERIT_INIT(PlayerFloatingPanel);

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Constructor
    CP_SDK_IL2CPP_DECLARE_CTOR_IMPL(PlayerFloatingPanel)
    {
        m_CancellationToken = CP_SDK::Misc::FastCancellationToken::Make();

        OnViewCreation      = {this, &PlayerFloatingPanel::OnViewCreation_Impl};
        OnViewActivation    = {this, &PlayerFloatingPanel::OnViewActivation_Impl};
    }
    /// @brief Destructor
    CP_SDK_IL2CPP_DECLARE_DTOR_MONOBEHAVIOUR_IMPL(PlayerFloatingPanel)
    {

    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief On view creation
    void PlayerFloatingPanel::OnViewCreation_Impl()
    {
        auto l_NextSprite        = CP_SDK::Unity::SpriteU::CreateFromRaw(Assets::Next_png);
        auto l_GlassSprite       = CP_SDK::Unity::SpriteU::CreateFromRaw(Assets::Glass_png);
        auto l_PauseSprite       = CP_SDK::Unity::SpriteU::CreateFromRaw(Assets::Pause_png);
        auto l_PlaySprite        = CP_SDK::Unity::SpriteU::CreateFromRaw(Assets::Play_png);
        auto l_PlaylistSprite    = CP_SDK::Unity::SpriteU::CreateFromRaw(Assets::Playlist_png);
        auto l_PrevSprite        = CP_SDK::Unity::SpriteU::CreateFromRaw(Assets::Prev_png);
        auto l_RandSprite        = CP_SDK::Unity::SpriteU::CreateFromRaw(Assets::Rand_png);
        auto l_SoundSprite       = CP_SDK::Unity::SpriteU::CreateFromRaw(Assets::Sound_png);

        m_PlaySprite    = l_PlaySprite;
        m_PauseSprite   = l_PauseSprite;

        Templates::FullRectLayout({
            XUIHLayout::Make()
                ->SetPadding(0)->SetSpacing(0)
                ->SetBackground(true, ColorU::WithAlpha(Color::get_gray(), 0.75f), true)
                ->OnReady([](CP_SDK::UI::Components::CHLayout* x) -> void {
                    x->CSizeFitter()->set_enabled  (false);
                    x->LElement()->set_ignoreLayout(true);
                    x->RTransform()->set_anchorMin (Vector2::get_zero());
                    x->RTransform()->set_anchorMax (Vector2::get_one());
                    x->RTransform()->set_sizeDelta (Vector2::get_zero());
                })
                ->Bind(&m_MusicBackground)
                ->AsShared(),

            XUIHLayout::Make({
                XUIHLayout::Make({
                    XUIHLayout::Make()
                        ->SetPadding(0)->SetSpacing(0)
                        ->SetBackground(true, ColorU::WithAlpha(Color::get_white(), 0.8f))->SetBackgroundSprite(l_GlassSprite)
                        ->OnReady([](CP_SDK::UI::Components::CHLayout* x) -> void {
                            x->CSizeFitter()->set_enabled  (false);
                            x->LElement()->set_ignoreLayout(true);
                            x->RTransform()->set_anchorMin (Vector2::get_zero());
                            x->RTransform()->set_anchorMax (Vector2::get_one());
                            x->RTransform()->set_sizeDelta (Vector2::get_zero());
                        })
                        ->AsShared()
                })
                ->SetBackground(true, Color::get_white())
                ->SetWidth(18.0f)->SetHeight(18.0f)
                ->Bind(&m_MusicCover)
                ->AsShared(),

                XUIVLayout::Make({
                    XUIText::Make(u"Song name...")    ->SetFontSize(3.5f)->SetOverflowMode(TMPro::TextOverflowModes::Ellipsis)->Bind(&m_SongTitle)->AsShared(),
                    XUIText::Make(u"Song artist...")  ->SetFontSize(3.0f)->SetOverflowMode(TMPro::TextOverflowModes::Ellipsis)->SetColor(ColorU::ToUnityColor("#A0A0A0"))->Bind(&m_SongArtist)->AsShared(),
                    XUIText::Make(u" "),
                    XUIHLayout::Make({
                        XUIIconButton::Make({this, &PlayerFloatingPanel::OnPrevPressed})       ->SetSprite(l_PrevSprite)->AsShared(),
                        XUIIconButton::Make({this, &PlayerFloatingPanel::OnRandPressed})       ->SetSprite(l_RandSprite)->AsShared(),
                        //XUIIconButton::Make({this, &PlayerFloatingPanel::OnPlaylistPressed}) ->SetSprite(l_PlaylistSprite),
                        XUIIconButton::Make({this, &PlayerFloatingPanel::OnPlayPausePressed})  ->SetSprite(l_PlaySprite)->Bind(&m_PlayPauseButton)->AsShared(),
                        XUIIconButton::Make({this, &PlayerFloatingPanel::OnNextPressed})       ->SetSprite(l_NextSprite)->AsShared()
                    })
                    ->SetPadding(0)
                    ->ForEachDirect<XUIIconButton>([](XUIIconButton* x) -> void {
                        x->OnReady([](CP_SDK::UI::Components::CIconButton* y) -> void { y->RTransform()->set_localScale(1.5f * Vector3::get_one()); } );
                    })
                    ->AsShared()
                })
                ->SetPadding(0, 0, 0, 1)->SetSpacing(0)
                ->OnReady([](CP_SDK::UI::Components::CVLayout* x) -> void {
                    x->CSizeFitter()->set_enabled        (false);
                    x->LElement()->set_flexibleWidth     (1000.0f);
                    x->VLayoutGroup()->set_childAlignment(TextAnchor::UpperLeft);
                })
                ->AsShared()
            })
            ->SetPadding(1)->SetSpacing(0)
            ->OnReady([](CP_SDK::UI::Components::CHLayout* x) -> void {
                x->HLayoutGroup()->set_childForceExpandWidth (true);
                x->HLayoutGroup()->set_childForceExpandHeight(true);
                x->CSizeFitter()->set_enabled                (false);
                x->LElement()->set_ignoreLayout              (true);
                x->RTransform()->set_anchorMin               (Vector2::get_zero());
                x->RTransform()->set_anchorMax               (Vector2::get_one());
            })
            ->AsShared()
        })
        ->SetPadding(0)->SetSpacing(0)
        ->OnReady([](CP_SDK::UI::Components::CVLayout* x) -> void {
            x->VLayoutGroup()->set_childForceExpandWidth (true);
            x->VLayoutGroup()->set_childForceExpandHeight(true);
        })
        ->BuildUI(get_transform());

        XUISlider::Make(u"Volume")
            ->SetMinValue(0.0f)->SetMaxValue(1.0f)
            ->SetValue(MMConfig::Instance()->PlaybackVolume)
            ->OnValueChanged([this](float) -> void { OnSettingChanged(); })
            ->SetFormatter(&CP_SDK::UI::ValueFormatters::Percentage)
            ->OnReady([](CP_SDK::UI::Components::CSlider* x) -> void
            {
                x->LElement()->set_enabled           (false);
                x->RTransform()->set_pivot           (Vector2(  1.00f, 0.00f));
                x->RTransform()->set_anchorMin       (Vector2(  1.00f, 0.00f));
                x->RTransform()->set_anchorMax       (Vector2(  1.00f, 0.00f));
                x->RTransform()->set_anchoredPosition(Vector2(-11.00f, 1.15f));
                x->RTransform()->set_sizeDelta       (Vector2( 35.00f, 5.00f));
                x->RTransform()->set_localScale      (0.7f * Vector3::get_one());
            })
            ->Bind(&m_Volume)
            ->BuildUI(get_transform());

        XUIPrimaryButton::Make(u"Play it", {this, &PlayerFloatingPanel::OnPlayItPressed})
            ->OnReady([](CP_SDK::UI::Components::CPrimaryButton* x) -> void
            {
                x->LElement()->set_enabled           (false);
                x->RTransform()->set_pivot           (Vector2( 1.00f, 0.00f));
                x->RTransform()->set_anchorMin       (Vector2( 1.00f, 0.00f));
                x->RTransform()->set_anchorMax       (Vector2( 1.00f, 0.00f));
                x->RTransform()->set_anchoredPosition(Vector2(-2.00f, 1.15f));
                x->RTransform()->set_localScale      (0.7f * Vector3::get_one());
            })
            ->Bind(&m_PlayItButton)
            ->BuildUI(get_transform());

        OnMusicChanged(nullptr);

        auto& l_Modules = CP_SDK::ChatPlexSDK::GetModules();
        if (std::count_if(l_Modules.begin(), l_Modules.end(), [](auto x) { return x->Name() == u"Audio Tweaker"; }))
            m_Volume->SetActive(false);
    }
    /// @brief On view activation
    void PlayerFloatingPanel::OnViewActivation_Impl()
    {
        UpdateVolume();
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief When settings are changed
    void PlayerFloatingPanel::OnSettingChanged()
    {
        MMConfig::Instance()->PlaybackVolume = m_Volume->Element()->GetValue();
        MenuMusic::Instance()->UpdatePlaybackVolume(false);
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Set current played music
    /// @param p_Music Current song name
    void PlayerFloatingPanel::OnMusicChanged(const Data::Music::Ptr& p_Music)
    {
        auto l_Str1 = p_Music ? std::u16string(p_Music->GetSongName()) : u"<alpha=#AA>No name...";
        if (l_Str1.size() > 30)
            l_Str1 = l_Str1.substr(0, 30) + u"...";

        auto l_Str2 = p_Music ? std::u16string(p_Music->GetSongArtist()) : u" ";
        if (l_Str2.size() > 50)
            l_Str2  = l_Str2.substr(0, 50) + u"...";

        if (m_SongTitle  && m_SongTitle->Element()) m_SongTitle->Element()->TMProUGUI()->set_text(l_Str1);
        if (m_SongArtist && m_SongTitle->Element()) m_SongArtist->Element()->TMProUGUI()->set_text(l_Str2);

        m_CancellationToken->Cancel();
        if (p_Music)
        {
            p_Music->GetCoverBytesAsync(m_CancellationToken, [this](const CP_SDK::Utils::MonoPtr<Array<uint8_t>>& x) -> void
            {
                Utils::ArtProvider::Prepare(x, m_CancellationToken, [this](Sprite* p_Cover, Sprite* p_Background) -> void
                {
                    m_MusicCover->SetBackgroundSprite(p_Cover);
                    m_MusicBackground->SetBackgroundSprite(p_Background);
                });
            }, nullptr);

            if (m_PlayItButton)
                m_PlayItButton->SetInteractable(p_Music->MusicProvider()->SupportPlayIt());
        }

        m_CurrentMusic = p_Music;
    }
    /// @brief Is the music paused
    /// @param p_IsPaused
    void PlayerFloatingPanel::SetIsPaused(bool p_IsPaused)
    {
        if (m_PlayPauseButton)
            m_PlayPauseButton->SetSprite(p_IsPaused ? m_PlaySprite.Ptr() : m_PauseSprite.Ptr());
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Update volume
    void PlayerFloatingPanel::UpdateVolume()
    {
        if (m_Volume)
            m_Volume->SetValue(MMConfig::Instance()->PlaybackVolume, false);
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Reset button
    void PlayerFloatingPanel::OnPrevPressed()
    {
        MenuMusic::Instance()->StartPreviousMusic();
    }
    /// @brief Reload songs
    void PlayerFloatingPanel::OnRandPressed()
    {
        MenuMusic::Instance()->StartNewMusic(true);
    }
    /// @brief When the playlist button is pressed
    void PlayerFloatingPanel::OnPlaylistPressed()
    {

    }
    /// @brief When the random button is pressed
    void PlayerFloatingPanel::OnPlayPausePressed()
    {
        MenuMusic::Instance()->TogglePause();
    }
    /// @brief When the next button is pressed
    void PlayerFloatingPanel::OnNextPressed()
    {
        MenuMusic::Instance()->StartNextMusic();
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief On play the map pressed
    void PlayerFloatingPanel::OnPlayItPressed()
    {
        if (m_CurrentMusic == nullptr || !m_CurrentMusic->MusicProvider()->SupportPlayIt())
            return;

        if (!m_CurrentMusic->MusicProvider()->StartGameSpecificGamePlay(m_CurrentMusic))
            ShowMessageModal(u"Map not found!");
    }

}   ///< namespace ChatPlexMod_MenuMusic::UI