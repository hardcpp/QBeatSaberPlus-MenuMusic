#pragma once

#include "Data/EMusicProviderType.hpp"

#include <CP_SDK/Config/JsonConfig.hpp>

namespace ChatPlexMod_MenuMusic {

    class MMConfig : public CP_SDK::Config::JsonConfig
    {
        CP_SDK_CONFIG_JSONCONFIG_INSTANCE_DECL(MMConfig);

        public:
            bool Enabled = false;

            Data::MusicProviderType::E MusicProvider = Data::MusicProviderType::E::GameMusic;

            bool ShowPlayer = true;

            bool    StartSongFromBeginning          = false;
            bool    StartANewMusicOnSceneChange     = true;
            bool    LoopCurrentMusic                = false;
            bool    UseOnlyCustomMenuSongsFolder    = false;
            float   PlaybackVolume                  = 0.5f;

        protected:
            /// @brief Reset config to default
            void Reset_Impl() override;

        protected:
            /// @brief Write the document
            /// @param p_Document Target
            CP_SDK_JSON_SERIALIZE_DECL() override;
            /// @brief Read the document
            /// @param p_Document Source
            CP_SDK_JSON_UNSERIALIZE_DECL() override;

        protected:
            /// @brief Get relative config path
            std::filesystem::path GetRelativePath() override;

        protected:
            /// @brief On config init
            /// @param p_OnCreation On creation
            void OnInit(bool p_OnCreation) override;

    };

}   ///< namespace ChatPlexMod_MenuMusic