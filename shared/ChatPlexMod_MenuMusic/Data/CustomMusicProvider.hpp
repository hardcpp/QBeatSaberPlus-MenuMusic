#pragma once

#include "IMusicProvider.hpp"

#include <custom-types/shared/coroutine.hpp>

namespace ChatPlexMod_MenuMusic { namespace Data {

    /// @brief Music provider interface
    class CustomMusicProvider : public IMusicProvider, public std::enable_shared_from_this<CustomMusicProvider>
    {
        CP_SDK_NO_COPYMOVE_CTORS(CustomMusicProvider);

        private:
            std::vector<std::shared_ptr<Music>> m_Musics;
            bool                                m_IsLoading;

        public:
            MusicProviderType::E                        Type()          override;
            bool                                        IsReady()       override;
            bool                                        SupportPlayIt() override;
            const std::vector<std::shared_ptr<Music>>&  Musics()        override;

        public:
            /// @brief Constructor
            CustomMusicProvider();
            /// @brief Destructor
            virtual ~CustomMusicProvider();

        public:
            /// @brief Init
            /// @param p_SelfPtr Self ptr
            void Init(const Ptr& p_SelfPtr) override;

        public:
            /// @brief Per game implementation of the Play It button
            /// @param p_Music Target music
            bool StartGameSpecificGamePlay(const std::shared_ptr<Music>& p_Music) override;
            /// @brief Shuffle music collection
            void Shuffle() override;

        private:
            /// @brief Load game songs
            static custom_types::Helpers::Coroutine Coroutine_Load(Ptr p_Self);

    };

}   ///< namespace Data
}   ///< namespace ChatPlexMod_MenuMusic