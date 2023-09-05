/**
 * @file plugin.cpp
 * @brief The main file of the plugin
 */

#include <llapi/LoggerAPI.h>
#include <llapi/EventAPI.h>
#include <llapi/HookAPI.h>

#include <llapi/mc/Level.hpp>
#include <llapi/mc/Player.hpp>
#include <llapi/mc/MapDataManager.hpp>
#include <llapi/mc/MapItemSavedData.hpp>

#include "version.h"

// We recommend using the global logger.
extern Logger logger;

void hookLiteLoader();

/**
 * @brief The entrypoint of the plugin. DO NOT remove or rename this function.
 *        
 */
void PluginInit()
{
    // Your code here
    // logger.info("Hello, world!");

    Event::PlayerLeftEvent::subscribe([](Event::PlayerLeftEvent ev) {
        auto player = ev.mPlayer;
        auto& manager = dAccess<MapDataManager, 1031*8>(&(player->getLevel()));
        // logger.info("MapDataManager size: {}", manager.getAllMapData().size());
        for (auto& [id, data] : manager.getAllMapData()) {
            auto& v = dAccess<std::vector<std::shared_ptr<MapItemTrackedActor>>, 96>(data.get());
            v.erase(std::remove_if(v.begin(), v.end(), [player](auto& ptr) { return dAccess<ActorUniqueID, 8>(ptr.get()) == player->getActorUniqueId(); }), v.end());
        }
        return true;
	});

    hookLiteLoader();
}

#include <llapi/mc/SimulatedPlayer.hpp>

// enable virtual destructor
#define ENABLE_VIRTUAL_FAKESYMBOL_CHUNKVIEWSOURCE
#include <llapi/mc/ChunkViewSource.hpp>

static_assert(sizeof(ChunkSource) == 0x60);
static_assert(sizeof(ChunkViewSource) == 0x1e8);

struct SimulatedPlayerHook {
    std::shared_ptr<class ChunkViewSource> hookCreateChunkSource(ChunkSource& chunkSource) {
        return std::make_shared<ChunkViewSource>(chunkSource, ChunkSource::LoadMode::Deferred);
    }
};

void hookLiteLoader() {
    static void* original;
    auto imageBase = GetModuleHandleA("LiteLoader.dll");
    if (!imageBase) {
        logger.error("LiteLoader not found");
        return;
    }
    auto addr = (uintptr_t)imageBase + 0x8D770;
    THookRegister((void*)addr, &SimulatedPlayerHook::hookCreateChunkSource, &original);
    // logger.info("LiteLoader hook {}", (void*)addr);
}