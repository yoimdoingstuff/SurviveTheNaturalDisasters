#include "engine/content/rbxlx_loader.h"
#include "engine/scene/part.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char* read_file(const char* path, size_t* out_size)
{
    FILE* f; long size; char* data;
    if (!path || !out_size) return NULL;
    f=fopen(path,"rb"); if(!f)return NULL;
    if(fseek(f,0,SEEK_END)!=0){fclose(f);return NULL;}
    size=ftell(f); if(size<0||size>64L*1024L*1024L){fclose(f);return NULL;}
    if(fseek(f,0,SEEK_SET)!=0){fclose(f);return NULL;}
    data=(char*)malloc((size_t)size+1); if(!data){fclose(f);return NULL;}
    if(fread(data,1,(size_t)size,f)!=(size_t)size){free(data);fclose(f);return NULL;}
    data[size]='\0'; fclose(f); *out_size=(size_t)size; return data;
}

static const char* find_item_end(const char* begin, const char* end)
{
    const char* p=begin; int depth=1;
    while(p<end){
        const char* open=strstr(p,"<Item");
        const char* close=strstr(p,"</Item>");
        if(!close)return NULL;
        if(open && open<close){++depth;p=open+5;}else{--depth;p=close+7;if(depth==0)return p;}
    }
    return NULL;
}

static int attr_value(const char* item,const char* item_end,const char* name,char* out,size_t out_size)
{
    char needle[64]; const char* p; const char* q; size_t n;
    snprintf(needle,sizeof(needle),"%s=\"",name); p=item;
    while(p<item_end && (q=strstr(p,needle))!=NULL){
        const char* value=q+strlen(needle); const char* quote=strchr(value,'\"');
        if(!quote||quote>item_end)return 0; n=(size_t)(quote-value); if(n>=out_size)n=out_size-1;
        memcpy(out,value,n);out[n]='\0';return 1;
    }
    return 0;
}

static int tag_text(const char* begin,const char* end,const char* tag,char* out,size_t out_size)
{
    char open[96]; const char* p; const char* q; size_t n;
    snprintf(open,sizeof(open),"<%s",tag); p=begin;
    while((q=strstr(p,open))!=NULL && q<end){
        const char* gt=strchr(q,'>'); if(!gt||gt>=end)break;
        ++gt; const char* close=strstr(gt,"</"); if(!close||close>=end)break;
        n=(size_t)(close-gt); while(n&&isspace((unsigned char)gt[0])){++gt;--n;} while(n&&isspace((unsigned char)gt[n-1]))--n;
        if(n>=out_size)n=out_size-1;memcpy(out,gt,n);out[n]='\0';return 1;
    }
    return 0;
}

static float property_float(const char* begin,const char* end,const char* tag,float fallback)
{
    char text[64];char* ep;float v;if(!tag_text(begin,end,tag,text,sizeof(text)))return fallback;v=strtof(text,&ep);return ep!=text?v:fallback;
}

static int property_bool(const char* begin,const char* end,const char* tag,int fallback)
{
    char text[32];if(!tag_text(begin,end,tag,text,sizeof(text)))return fallback;return strcmp(text,"true")==0?1:strcmp(text,"false")==0?0:fallback;
}

static void property_vec3(const char* begin,const char* end,const char* property,float* x,float* y,float* z)
{
    char section[4096]; const char* p; const char* q; char tag[32];
    *x=*y=*z=0.0f;
    snprintf(tag,sizeof(tag),"<Vector3 name=\"%s\"",property);p=strstr(begin,tag);if(!p||p>=end)return;
    q=strstr(p,"</Vector3>");if(!q||q>=end)return;if((size_t)(q-p)>=sizeof(section))return;memcpy(section,p,(size_t)(q-p));section[q-p]='\0';
    *x=property_float(section,section+strlen(section),"X",0);*y=property_float(section,section+strlen(section),"Y",0);*z=property_float(section,section+strlen(section),"Z",0);
}

