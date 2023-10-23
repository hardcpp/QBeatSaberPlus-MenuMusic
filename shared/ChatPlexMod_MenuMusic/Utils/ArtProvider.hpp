#pragma once

#include <CP_SDK/Misc/FastCancellationToken.hpp>
#include <CP_SDK/Utils/Il2cpp.hpp>
#include <CP_SDK/Utils/MonoPtr.hpp>
#include <CP_SDK/Unity/TextureRaw.hpp>

#include <UnityEngine/Color.hpp>
#include <UnityEngine/Sprite.hpp>

#include <stdint.h>

namespace ChatPlexMod_MenuMusic::Utils {

    namespace _u
    {
        using namespace UnityEngine;
    }

    /// @brief Art provider for the player floating panel
    class ArtProvider
    {
        CP_SDK_NO_DEF_CTORS(ArtProvider);

        private:
            static CP_SDK::Unity::TextureRaw::PixelArray m_BackgroundMask;
            static CP_SDK::Unity::TextureRaw::PixelArray m_CoverMask;

        public:
            using t_BytesPtr  = CP_SDK::Utils::MonoPtr<Array<uint8_t>>;
            using t_Callback  = CP_SDK::Utils::Action<_u::Sprite*, _u::Sprite*>;

            /// @brief Prepare
            /// @param p_RawByte           Input raw bytes
            /// @param p_CancellationToken Cancellation token<
            /// @param p_Callback          Result callback
            static void Prepare(const t_BytesPtr&                               p_RawByte,
                                const CP_SDK::Misc::FastCancellationToken::Ptr& p_CancellationToken,
                                const t_Callback&                               p_Callback);

        private:
            /// @brief Prepare implementation
            /// @param p_RawByte           Input raw bytes
            /// @param p_CancellationToken Cancellation token<
            /// @param p_Callback          Result callback
            static void PrepareImpl(t_BytesPtr                               p_RawByte,
                                    CP_SDK::Misc::FastCancellationToken::Ptr p_CancellationToken,
                                    t_Callback                               p_Callback);


    };

}   ///< namespace ChatPlexMod_MenuMusic::Utils