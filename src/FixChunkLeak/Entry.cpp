#include "Entry.h"
#include "ll/api/memory/Memory.h"

#include <fmt/format.h>
#include <functional>

#include <ll/api/Config.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/player/PlayerLeaveEvent.h>
#include <ll/api/io/FileUtils.h>
#include <ll/api/plugin/NativePlugin.h>
#include <ll/api/plugin/PluginManagerRegistry.h>

#include <mc/server/ServerLevel.h>
#include <mc/server/ServerPlayer.h>

#include <mc/world/ActorUniqueID.h>
#include <mc/world/components/MapDataManager.h>
#include <mc/world/level/saveddata/maps/MapItemTrackedActor.h>

#include <memory>
#include <stdexcept>
#include <unordered_map>

namespace FixChunkLeak {

namespace {

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)

std::unique_ptr<std::reference_wrapper<ll::plugin::NativePlugin>> selfPluginInstance;

// Event listeners
ll::event::ListenerPtr playerLeaveEventListener;

// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

} // namespace

auto disable(ll::plugin::NativePlugin& self) -> bool;
auto enable(ll::plugin::NativePlugin& self) -> bool;
auto load(ll::plugin::NativePlugin& self) -> bool;
auto unload(ll::plugin::NativePlugin& self) -> bool;

extern "C" {
_declspec(dllexport) auto ll_plugin_disable(ll::plugin::NativePlugin& self) -> bool { return disable(self); }
_declspec(dllexport) auto ll_plugin_enable(ll::plugin::NativePlugin& self) -> bool { return enable(self); }
_declspec(dllexport) auto ll_plugin_load(ll::plugin::NativePlugin& self) -> bool { return load(self); }
_declspec(dllexport) auto ll_plugin_unload(ll::plugin::NativePlugin& self) -> bool { return unload(self); }
}

auto disable(ll::plugin::NativePlugin& /*self*/) -> bool {
    auto& logger = getSelfPluginInstance().getLogger();

    logger.info("disabling...");

    auto& eventBus = ll::event::EventBus::getInstance();

    eventBus.removeListener(playerLeaveEventListener);

    logger.info("disabled");

    return true;
}

auto enable(ll::plugin::NativePlugin& /*self*/) -> bool {
    auto& logger = getSelfPluginInstance().getLogger();

    logger.info("enabling...");

    auto& eventBus = ll::event::EventBus::getInstance();

    playerLeaveEventListener = eventBus.emplaceListener<ll::event::player::PlayerLeaveEvent>(
        [&](ll::event::player::PlayerLeaveEvent& event) {
            auto& player = event.self();
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            auto& level = static_cast<ServerLevel&>(event.self().getLevel());
            auto& manager = level._getMapDataManager();
            auto& allMapData = ll::memory::dAccess<std::unordered_map<ActorUniqueID,std::unique_ptr<MapItemSavedData>>>(&manager, 112);
            for (auto& [id, data] : allMapData) {
                auto& v = ll::memory::dAccess<std::vector<std::shared_ptr<MapItemTrackedActor>>>(data.get(), 96);
                v.erase(std::remove_if(v.begin(), v.end(), [&player](auto& ptr) {
                    return ll::memory::dAccess<ActorUniqueID>(ptr.get(), 8) == player.getOrCreateUniqueID();
                }), v.end());
            }
        }
    );

    logger.info("enabled");

    return true;
}

auto getSelfPluginInstance() -> ll::plugin::NativePlugin& {
    if (!selfPluginInstance) {
        throw std::runtime_error("selfPluginInstance is null");
    }

    return *selfPluginInstance;
}

auto load(ll::plugin::NativePlugin& self) -> bool {
    auto& logger = self.getLogger();

    logger.info("loading...");

    selfPluginInstance = std::make_unique<std::reference_wrapper<ll::plugin::NativePlugin>>(self);

    // Your code here.

    logger.info("loaded");

    return true;
}

auto unload(ll::plugin::NativePlugin& self) -> bool {
    auto& logger = self.getLogger();

    logger.info("unloading...");

    selfPluginInstance.reset();

    // Your code here.

    logger.info("unloaded");

    return true;
}

} // namespace FixChunkLeak
