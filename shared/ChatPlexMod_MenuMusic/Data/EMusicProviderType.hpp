#pragma once

#include <CP_SDK/Utils/Il2cpp.hpp>

namespace ChatPlexMod_MenuMusic { namespace Data {

    struct MusicProviderType
    {
        enum class E
        {
            GameMusic,
            CustomMusic,
            //GamePlaylistMusic,
        };

        static constexpr std::array<std::u16string_view, 2> const S = {
            u"GameMusic",
            u"CustomMusic",
            //u"GamePlaylistMusic",
        };

        CP_SDK_IL2CPP_ENUM_UTILS();

    };

}   ///< namespace Data
}   ///< namespace ChatPlexMod_MenuMusic