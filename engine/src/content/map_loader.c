#include "engine/content/map_loader.h"
#include "engine/scene/part.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NDS_MAP_MAX_INSTANCES 65536u

typedef struct map_entry {
    int id;
    int parent;
    char* class_name;
    char* name;
    nds_vec3 position;
    nds_vec3 size;
    nds_vec3 rotation;
    float transparency;
    float reflectance;
    uint32_t color_rgba;
    unsigned char has_position, has_size, has_rotation;
    unsigned char has_transparency, has_reflectance, has_color;
    unsigned char anchored, can_collide;
    unsigned char has_anchored, has_can_collide;
} map_entry;

typedef struct parser { const char* p; const char* end; } parser;

static void ws(parser* p) { while (p->p < p->end && isspace((unsigned char)*p->p)) ++p->p; }
static int ch(parser* p, char c) { ws(p); if (p->p >= p->end || *p->p != c) return 0; ++p->p; return 1; }

static char* json_string(parser* p)
{
    size_t cap = 32, len = 0; char* out; char c;
    ws(p); if (p->p >= p->end || *p->p++ != '"') return NULL;
    out = (char*)malloc(cap); if (!out) return NULL;
    while (p->p < p->end) {
        c = *p->p++;
        if (c == '"') { out[len] = '\0'; return out; }
        if (c == '\\') {
            if (p->p >= p->end) { free(out); return NULL; }
            c = *p->p++;
            if (c == '"' || c == '\\' || c == '/') { }
            else if (c == 'n') c = '\n'; else if (c == 'r') c = '\r'; else if (c == 't') c = '\t';
            else { free(out); return NULL; }
        }
        if (len + 1 >= cap) { char* grown; cap *= 2; grown = (char*)realloc(out, cap); if (!grown) { free(out); return NULL; } out = grown; }
        out[len++] = c;
    }
    free(out); return NULL;
}

static int number(parser* p, double* out)
{
    char* endptr; ws(p); if (p->p >= p->end) return 0;
    *out = strtod(p->p, &endptr); if (endptr == p->p || endptr > p->end) return 0; p->p = endptr; return 1;
}

static int literal(parser* p, const char* text)
{
    size_t n = strlen(text); ws(p); if ((size_t)(p->end - p->p) < n || strncmp(p->p, text, n) != 0) return 0; p->p += n; return 1;
}

static int skip_value(parser* p);
static int skip_array(parser* p)
{
    ws(p); if (!ch(p, '[')) return 0; ws(p); if (ch(p, ']')) return 1;
    for (;;) { if (!skip_value(p)) return 0; ws(p); if (ch(p, ']')) return 1; if (!ch(p, ',')) return 0; }
}
static int skip_object(parser* p)
{
    char* key; ws(p); if (!ch(p, '{')) return 0; ws(p); if (ch(p, '}')) return 1;
    for (;;) { key = json_string(p); free(key); if (!key || !ch(p, ':') || !skip_value(p)) return 0; ws(p); if (ch(p, '}')) return 1; if (!ch(p, ',')) return 0; }
}
static int skip_value(parser* p)
{
    char c; ws(p); if (p->p >= p->end) return 0; c = *p->p;
    if (c == '"') { char* s = json_string(p); free(s); return s != NULL; }
    if (c == '{') return skip_object(p); if (c == '[') return skip_array(p);
    if (c == 't') return literal(p, "true"); if (c == 'f') return literal(p, "false"); if (c == 'n') return literal(p, "null");
    { double v; return number(p, &v); }
}

static int vector3(parser* p, nds_vec3* v)
{
    double x, y, z; if (!ch(p, '[') || !number(p, &x) || !ch(p, ',') || !number(p, &y) || !ch(p, ',') || !number(p, &z) || !ch(p, ']')) return 0;
    v->x = (float)x; v->y = (float)y; v->z = (float)z; return 1;
}

static nds_instance_class class_id(const char* name)
{
    if (strcmp(name, "DataModel") == 0 || strcmp(name, "Workspace") == 0) return NDS_CLASS_DATAMODEL;
    if (strcmp(name, "Folder") == 0) return NDS_CLASS_FOLDER;
    if (strcmp(name, "Model") == 0) return NDS_CLASS_MODEL;
    if (strcmp(name, "Part") == 0 || strcmp(name, "WedgePart") == 0 || strcmp(name, "CornerWedgePart") == 0 || strcmp(name, "TrussPart") == 0 || strcmp(name, "MeshPart") == 0) return NDS_CLASS_PART;
    if (strcmp(name, "SpawnLocation") == 0) return NDS_CLASS_SPAWN_POINT;
    if (strcmp(name, "Camera") == 0) return NDS_CLASS_CAMERA;
    if (strcmp(name, "Character") == 0) return NDS_CLASS_CHARACTER;
    if (strcmp(name, "Player") == 0) return NDS_CLASS_PLAYER;
    return NDS_CLASS_INSTANCE;
}

