#include "engine/content/map_loader.h"
#include "engine/core/log.h"
#include "engine/scene/part.h"
#include "engine/render/mesh_cache.h"
#include "engine/render/texture_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The map loader implementation is intentionally kept source-compatible with
 * the existing JSON parser. This small compatibility fix uses the canonical
 * child_count/child_at Instance API while resolving imported assets. */
