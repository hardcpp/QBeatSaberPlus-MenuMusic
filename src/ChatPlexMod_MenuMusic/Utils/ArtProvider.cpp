#include "ChatPlexMod_MenuMusic/Utils/ArtProvider.hpp"
#include "ChatPlexMod_MenuMusic/Logger.hpp"

#include "assets.hpp"

#include <CP_SDK/Unity/MTMainThreadInvoker.hpp>
#include <CP_SDK/Unity/MTThreadInvoker.hpp>
#include <CP_SDK/Unity/SpriteU.hpp>
#include <CP_SDK/Unity/TextureRaw.hpp>

#include <UnityEngine/Texture2D.hpp>
#include <UnityEngine/TextureFormat.hpp>
#include <UnityEngine/TextureWrapMode.hpp>
#include <UnityEngine/Vector2Int.hpp>

using namespace UnityEngine;

namespace ChatPlexMod_MenuMusic::Utils {

    CP_SDK::Unity::TextureRaw::PixelArray ArtProvider::m_BackgroundMask;
    CP_SDK::Unity::TextureRaw::PixelArray ArtProvider::m_CoverMask;

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Prepare
    /// @param p_RawByte           Input raw bytes
    /// @param p_CancellationToken Cancellation token<
    /// @param p_Callback          Result callback
    void ArtProvider::Prepare(  const t_BytesPtr&                               p_RawByte,
                                const CP_SDK::Misc::FastCancellationToken::Ptr& p_CancellationToken,
                                const t_Callback&                               p_Callback)
    {
        CP_SDK::Unity::MTThreadInvoker::EnqueueOnThread([=]() -> void {
            ArtProvider::PrepareImpl(p_RawByte, p_CancellationToken, p_Callback);
        });
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Prepare implementation
    /// @param p_RawByte           Input raw bytes
    /// @param p_CancellationToken Cancellation token<
    /// @param p_Callback          Result callback
    void ArtProvider::PrepareImpl(  t_BytesPtr                               p_RawByte,
                                    CP_SDK::Misc::FastCancellationToken::Ptr p_CancellationToken,
                                    t_Callback                               p_Callback)
    {
        try
        {
            auto l_StartSerial = p_CancellationToken ? p_CancellationToken->Serial() : 0;

            if (!m_BackgroundMask)
            {
                int l_Width, l_Height;
                CP_SDK::Unity::TextureRaw::Load(Assets::BackgroundMask_png, l_Width, l_Height, &m_BackgroundMask);
            }

            if (!m_CoverMask)
            {
                int l_Width, l_Height;
                CP_SDK::Unity::TextureRaw::Load(Assets::CoverMask_png, l_Width, l_Height, &m_CoverMask);
            }

            if (p_CancellationToken && p_CancellationToken->IsCancelled(l_StartSerial))
                return;

            int l_OGWidth, l_OGHeight;
            CP_SDK::Unity::TextureRaw::PixelArray l_OGPixels;
            if (!CP_SDK::Unity::TextureRaw::Load(p_RawByte.Ptr(), l_OGWidth, l_OGHeight, &l_OGPixels))
            {
                CP_SDK::Unity::MTMainThreadInvoker::Enqueue([=]() -> void {
                    p_Callback(nullptr, nullptr);
                });
                return;
            }

            auto l_CoverSize         = Vector2Int(18 * 4 * 10, 18 * 4 * 10);
            auto l_BackgroundSize    = Vector2Int(80 * 1 * 10, 20 * 1 * 10);
            auto l_CoverPixels       = CP_SDK::Unity::TextureRaw::ResampleAndCrop(l_OGWidth,       l_OGHeight,      l_OGPixels,    l_CoverSize.m_X,      l_CoverSize.m_Y);
            auto l_BackgroundPixels  = CP_SDK::Unity::TextureRaw::ResampleAndCrop(l_CoverSize.m_X, l_CoverSize.m_Y, l_CoverPixels, l_BackgroundSize.m_X, l_BackgroundSize.m_Y);

            if (p_CancellationToken && p_CancellationToken->IsCancelled(l_StartSerial))
                return;

            CP_SDK::Unity::TextureRaw::FastGaussianBlur(l_BackgroundSize.m_X, l_BackgroundSize.m_Y, l_BackgroundPixels, 4);

            if (p_CancellationToken && p_CancellationToken->IsCancelled(l_StartSerial))
                return;

            CP_SDK::Unity::TextureRaw::Multiply(l_CoverPixels,         m_CoverMask);
            CP_SDK::Unity::TextureRaw::Multiply(l_BackgroundPixels,    m_BackgroundMask);

            if (p_CancellationToken && p_CancellationToken->IsCancelled(l_StartSerial))
                return;

            CP_SDK::Utils::MonoPtr<::Array<Color>> l_CoverPixelsConverted;
            CP_SDK::Utils::MonoPtr<::Array<Color>> l_BackgroundPixelsConverted;

            if (CP_SDK::ChatPlexSDK::ActiveGenericScene() != CP_SDK::EGenericScene::Menu)
                return;

            try
            {
                l_CoverPixelsConverted      = ::Array<Color>::NewLength(l_CoverPixels->size());
                l_BackgroundPixelsConverted = ::Array<Color>::NewLength(l_BackgroundPixels->size());

                memcpy(&l_CoverPixelsConverted->_values[0],      &(*l_CoverPixels.get())[0],        sizeof(Color) * l_CoverPixels->size());
                memcpy(&l_BackgroundPixelsConverted->_values[0], &(*l_BackgroundPixels.get())[0],   sizeof(Color) * l_BackgroundPixels->size());
            }
            catch (const std::exception& l_Exception)
            {
                Logger::Instance->Error(u"[ChatPlexMod_MenuMusic.Utils][ArtProvider.PrepareImpl] Error:");
                Logger::Instance->Error(l_Exception);
            }

            CP_SDK::Unity::MTMainThreadInvoker::Enqueue([=]() -> void
            {
                try
                {
                    if (CP_SDK::ChatPlexSDK::ActiveGenericScene() != CP_SDK::EGenericScene::Menu)
                        return;

                    if (p_CancellationToken && p_CancellationToken->IsCancelled(l_StartSerial))
                        return;

                    auto l_CoverTexture = Texture2D::New_ctor(l_CoverSize.m_X, l_CoverSize.m_Y, TextureFormat::RGBA32, false);
                    l_CoverTexture->set_wrapMode(TextureWrapMode::Clamp);
                    l_CoverTexture->SetPixels(l_CoverPixelsConverted.Ptr());
                    l_CoverTexture->Apply(true);

                    auto l_BackgroundTexture = Texture2D::New_ctor(l_BackgroundSize.m_X, l_BackgroundSize.m_Y, TextureFormat::RGBA32, false);
                    l_BackgroundTexture->set_wrapMode(TextureWrapMode::Clamp);
                    l_BackgroundTexture->SetPixels(l_BackgroundPixelsConverted.Ptr());
                    l_BackgroundTexture->Apply(true);

                    p_Callback(
                        CP_SDK::Unity::SpriteU::CreateFromTexture(l_CoverTexture),
                        CP_SDK::Unity::SpriteU::CreateFromTexture(l_BackgroundTexture)
                    );
                }
                catch (const std::exception& l_Exception)
                {
                    Logger::Instance->Error(u"[ChatPlexMod_MenuMusic.Utils][ArtProvider.PrepareImpl] Error:");
                    Logger::Instance->Error(l_Exception);
                }
            });

            return;
        }
        catch (const std::exception& l_Exception)
        {
            Logger::Instance->Error(u"[ChatPlexMod_MenuMusic.Utils][ArtProvider.PrepareImpl] Error:");
            Logger::Instance->Error(l_Exception);
        }

        CP_SDK::Unity::MTMainThreadInvoker::Enqueue([=]() -> void {
            p_Callback(nullptr, nullptr);
        });
    }

}   ///< namespace ChatPlexMod_MenuMusic::Utils