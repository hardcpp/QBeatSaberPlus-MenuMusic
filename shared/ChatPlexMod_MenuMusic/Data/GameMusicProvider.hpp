#pragma once

#include "IMusicProvider.hpp"

#include <custom-types/shared/coroutine.hpp>

namespace ChatPlexMod_MenuMusic { namespace Data {

    /// @brief Music provider interface
    class GameMusicProvider : public IMusicProvider
    {
        CP_SDK_NO_COPYMOVE_CTORS(GameMusicProvider);

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
            GameMusicProvider();
            /// @brief Destructor
            virtual ~GameMusicProvider();

        public:
            /// @brief Init
            /// @param p_SelfPtr Self ptr
            void Init(const Ptr& p_SelfPtr) override;

        public:
            /// @brief Per game implementation of the Play It button
            /// @param p_Music Target music
            bool StartGameSpecificGamePlay(const std::shared_ptr<Music>& p_Music) override;

        private:
            /// @brief Load game songs
            static custom_types::Helpers::Coroutine Coroutine_LoadGameSongs(Ptr p_Self);

    };

}   ///< namespace Data
}   ///< namespace ChatPlexMod_MenuMusic