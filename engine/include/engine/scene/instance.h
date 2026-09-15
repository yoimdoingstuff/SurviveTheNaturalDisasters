#ifndef NDS_ENGINE_INSTANCE_H
#define NDS_ENGINE_INSTANCE_H

#include "engine/core/types.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nds_instance nds_instance;

typedef enum nds_instance_class {
    NDS_CLASS_INSTANCE = 0,
    NDS_CLASS_DATAMODEL,
    NDS_CLASS_FOLDER,
    NDS_CLASS_MODEL,
    NDS_CLASS_PART,
    NDS_CLASS_SPAWN_POINT,
    NDS_CLASS_CAMERA,
    NDS_CLASS_CHARACTER,
    NDS_CLASS_PLAYER,
    NDS_CLASS_VALUE
} nds_instance_class;

nds_instance* nds_instance_create(nds_instance_class class_id, const char* name);
void nds_instance_destroy(nds_instance* instance);
nds_instance_class nds_instance_get_class(const nds_instance* instance);
const char* nds_instance_get_name(const nds_instance* instance);
void nds_instance_set_name(nds_instance* instance, const char* name);
nds_instance* nds_instance_get_parent(const nds_instance* instance);
nds_result nds_instance_set_parent(nds_instance* instance, nds_instance* parent);
size_t nds_instance_child_count(const nds_instance* instance);
nds_instance* nds_instance_child_at(const nds_instance* instance, size_t index);
nds_instance* nds_instance_find_child(const nds_instance* instance, const char* name);
nds_instance* nds_instance_find_descendant(const nds_instance* instance, const char* name);
nds_instance* nds_instance_find_path(const nds_instance* root, const char* path);
const char* nds_instance_class_name(nds_instance_class class_id);

#ifdef __cplusplus
}
#endif

#endif
