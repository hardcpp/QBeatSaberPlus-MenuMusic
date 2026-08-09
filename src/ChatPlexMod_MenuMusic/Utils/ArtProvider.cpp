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
#include <UnityEngine/Object.hpp>
#include <UnityEngine/Vector2Int.hpp>

#include <stdexcept>

using namespace UnityEngine;

namespace ChatPlexMod_MenuMusic::Utils {

    CP_SDK::Unity::TextureRaw::PixelArray ArtProvider::m_BackgroundMask;
    CP_SDK::Unity::TextureRaw::PixelArray ArtProvider::m_CoverMask;
    std::once_flag                        ArtProvider::m_MasksInitFlag;
    bool                                  ArtProvider::m_MasksReady = false;

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
        std::call_once(m_MasksInitFlag, []() -> void
        {
            int l_BackgroundWidth  = 0;
            int l_BackgroundHeight = 0;
            int l_CoverWidth       = 0;
            int l_CoverHeight      = 0;

            CP_SDK::Unity::TextureRaw::Load(
                Assets::BackgroundMask_png, l_BackgroundWidth, l_BackgroundHeight, &m_BackgroundMask
            );
            CP_SDK::Unity::TextureRaw::Load(
                Assets::CoverMask_png, l_CoverWidth, l_CoverHeight, &m_CoverMask
            );

            m_MasksReady = true;
        });

        if (!m_MasksReady)
            throw std::runtime_error("Menu Music artwork masks could not be loaded");

        auto l_StartSerial = p_CancellationToken ? p_CancellationToken->Serial() : 0;
        try
        {
            if (!p_RawByte || (p_CancellationToken && p_CancellationToken->IsCancelled(l_StartSerial)))
                return;


            int l_OGWidth  = 0;
            int l_OGHeight = 0;
            CP_SDK::Unity::TextureRaw::PixelArray l_OGPixels;
            if (!CP_SDK::Unity::TextureRaw::Load(p_RawByte.Ptr(), l_OGWidth, l_OGHeight, &l_OGPixels))
            {
                CP_SDK::Unity::MTMainThreadInvoker::Enqueue([=]() -> void {
                    if (p_CancellationToken && p_CancellationToken->IsCancelled(l_StartSerial))
                        return;

                    p_Callback(nullptr, nullptr);
                });
                return;
            }

            if (l_OGWidth <= 0 || l_OGHeight <= 0 || !l_OGPixels || l_OGPixels->empty())
                throw std::runtime_error("Decoded artwork contains no pixels");

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

            l_CoverPixelsConverted      = ::Array<Color>::NewLength(l_CoverPixels->size());
            l_BackgroundPixelsConverted = ::Array<Color>::NewLength(l_BackgroundPixels->size());

            memcpy(l_CoverPixelsConverted->_values,      l_CoverPixels->data(),      sizeof(Color) * l_CoverPixels->size());
            memcpy(l_BackgroundPixelsConverted->_values, l_BackgroundPixels->data(), sizeof(Color) * l_BackgroundPixels->size());

            CP_SDK::Unity::MTMainThreadInvoker::Enqueue([=]() -> void
            {
                auto l_CoverTexture      = CP_SDK::Utils::MonoPtr<Texture2D>(nullptr);
                auto l_BackgroundTexture = CP_SDK::Utils::MonoPtr<Texture2D>(nullptr);
                auto l_CoverSprite       = CP_SDK::Utils::MonoPtr<Sprite>(nullptr);
                auto l_BackgroundSprite  = CP_SDK::Utils::MonoPtr<Sprite>(nullptr);

                try
                {
                    if (CP_SDK::ChatPlexSDK::ActiveGenericScene() != CP_SDK::EGenericScene::Menu)
                        return;

                    if (p_CancellationToken && p_CancellationToken->IsCancelled(l_StartSerial))
                        return;

                    l_CoverTexture = Texture2D::New_ctor(l_CoverSize.m_X, l_CoverSize.m_Y, TextureFormat::RGBA32, false);
                    l_CoverTexture->set_wrapMode(TextureWrapMode::Clamp);
                    l_CoverTexture->SetPixels(l_CoverPixelsConverted.Ptr());
                    l_CoverTexture->Apply(false, true);

                    l_BackgroundTexture = Texture2D::New_ctor(l_BackgroundSize.m_X, l_BackgroundSize.m_Y, TextureFormat::RGBA32, false);
                    l_BackgroundTexture->set_wrapMode(TextureWrapMode::Clamp);
                    l_BackgroundTexture->SetPixels(l_BackgroundPixelsConverted.Ptr());
                    l_BackgroundTexture->Apply(false, true);

                    l_CoverSprite      = CP_SDK::Unity::SpriteU::CreateFromTexture(l_CoverTexture.Ptr());
                    l_BackgroundSprite = CP_SDK::Unity::SpriteU::CreateFromTexture(l_BackgroundTexture.Ptr());

                    if (!l_CoverSprite || !l_BackgroundSprite)
                        throw std::runtime_error("Unable to create artwork sprites");

                    p_Callback(l_CoverSprite.Ptr(), l_BackgroundSprite.Ptr());
                }
                catch (const std::exception& l_Exception)
                {
                    if (l_CoverSprite)       Object::Destroy(l_CoverSprite.Ptr(false));
                    if (l_BackgroundSprite)  Object::Destroy(l_BackgroundSprite.Ptr(false));
                    if (l_CoverTexture)      Object::Destroy(l_CoverTexture.Ptr(false));
                    if (l_BackgroundTexture) Object::Destroy(l_BackgroundTexture.Ptr(false));

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
            if (p_CancellationToken && p_CancellationToken->IsCancelled(l_StartSerial))
                return;

            p_Callback(nullptr, nullptr);
        });
    }

}   ///< namespace ChatPlexMod_MenuMusic::Utils
