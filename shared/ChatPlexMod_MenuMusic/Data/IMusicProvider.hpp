#pragma once

#include "EMusicProviderType.hpp"

#include <vector>
#include <memory>

#include <CP_SDK/Utils/Il2cpp.hpp>

namespace ChatPlexMod_MenuMusic { namespace Data {

    class Music;

    /// @brief Music provider interface
    class IMusicProvider
    {
        DISABLE_COPY_MOVECONSTRUCTORS(IMusicProvider);

        protected:
            PRIVATE_TAG();

        public:
            virtual MusicProviderType::E                        Type()          = 0;
            virtual bool                                        IsReady()       = 0;
            virtual bool                                        SupportPlayIt() = 0;
            virtual const std::vector<std::shared_ptr<Music>>&  Musics()        = 0;

        public:
            using Ptr = std::shared_ptr<IMusicProvider>;

            /// @brief Constructor
            IMusicProvider(PRIVATE_TAG_ARG());
            /// @brief Destructor
            virtual ~IMusicProvider();

        public:
            /// @brief Init
            /// @param p_SelfPtr Self ptr
            virtual void Init(const Ptr& p_SelfPtr) = 0;

        public:
            /// @brief Per game implementation of the Play It button
            /// @param p_Music Target music
            virtual bool StartGameSpecificGamePlay(const std::shared_ptr<Music>& p_Music) = 0;

    };

}   ///< namespace Data
}   ///< namespace ChatPlexMod_MenuMusic