#include "ChatPlexMod_MenuMusic/Data/Music.hpp"
#include "ChatPlexMod_MenuMusic/Logger.hpp"
#include "assets.hpp"

#include <CP_SDK/Unity/MTCoroutineStarter.hpp>
#include <CP_SDK/Unity/MTThreadInvoker.hpp>

#include <System/UriHelper.hpp>
#include <System/IO/File.hpp>
#include <UnityEngine/AudioDataLoadState.hpp>
#include <UnityEngine/AudioType.hpp>
#include <UnityEngine/WaitForSecondsRealtime.hpp>
#include <UnityEngine/Networking/DownloadHandlerAudioClip.hpp>
#include <UnityEngine/Networking/UnityWebRequest.hpp>
#include <UnityEngine/Networking/UnityWebRequestMultimedia.hpp>
#include <UnityEngine/Networking/UnityWebRequestAsyncOperation.hpp>

using namespace UnityEngine;
using namespace UnityEngine::Networking;

namespace ChatPlexMod_MenuMusic { namespace Data {

    const IMusicProvider::Ptr& Music::MusicProvider()
    {
        return m_MusicProvider;
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Constructor
    /// @param p_MusicProvider Music provider instance
    /// @param p_SongPath      Path to the music file
    /// @param p_SongCoverPath Path to the music cover file
    /// @param p_SongName      Name of the song
    /// @param p_SongArtist    Artist of the song
    /// @param p_CustomData    Custom data
    Music::Music(const IMusicProvider::Ptr& p_MusicProvider, std::u16string_view p_SongPath, std::u16string_view p_SongCoverPath, std::u16string_view p_SongName, std::u16string_view p_SongArtist, std::u16string_view p_CustomData)
    {
        auto f_Trim = [](std::u16string& p_Str) {
            p_Str.erase(p_Str.find_last_not_of(u" \n\r\t")+1);
            p_Str.erase(0, p_Str.find_first_not_of(u" \n\r\t"));
        };

        m_MusicProvider = p_MusicProvider;
        m_SongPath      = StringW(p_SongPath)->Replace('\\', '/');
        m_SongCoverPath = StringW(p_SongCoverPath)->Replace('\\', '/');
        m_SongName      = p_SongName;
        m_SongArtist    = p_SongArtist;

        f_Trim(m_SongName);
        f_Trim(m_SongArtist);

        if (!p_CustomData.empty())
            m_CustomData = p_CustomData;
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Get song path
    std::u16string_view Music::GetSongPath()
    {
        return m_SongPath;
    }
    /// @brief Get cover path
    std::u16string_view Music::GetCoverPath()
    {
        return m_SongCoverPath;
    }
    /// @brief Get song name
    std::u16string_view Music::GetSongName()
    {
        return m_SongName;
    }
    /// @brief Get song name
    std::u16string_view Music::GetSongArtist()
    {
        return m_SongArtist;
    }
    /// @brief Get custom data
    std::u16string_view Music::GetCustomData()
    {
        return m_CustomData;
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Get audio clip async
    /// @param p_Token">Cancellation token</param>
    /// @param p_OnSuccess">On success callback</param>
    /// @param p_OnError">On error callback</param>
    void Music::GetAudioAsync(  const CP_SDK::Misc::FastCancellationToken::Ptr&                     p_Token,
                                CP_SDK::Utils::CActionRef<CP_SDK::Utils::CMonoPtrRef<AudioClip>>    p_OnSuccess,
                                CP_SDK::Utils::CActionRef<>                                         p_OnError)
    {
        CP_SDK::Unity::MTCoroutineStarter::EnqueueFromThread(
            custom_types::Helpers::CoroutineHelper::New(
                Coroutine_GetAudioAsync(shared_from_this(), p_Token, p_OnSuccess, p_OnError)
            )
        );
    }
    /// @brief Get cover async
    /// @param p_Token     Cancellation token</param>
    /// @param p_OnSuccess On success callback</param>
    /// @param p_OnError   On error callback</param>
    void Music::GetCoverBytesAsync( const CP_SDK::Misc::FastCancellationToken::Ptr&                         p_Token,
                                    CP_SDK::Utils::CActionRef<CP_SDK::Utils::CMonoPtrRef<::Array<uint8_t>>> p_OnSuccess,
                                    CP_SDK::Utils::CActionRef<>                                             p_OnError)
    {
        auto l_StartSerial = p_Token->Serial();
        CP_SDK::Unity::MTThreadInvoker::EnqueueOnThread([this, p_Token, p_OnSuccess, p_OnError, l_StartSerial]() -> void
        {
            if (p_Token->IsCancelled(l_StartSerial))
                return;

            try
            {
                CP_SDK::Utils::MonoPtr<Array<uint8_t>> l_Bytes;
                if (System::IO::File::Exists(m_SongCoverPath))
                {
                    auto l_Raw = System::IO::File::ReadAllBytes(m_SongCoverPath);
                    l_Bytes = reinterpret_cast<::Array<uint8_t>*>(l_Raw.convert());
                }
                else
                {
                    auto l_Raw = Assets::DefaultCover_png.operator ArrayW<uint8_t, Array<uint8_t> *>();
                    l_Bytes = reinterpret_cast<::Array<uint8_t>*>(l_Raw.convert());
                }

                if (p_Token->IsCancelled(l_StartSerial))
                    return;

                p_OnSuccess(l_Bytes);
            }
            catch (const std::exception& p_Exception)
            {
                Logger::Instance->Error(u"[ChatPlexMod_MenuMusic.Data][Music.GetCoverAsync] Error:");
                Logger::Instance->Error(p_Exception);
                p_OnError();
            }
        });
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// Get audio clip async
    /// @param p_Token     Cancellation token
    /// @param p_OnSuccess On success callback
    /// @param p_OnError   On error callback
    custom_types::Helpers::Coroutine Music::Coroutine_GetAudioAsync(Ptr                                                             p_Self,
                                                                    CP_SDK::Misc::FastCancellationToken::Ptr                        p_Token,
                                                                    CP_SDK::Utils::Action<CP_SDK::Utils::CMonoPtrRef<AudioClip>>    p_OnSuccess,
                                                                    CP_SDK::Utils::Action<>                                         p_OnError)
    {
        auto l_StartSerial = p_Token ? p_Token->Serial() : 0;

        co_yield nullptr;

        if (p_Token && p_Token->IsCancelled(l_StartSerial))
            co_return;

        auto l_PathParts = StringW(p_Self->m_SongPath)->Split('/');
        auto l_SafePath  = l_PathParts[0];

        for (auto l_I = 1; l_I < l_PathParts->get_Length(); ++l_I)
        {
            auto l_DestPos = 0;
            auto l_Escaped = System::UriHelper::EscapeString(l_PathParts[l_I], 0, l_PathParts[l_I]->get_Length(), nullptr, l_DestPos, true, u'\uffff', u'\uffff', u'\uffff');

            l_SafePath += StringW(System::String::New_ctor(l_Escaped, 0, l_DestPos));
        }

        auto l_FinalURL = "file://" + l_SafePath->Replace("#", "%23");
        auto l_Loader   = CP_SDK::Utils::MonoPtr<UnityWebRequest>(nullptr);
        auto l_Request  = CP_SDK::Utils::MonoPtr<UnityWebRequestAsyncOperation>(nullptr);

        try
        {
            l_Loader    = UnityWebRequestMultimedia::GetAudioClip(l_FinalURL, AudioType::OGGVORBIS);
            l_Request   = l_Loader->SendWebRequest();
        }
        catch (const std::exception& l_Exception)
        {
            Logger::Instance->Error(u"[ChatPlexMod_MenuMusic.Data][Music.GetAudioAsync] Can't load audio! Exception:");
            Logger::Instance->Error(l_Exception);

            p_OnError();
            co_return;
        }

        co_yield reinterpret_cast<System::Collections::IEnumerator*>(l_Request.Ptr());

        /// Skip if it's not the menu
        if (CP_SDK::ChatPlexSDK::ActiveGenericScene() != CP_SDK::EGenericScene::Menu)
            co_return;

        if (p_Token && p_Token->IsCancelled(l_StartSerial))
            co_return;

        if (l_Loader->get_result() == UnityWebRequest::Result::ConnectionError
            || l_Loader->get_result() == UnityWebRequest::Result::ProtocolError
            || !System::String::IsNullOrEmpty(l_Loader->get_error()))
        {
            Logger::Instance->Error(u"[ChatPlexMod_MenuMusic.Data][Music.GetAudioAsync] Can't load audio! " + (!System::String::IsNullOrEmpty(l_Loader->get_error()) ? l_Loader->get_error() : u""));
            p_OnError();
            co_return;
        }

        auto l_AudioClip = CP_SDK::Utils::MonoPtr<AudioClip>(nullptr);
        try
        {
            ((DownloadHandlerAudioClip*)l_Loader->get_downloadHandler())->set_streamAudio(true);

            l_AudioClip = DownloadHandlerAudioClip::GetContent(l_Loader.Ptr());
            if (l_AudioClip)
                l_AudioClip->set_name(p_Self->m_SongName);
            else
            {
                Logger::Instance->Error(u"[ChatPlexMod_MenuMusic.Data][Music.GetAudioAsync] No audio found");
                p_OnError();
                co_return;
            }
        }
        catch (const std::exception& l_Exception)
        {
            Logger::Instance->Error(u"[ChatPlexMod_MenuMusic.Data][Music.GetAudioAsync] Can't load audio! Exception:");
            Logger::Instance->Error(l_Exception);

            p_OnError();
            co_return;
        }

        auto l_RemainingTry  = 15;
        auto l_Waiter        = WaitForSecondsRealtime::New_ctor(0.1f)->i___System__Collections__IEnumerator();

        while (    l_AudioClip->get_loadState() != AudioDataLoadState::Loaded
                && l_AudioClip->get_loadState() != AudioDataLoadState::Failed)
        {
            co_yield l_Waiter;
            l_RemainingTry--;

            if (l_RemainingTry < 0)
            {
                p_OnError();
                co_return;
            }

            if (p_Token && p_Token->IsCancelled(l_StartSerial))
                co_return;
        }

        if (CP_SDK::ChatPlexSDK::ActiveGenericScene() != CP_SDK::EGenericScene::Menu)
            co_return;

        if (p_Token && p_Token->IsCancelled(l_StartSerial))
            co_return;

        if (l_AudioClip->get_loadState() != AudioDataLoadState::Loaded)
        {
            p_OnError();
            co_return;
        }

        p_OnSuccess(l_AudioClip);
    }

}   ///< namespace Data
}   ///< namespace ChatPlexMod_MenuMusic