#ifndef NDS_ENGINE_CONFIG_H
#define NDS_ENGINE_CONFIG_H

#include "engine/core/types.h"

/* Minimal INI-style (key=value, '#'/';' comments) config store. Fixed
 * capacity, no per-entry heap allocations, so behavior stays predictable
 * even though it's only used at boot/settings-change time right now. */

typedef struct nds_config nds_config;

nds_config* nds_config_create(void);
void nds_config_destroy(nds_config* cfg);

nds_result nds_config_load_file(nds_config* cfg, const char* path);
nds_result nds_config_save_file(const nds_config* cfg, const char* path);

void nds_config_set_string(nds_config* cfg, const char* key, const char* value);
const char* nds_config_get_string(const nds_config* cfg, const char* key, const char* default_value);

void nds_config_set_int(nds_config* cfg, const char* key, int value);
int nds_config_get_int(const nds_config* cfg, const char* key, int default_value);

#endif /* NDS_ENGINE_CONFIG_H */
