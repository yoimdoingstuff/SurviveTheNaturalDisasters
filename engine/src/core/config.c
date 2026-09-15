#include "engine/core/config.h"
#include "engine/platform/platform.h"
#include "engine/core/log.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define NDS_CONFIG_MAX_ENTRIES 256
#define NDS_CONFIG_KEY_LEN 64
#define NDS_CONFIG_VALUE_LEN 192

static const char* TAG = "config";

typedef struct nds_config_entry {
    char key[NDS_CONFIG_KEY_LEN];
    char value[NDS_CONFIG_VALUE_LEN];
    int used;
} nds_config_entry;

struct nds_config {
    nds_config_entry entries[NDS_CONFIG_MAX_ENTRIES];
    int count;
};

nds_config* nds_config_create(void)
{
    return (nds_config*)calloc(1, sizeof(nds_config));
}

void nds_config_destroy(nds_config* cfg)
{
    free(cfg);
}

static nds_config_entry* find_entry(nds_config* cfg, const char* key)
{
    for (int i = 0; i < cfg->count; ++i) {
        if (cfg->entries[i].used && strncmp(cfg->entries[i].key, key, NDS_CONFIG_KEY_LEN) == 0) {
            return &cfg->entries[i];
        }
    }
    return NULL;
}

static const nds_config_entry* find_entry_const(const nds_config* cfg, const char* key)
{
    for (int i = 0; i < cfg->count; ++i) {
        if (cfg->entries[i].used && strncmp(cfg->entries[i].key, key, NDS_CONFIG_KEY_LEN) == 0) {
            return &cfg->entries[i];
        }
    }
    return NULL;
}

void nds_config_set_string(nds_config* cfg, const char* key, const char* value)
{
    if (!cfg || !key || !value) {
        return;
    }

    nds_config_entry* entry = find_entry(cfg, key);
    if (!entry) {
        if (cfg->count >= NDS_CONFIG_MAX_ENTRIES) {
            NDS_LOGW(TAG, "config table full (%d entries), dropping key '%s'", NDS_CONFIG_MAX_ENTRIES, key);
            return;
        }
        entry = &cfg->entries[cfg->count++];
        entry->used = 1;
        strncpy(entry->key, key, NDS_CONFIG_KEY_LEN - 1);
        entry->key[NDS_CONFIG_KEY_LEN - 1] = '\0';
    }

    strncpy(entry->value, value, NDS_CONFIG_VALUE_LEN - 1);
    entry->value[NDS_CONFIG_VALUE_LEN - 1] = '\0';
}

const char* nds_config_get_string(const nds_config* cfg, const char* key, const char* default_value)
{
    if (!cfg || !key) {
        return default_value;
    }
    const nds_config_entry* entry = find_entry_const(cfg, key);
    return entry ? entry->value : default_value;
}

void nds_config_set_int(nds_config* cfg, const char* key, int value)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", value);
    nds_config_set_string(cfg, key, buf);
}

int nds_config_get_int(const nds_config* cfg, const char* key, int default_value)
{
    const char* raw = nds_config_get_string(cfg, key, NULL);
    if (!raw) {
        return default_value;
    }
    return atoi(raw);
}

nds_result nds_config_load_file(nds_config* cfg, const char* path)
{
    if (!cfg || !path) {
        return NDS_ERR_INVALID_ARG;
    }

    platform_file* file = platform_file_open(path, "rb");
    if (!file) {
        return NDS_ERR_IO;
    }

    long size = platform_file_size(file);
    if (size <= 0) {
        platform_file_close(file);
        return NDS_ERR_IO;
    }

    char* buffer = (char*)malloc((size_t)size + 1);
    if (!buffer) {
        platform_file_close(file);
        return NDS_ERR_UNKNOWN;
    }

    size_t read = platform_file_read(file, buffer, (size_t)size);
    platform_file_close(file);
    buffer[read] = '\0';

    char* cursor = buffer;
    while (*cursor) {
        char* line_end = strchr(cursor, '\n');
        if (line_end) {
            *line_end = '\0';
        }

        char* trimmed = cursor;
        while (*trimmed == ' ' || *trimmed == '\t' || *trimmed == '\r') {
            trimmed++;
        }

        if (*trimmed != '\0' && *trimmed != '#' && *trimmed != ';') {
            char* eq = strchr(trimmed, '=');
            if (eq) {
                *eq = '\0';
                char* key = trimmed;
                char* value = eq + 1;

                char* key_end = key + strlen(key);
                while (key_end > key && (key_end[-1] == ' ' || key_end[-1] == '\t')) {
                    *(--key_end) = '\0';
                }

                while (*value == ' ' || *value == '\t') {
                    value++;
                }
                char* value_end = value + strlen(value);
                while (value_end > value && (value_end[-1] == '\r' || value_end[-1] == ' ')) {
                    *(--value_end) = '\0';
                }

                nds_config_set_string(cfg, key, value);
            }
        }

        cursor = line_end ? line_end + 1 : cursor + strlen(cursor);
    }

    free(buffer);
    return NDS_OK;
}

nds_result nds_config_save_file(const nds_config* cfg, const char* path)
{
    if (!cfg || !path) {
        return NDS_ERR_INVALID_ARG;
    }

    platform_file* file = platform_file_open(path, "wb");
    if (!file) {
        return NDS_ERR_IO;
    }

    char line[NDS_CONFIG_KEY_LEN + NDS_CONFIG_VALUE_LEN + 4];
    for (int i = 0; i < cfg->count; ++i) {
        if (!cfg->entries[i].used) {
            continue;
        }
        int n = snprintf(line, sizeof(line), "%s=%s\n", cfg->entries[i].key, cfg->entries[i].value);
        if (n > 0) {
            platform_file_write(file, line, (size_t)n);
        }
    }

    platform_file_close(file);
    return NDS_OK;
}
