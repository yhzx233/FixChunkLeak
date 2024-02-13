#pragma once

#include <ll/api/plugin/NativePlugin.h>

namespace FixChunkLeak {

[[nodiscard]] auto getSelfPluginInstance() -> ll::plugin::NativePlugin&;

} // namespace FixChunkLeak