static uint32_t property_color(const char* begin,const char* end)
{
    const char* p=strstr(begin,"<Color3uint8 name=\"Color\"");char text[32];float r,g,b;
    if(p&&p<end){const char* q=strstr(p,"</Color3uint8>");if(q&&q<end){r=property_float(p,q,"R",128);g=property_float(p,q,"G",128);b=property_float(p,q,"B",128);return ((uint32_t)r<<24)|((uint32_t)g<<16)|((uint32_t)b<<8)|255u;}}
    if(tag_text(begin,end,"BrickColor",text,sizeof(text)))return 0x808080ffu;
    return 0x808080ffu;
}

nds_result nds_rbxlx_load(const char* path, nds_instance** out_root)
{
    char* data; size_t size; const char* p; const char* end; nds_instance* root;
    if(!path||!out_root)return NDS_ERR_INVALID_ARG;*out_root=NULL;
    data=read_file(path,&size);if(!data)return NDS_ERR_IO;
    if(strncmp(data,"<roblox",7)!=0){free(data);return NDS_ERR_IO;}
    root=nds_instance_create(NDS_CLASS_DATAMODEL,"ImportedPlace");if(!root){free(data);return NDS_ERR_UNKNOWN;}
    p=data;end=data+size;
    while((p=strstr(p,"<Item"))!=NULL && p<end){
        const char* item_end;char class_name[64];char name[256];nds_instance* object;nds_part_properties props;float x,y,z,sx,sy,sz,rx,ry,rz;
        if(p>data&&p[-1]=='/') {p+=5;continue;} item_end=find_item_end(p,end);if(!item_end)break;
        if(!attr_value(p,item_end,"class",class_name,sizeof(class_name))){p=item_end;continue;}
        if(strcmp(class_name,"Part")!=0&&strcmp(class_name,"SpawnLocation")!=0&&strcmp(class_name,"WedgePart")!=0&&strcmp(class_name,"CornerWedgePart")!=0&&strcmp(class_name,"TrussPart")!=0&&strcmp(class_name,"MeshPart")!=0){p=item_end;continue;}
        if(!attr_value(p,item_end,"referent",name,sizeof(name)))snprintf(name,sizeof(name),"ImportedPart");
        /* The actual Name property is inside <Properties>, not the Item tag. */
        {char property_name[256];if(tag_text(p,item_end,"string name=\"Name\"",property_name,sizeof(property_name)))snprintf(name,sizeof(name),"%s",property_name);}
        object=nds_instance_create(strcmp(class_name,"SpawnLocation")==0?NDS_CLASS_SPAWN_POINT:NDS_CLASS_PART,name);if(!object){nds_instance_destroy(root);free(data);return NDS_ERR_UNKNOWN;}
        if(nds_part_get_properties(object,&props)!=NDS_OK){nds_instance_destroy(object);p=item_end;continue;}
        property_vec3(p,item_end,"Position",&x,&y,&z);property_vec3(p,item_end,"Size",&sx,&sy,&sz);property_vec3(p,item_end,"Rotation",&rx,&ry,&rz);
        props.position=(nds_vec3){x,y,z};props.size=(nds_vec3){sx>0?sx:1,sy>0?sy:1,sz>0?sz:1};props.rotation=(nds_vec3){rx,ry,rz};props.color_rgba=property_color(p,item_end);props.transparency=property_float(p,item_end,"float name=\"Transparency\"",0);props.anchored=property_bool(p,item_end,"bool name=\"Anchored\"",1);props.can_collide=property_bool(p,item_end,"bool name=\"CanCollide\"",1);
        if(nds_part_set_properties(object,&props)!=NDS_OK||nds_instance_set_parent(object,root)!=NDS_OK){nds_instance_destroy(object);nds_instance_destroy(root);free(data);return NDS_ERR_IO;}
        p=item_end;
    }
    free(data);*out_root=root;return NDS_OK;
}
