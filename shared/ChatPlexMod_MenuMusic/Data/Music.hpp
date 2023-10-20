#pragma once

#include "IMusicProvider.hpp"

#include <custom-types/shared/coroutine.hpp>
#include <CP_SDK/Misc/FastCancellationToken.hpp>
#include <CP_SDK/Utils/Delegate.hpp>
#include <CP_SDK/Utils/MonoPtr.hpp>

#include <UnityEngine/AudioClip.hpp>

namespace ChatPlexMod_MenuMusic { namespace Data {

    namespace _u
    {
        using namespace UnityEngine;
    }

    /// @brief Generic music entry
    class Music : public std::enable_shared_from_this<Music>
    {
        DISABLE_CONSTRUCTORS(Music);

        private:
            IMusicProvider::Ptr m_MusicProvider;
            std::u16string      m_SongPath;
            std::u16string      m_SongCoverPath;
            std::u16string      m_SongName;
            std::u16string      m_SongArtist;
            std::string         m_CustomData;

        public:
            const IMusicProvider::Ptr& MusicProvider();

        public:
            using Ptr = std::shared_ptr<Music>;

            /// @brief Constructor
            /// @param p_MusicProvider Music provider instance
            /// @param p_SongPath      Path to the music file
            /// @param p_SongCoverPath Path to the music cover file
            /// @param p_SongName      Name of the song
            /// @param p_SongArtist    Artist of the song
            /// @param p_CustomData    Custom data
            Music(const IMusicProvider::Ptr& p_MusicProvider, std::u16string_view p_SongPath, std::u16string_view p_SongCoverPath, std::u16string_view p_SongName, std::u16string_view p_SongArtist, std::string_view p_CustomData);

        public:
            /// @brief Get song path
            std::u16string_view GetSongPath();
            /// @brief Get cover path
            std::u16string_view GetCoverPath();
            /// @brief Get song name
            std::u16string_view GetSongName();
            /// @brief Get song name
            std::u16string_view GetSongArtist();
            /// @brief Get custom data
            std::string_view GetCustomData();

        public:
            /// @brief Get audio clip async
            /// @param p_Token">Cancellation token</param>
            /// @param p_OnSuccess">On success callback</param>
            /// @param p_OnError">On error callback</param>
            void GetAudioAsync( const CP_SDK::Misc::FastCancellationToken::Ptr&                         p_Token,
                                CP_SDK::Utils::CActionRef<CP_SDK::Utils::CMonoPtrRef<_u::AudioClip>>    p_OnSuccess,
                                CP_SDK::Utils::CActionRef<>                                             p_OnError);
            /// @brief Get cover async
            /// @param p_Token     Cancellation token</param>
            /// @param p_OnSuccess On success callback</param>
            /// @param p_OnError   On error callback</param>
            void GetCoverBytesAsync(const CP_SDK::Misc::FastCancellationToken::Ptr&                         p_Token,
                                    CP_SDK::Utils::CActionRef<CP_SDK::Utils::CMonoPtrRef<::Array<uint8_t>>> p_OnSuccess,
                                    CP_SDK::Utils::CActionRef<>                                             p_OnError);

        private:
            /// Get audio clip async
            /// @param p_Token     Cancellation token
            /// @param p_OnSuccess On success callback
            /// @param p_OnError   On error callback
            static custom_types::Helpers::Coroutine Coroutine_GetAudioAsync(Ptr                                                                 p_Self,
                                                                            CP_SDK::Misc::FastCancellationToken::Ptr                            p_Token,
                                                                            CP_SDK::Utils::Action<CP_SDK::Utils::CMonoPtrRef<_u::AudioClip>>    p_OnSuccess,
                                                                            CP_SDK::Utils::Action<>                                             p_OnError);

    };

}   ///< namespace Data
}   ///< namespace ChatPlexMod_MenuMusic