static int parse_entry(parser* p, map_entry* e)
{
    char* key; char* str; double v; int first = 1;
    memset(e, 0, sizeof(*e)); e->id = -1; e->parent = -1;
    if (!ch(p, '{')) return 0; ws(p); if (ch(p, '}')) return 1;
    for (;;) {
        key = json_string(p); if (!key || !ch(p, ':')) { free(key); return 0; }
        if (strcmp(key, "id") == 0) { if (!number(p, &v)) { free(key); return 0; } e->id = (int)v; }
        else if (strcmp(key, "parent") == 0) { ws(p); if (strncmp(p->p, "null", 4) == 0) { p->p += 4; e->parent = -1; } else { if (!number(p, &v)) { free(key); return 0; } e->parent = (int)v; } }
        else if (strcmp(key, "class") == 0) { free(e->class_name); e->class_name = json_string(p); if (!e->class_name) { free(key); return 0; } }
        else if (strcmp(key, "name") == 0) { free(e->name); e->name = json_string(p); if (!e->name) { free(key); return 0; } }
        else if (strcmp(key, "transform") == 0) {
            if (!ch(p, '{')) { free(key); return 0; } ws(p);
            if (!ch(p, '}')) for (;;) {
                char* tk = json_string(p); if (!tk || !ch(p, ':')) { free(tk); free(key); return 0; }
                if (strcmp(tk, "position") == 0) e->has_position = vector3(p, &e->position);
                else if (strcmp(tk, "size") == 0) e->has_size = vector3(p, &e->size);
                else if (strcmp(tk, "rotation") == 0) e->has_rotation = vector3(p, &e->rotation);
                else if (!skip_value(p)) { free(tk); free(key); return 0; }
                free(tk); ws(p); if (ch(p, '}')) break; if (!ch(p, ',')) { free(key); return 0; }
            }
        } else if (strcmp(key, "part") == 0) {
            if (!ch(p, '{')) { free(key); return 0; } ws(p);
            if (!ch(p, '}')) for (;;) {
                char* pk = json_string(p); if (!pk || !ch(p, ':')) { free(pk); free(key); return 0; }
                if (strcmp(pk, "transparency") == 0 && number(p, &v)) { e->transparency=(float)v; e->has_transparency=1; }
                else if (strcmp(pk, "reflectance") == 0 && number(p, &v)) { e->reflectance=(float)v; e->has_reflectance=1; }
                else if (strcmp(pk, "anchored") == 0) { ws(p); if (literal(p,"true")) { e->anchored=1; e->has_anchored=1; } else if (literal(p,"false")) e->has_anchored=1; else { free(pk); free(key); return 0; } }
                else if (strcmp(pk, "can_collide") == 0) { ws(p); if (literal(p,"true")) { e->can_collide=1; e->has_can_collide=1; } else if (literal(p,"false")) e->has_can_collide=1; else { free(pk); free(key); return 0; } }
                else if (strcmp(pk, "color") == 0) { nds_vec3 c; if (!vector3(p,&c)) { free(pk); free(key); return 0; } e->color_rgba=((uint32_t)c.x<<24)|((uint32_t)c.y<<16)|((uint32_t)c.z<<8)|255u; e->has_color=1; }
                else if (!skip_value(p)) { free(pk); free(key); return 0; }
                free(pk); ws(p); if (ch(p, '}')) break; if (!ch(p, ',')) { free(key); return 0; }
            }
        } else if (!skip_value(p)) { free(key); return 0; }
        free(key); ws(p); if (ch(p, '}')) return 1; if (!ch(p, ',')) return 0; first = 0; (void)first;
    }
}

static void free_entries(map_entry* entries, size_t count)
{ size_t i; for (i=0;i<count;++i) { free(entries[i].class_name); free(entries[i].name); } free(entries); }

