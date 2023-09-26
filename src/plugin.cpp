/**
 * @file plugin.cpp
 * @brief The main file of the plugin
 */

#include <llapi/LoggerAPI.h>
#include <llapi/EventAPI.h>
#include <llapi/HookAPI.h>

#include <llapi/GlobalServiceAPI.h>
#include <llapi/mc/LevelChunk.hpp>
#include <llapi/mc/Player.hpp>
#include <llapi/mc/MapDataManager.hpp>
#include <llapi/mc/MapItemSavedData.hpp>

// enable virtual destructor
#define ENABLE_VIRTUAL_FAKESYMBOL_CHUNKVIEWSOURCE
#include <llapi/mc/ChunkViewSource.hpp>

#define ENABLE_VIRTUAL_FAKESYMBOL_SERVERLEVEL
#include <llapi/mc/ServerLevel.hpp>

#include "version.h"

// We recommend using the global logger.
extern Logger logger;

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
        // auto& manager = dAccess<MapDataManager, 11232>(&(player->getLevel()));
        auto& manager = Global<ServerLevel>->_getMapDataManager();
        // logger.info("MapDataManager size: {}", manager.getAllMapData().size());
        for (auto& [id, data] : manager.getAllMapData()) {
            auto& v = dAccess<std::vector<std::shared_ptr<MapItemTrackedActor>>, 96>(data.get());
            v.erase(std::remove_if(v.begin(), v.end(), [player](auto& ptr) { return dAccess<ActorUniqueID, 8>(ptr.get()) == player->getActorUniqueId(); }), v.end());
        }
        return true;
	});
}