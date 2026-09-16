#ifndef NDS_ENGINE_RBXLX_LOADER_H
#define NDS_ENGINE_RBXLX_LOADER_H

#include "engine/core/types.h"
#include "engine/scene/instance.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Import the geometry-oriented subset of a Roblox XML place/model. Parts and
 * SpawnLocations are imported as flat children of a DataModel root. Scripts,
 * services, terrain and unsupported Roblox classes are intentionally ignored.
 */
nds_result nds_rbxlx_load(const char* path, nds_instance** out_root);

#ifdef __cplusplus
}
#endif

#endif