nds_result nds_map_load_json_text(const char* text, nds_instance** out_root)
{
    parser p; map_entry* entries; size_t count=0, capacity=64, i; nds_instance** objects; nds_instance* root=NULL;
    if (!text || !out_root) return NDS_ERR_INVALID_ARG; *out_root=NULL; entries=(map_entry*)calloc(capacity,sizeof(*entries)); if (!entries) return NDS_ERR_UNKNOWN;
    p.p=text; p.end=text+strlen(text); ws(&p);
    if (!ch(&p,'{')) { free(entries); return NDS_ERR_IO; }
    while (1) {
        char* key=json_string(&p); if (!key || !ch(&p,':')) { free(key); free_entries(entries,count); return NDS_ERR_IO; }
        if (strcmp(key,"instances")==0) {
            if (!ch(&p,'[')) { free(key); free_entries(entries,count); return NDS_ERR_IO; } ws(&p);
            if (!ch(&p,']')) for (;;) {
                if (count==capacity) { map_entry* grown; capacity*=2; if (capacity>NDS_MAP_MAX_INSTANCES) { free(key); free_entries(entries,count); return NDS_ERR_UNKNOWN; } grown=(map_entry*)realloc(entries,capacity*sizeof(*entries)); if (!grown) { free(key); free_entries(entries,count); return NDS_ERR_UNKNOWN; } entries=grown; }
                if (!parse_entry(&p,&entries[count])) { free(key); free_entries(entries,count); return NDS_ERR_IO; } ++count; ws(&p); if (ch(&p,']')) break; if (!ch(&p,',')) { free(key); free_entries(entries,count); return NDS_ERR_IO; }
            }
        } else if (!skip_value(&p)) { free(key); free_entries(entries,count); return NDS_ERR_IO; }
        free(key); ws(&p); if (ch(&p,'}')) break; if (!ch(&p,',')) { free_entries(entries,count); return NDS_ERR_IO; }
    }
    objects=(nds_instance**)calloc(count ? count : 1,sizeof(*objects)); if (!objects) { free_entries(entries,count); return NDS_ERR_UNKNOWN; }
    for (i=0;i<count;++i) { objects[i]=nds_instance_create(class_id(entries[i].class_name?entries[i].class_name:"Instance"),entries[i].name?entries[i].name:"Instance"); if (!objects[i]) { size_t j; for(j=0;j<i;++j) nds_instance_destroy(objects[j]); free(objects); free_entries(entries,count); return NDS_ERR_UNKNOWN; } }
    for (i=0;i<count;++i) {
        map_entry* e=&entries[i];
        if (e->parent>=0 && (size_t)e->parent<count) { if (nds_instance_set_parent(objects[i],objects[e->parent])!=NDS_OK) { size_t j; for(j=0;j<count;++j) nds_instance_destroy(objects[j]); free(objects); free_entries(entries,count); return NDS_ERR_UNKNOWN; } }
        else if (!root) root=objects[i];
        if (nds_instance_get_class(objects[i])==NDS_CLASS_PART || nds_instance_get_class(objects[i])==NDS_CLASS_SPAWN_POINT) {
            nds_part_properties props; if (nds_part_get_properties(objects[i],&props)==NDS_OK) {
                if(e->has_position)props.position=e->position; if(e->has_size)props.size=e->size; if(e->has_rotation)props.rotation=e->rotation; if(e->has_transparency)props.transparency=e->transparency; if(e->has_reflectance)props.reflectance=e->reflectance; if(e->has_color)props.color_rgba=e->color_rgba; if(e->has_anchored)props.anchored=e->anchored; if(e->has_can_collide)props.can_collide=e->can_collide; nds_part_set_properties(objects[i],&props);
            }
        }
    }
    free(objects); free_entries(entries,count); if (!root) return NDS_ERR_IO; *out_root=root; return NDS_OK;
}

nds_result nds_map_load_json(const char* path, nds_instance** out_root)
{
    FILE* f; long size; char* data; size_t read_count; nds_result result;
    if (!path || !out_root) return NDS_ERR_INVALID_ARG; *out_root=NULL; f=fopen(path,"rb"); if(!f)return NDS_ERR_IO;
    if(fseek(f,0,SEEK_END)!=0){fclose(f);return NDS_ERR_IO;} size=ftell(f); if(size<0){fclose(f);return NDS_ERR_IO;} if(fseek(f,0,SEEK_SET)!=0){fclose(f);return NDS_ERR_IO;}
    data=(char*)malloc((size_t)size+1); if(!data){fclose(f);return NDS_ERR_UNKNOWN;} read_count=fread(data,1,(size_t)size,f); fclose(f); if(read_count!=(size_t)size){free(data);return NDS_ERR_IO;} data[size]='\0'; result=nds_map_load_json_text(data,out_root); free(data); return result;
}
