#include "engine/scene/instance.h"

#include <stdlib.h>
#include <string.h>

struct nds_instance {
    nds_instance_class class_id;
    char* name;
    nds_instance* parent;
    nds_instance** children;
    size_t child_count;
    size_t child_capacity;
};

static char* copy_string(const char* value)
{
    size_t length;
    char* copy;
    if (!value) value = "";
    length = strlen(value);
    copy = (char*)malloc(length + 1);
    if (!copy) return NULL;
    memcpy(copy, value, length + 1);
    return copy;
}

const char* nds_instance_class_name(nds_instance_class class_id)
{
    switch (class_id) {
        case NDS_CLASS_DATAMODEL: return "DataModel";
        case NDS_CLASS_FOLDER: return "Folder";
        case NDS_CLASS_MODEL: return "Model";
        case NDS_CLASS_PART: return "Part";
        case NDS_CLASS_SPAWN_POINT: return "SpawnPoint";
        case NDS_CLASS_CAMERA: return "Camera";
        case NDS_CLASS_CHARACTER: return "Character";
        case NDS_CLASS_PLAYER: return "Player";
        case NDS_CLASS_VALUE: return "Value";
        default: return "Instance";
    }
}

nds_instance* nds_instance_create(nds_instance_class class_id, const char* name)
{
    nds_instance* instance = (nds_instance*)calloc(1, sizeof(*instance));
    if (!instance) return NULL;
    instance->class_id = class_id;
    instance->name = copy_string(name ? name : nds_instance_class_name(class_id));
    if (!instance->name) {
        free(instance);
        return NULL;
    }
    return instance;
}

static int remove_child(nds_instance* parent, nds_instance* child)
{
    size_t i;
    if (!parent || !child) return 0;
    for (i = 0; i < parent->child_count; ++i) {
        if (parent->children[i] == child) {
            memmove(&parent->children[i], &parent->children[i + 1],
                    (parent->child_count - i - 1) * sizeof(parent->children[0]));
            --parent->child_count;
            return 1;
        }
    }
    return 0;
}

nds_result nds_instance_set_parent(nds_instance* instance, nds_instance* parent)
{
    nds_instance* cursor;
    nds_instance** grown;
    size_t new_capacity;

    if (!instance) return NDS_ERR_INVALID_ARG;
    if (parent == instance) return NDS_ERR_INVALID_ARG;

    /* Prevent cycles in the object tree. */
    cursor = parent;
    while (cursor) {
        if (cursor == instance) return NDS_ERR_INVALID_ARG;
        cursor = cursor->parent;
    }

    if (instance->parent == parent) return NDS_OK;
    if (instance->parent) remove_child(instance->parent, instance);
    instance->parent = NULL;

    if (!parent) return NDS_OK;

    if (parent->child_count == parent->child_capacity) {
        new_capacity = parent->child_capacity ? parent->child_capacity * 2 : 8;
        grown = (nds_instance**)realloc(parent->children,
                                        new_capacity * sizeof(parent->children[0]));
        if (!grown) return NDS_ERR_UNKNOWN;
        parent->children = grown;
        parent->child_capacity = new_capacity;
    }

    parent->children[parent->child_count++] = instance;
    instance->parent = parent;
    return NDS_OK;
}

void nds_instance_destroy(nds_instance* instance)
{
    size_t i;
    if (!instance) return;
    if (instance->parent) remove_child(instance->parent, instance);
    for (i = instance->child_count; i > 0; --i) {
        instance->children[i - 1]->parent = NULL;
        nds_instance_destroy(instance->children[i - 1]);
    }
    free(instance->children);
    free(instance->name);
    free(instance);
}

nds_instance_class nds_instance_get_class(const nds_instance* instance)
{
    return instance ? instance->class_id : NDS_CLASS_INSTANCE;
}

const char* nds_instance_get_name(const nds_instance* instance)
{
    return instance ? instance->name : "";
}

void nds_instance_set_name(nds_instance* instance, const char* name)
{
    char* replacement;
    if (!instance) return;
    replacement = copy_string(name ? name : "");
    if (!replacement) return;
    free(instance->name);
    instance->name = replacement;
}

nds_instance* nds_instance_get_parent(const nds_instance* instance)
{
    return instance ? instance->parent : NULL;
}

size_t nds_instance_child_count(const nds_instance* instance)
{
    return instance ? instance->child_count : 0;
}

nds_instance* nds_instance_child_at(const nds_instance* instance, size_t index)
{
    if (!instance || index >= instance->child_count) return NULL;
    return instance->children[index];
}

nds_instance* nds_instance_find_child(const nds_instance* instance, const char* name)
{
    size_t i;
    if (!instance || !name) return NULL;
    for (i = 0; i < instance->child_count; ++i) {
        if (strcmp(instance->children[i]->name, name) == 0) return instance->children[i];
    }
    return NULL;
}

nds_instance* nds_instance_find_descendant(const nds_instance* instance, const char* name)
{
    size_t i;
    nds_instance* found;
    if (!instance || !name) return NULL;
    for (i = 0; i < instance->child_count; ++i) {
        if (strcmp(instance->children[i]->name, name) == 0) return instance->children[i];
        found = nds_instance_find_descendant(instance->children[i], name);
        if (found) return found;
    }
    return NULL;
}

nds_instance* nds_instance_find_path(const nds_instance* root, const char* path)
{
    const char* start;
    const char* end;
    size_t length;
    char* segment;
    nds_instance* current = (nds_instance*)root;

    if (!root || !path || !*path) return current;
    start = path;

    /* A path may start with the root name. */
    end = strchr(start, '.');
    length = end ? (size_t)(end - start) : strlen(start);
    segment = (char*)malloc(length + 1);
    if (!segment) return NULL;
    memcpy(segment, start, length);
    segment[length] = '\0';
    if (strcmp(segment, current->name) == 0) start += length + (end ? 1 : 0);
    free(segment);

    while (*start) {
        nds_instance* next;
        end = strchr(start, '.');
        length = end ? (size_t)(end - start) : strlen(start);
        segment = (char*)malloc(length + 1);
        if (!segment) return NULL;
        memcpy(segment, start, length);
        segment[length] = '\0';
        next = nds_instance_find_child(current, segment);
        free(segment);
        if (!next) return NULL;
        current = next;
        if (!end) break;
        start = end + 1;
    }
    return current;
}
