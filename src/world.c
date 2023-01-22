#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_types.h"
#include "gfc_config.h"

#include "gf3d_lights.h"

#include "camera_entity.h"
#include "player.h"
#include "spawns.h"
#include "world.h"

#define PARKING_WIDTH 10
#define PARKING_DEPTH 10

static World *the_world = NULL;

CombatZone *combat_zone_parse(SJson *def)
{
    const char * str;
    CombatZone *combatZone = NULL;
    if (!def)return NULL;
    combatZone =  gfc_allocate_array(sizeof(CombatZone),1);
    if (!combatZone)return NULL;
    str = sj_object_get_value_as_string(def,"name");
    if (str)gfc_line_cpy(combatZone->name,str);
    str = sj_object_get_value_as_string(def,"backgroundMusic");
    if (str)gfc_line_cpy(combatZone->backgroundMusic,str);
    sj_object_get_value_as_bool(def,"draw_home",&combatZone->draw_home);
    sj_value_as_vector3d(sj_object_get_value(def,"center"),&combatZone->center);
    sj_value_as_vector3d(sj_object_get_value(def,"camera"),&combatZone->camera);
    sj_value_as_vector3d(sj_object_get_value(def,"a_side"),&combatZone->a_side);
    sj_value_as_vector3d(sj_object_get_value(def,"b_side"),&combatZone->b_side);
    return combatZone;
}

World *world_load(char *filename)
{
    Entity *ent;
    CombatZone *combatZone;
    Vector4D globalColor = {0};
    Vector3D globalDir  = {0};
    Vector3D position,rotation;
    SJson *json,*wjson,*list,*item;
    World *w = NULL;
    Vector3D skyScale = {1,1,1};
    int i,c;
    ModelMat *m;
    const char *modelName = NULL;
    const char *str;
    json = sj_load(filename);
    if (!json)
    {
        slog("failed to load json file (%s) for the world data",filename);
        return NULL;
    }
    w = gfc_allocate_array(sizeof(World),1);
    if (w == NULL)
    {
        slog("failed to allocate data for the world");
        sj_free(json);
        return NULL;
    }
    wjson = sj_object_get_value(json,"world");
    if (!wjson)
    {
        slog("failed to find world object in %s world condig",filename);
        free(w);
        sj_free(json);
        return NULL;
    }
    list = sj_object_get_value(wjson,"models");
    c = sj_array_get_count(list);
    w->model_list = gfc_list_new();
    for (i = 0; i < c; i++)
    {
        item = sj_array_get_nth(list,i);
        if (!item)continue;
        m = gf3d_model_mat_new();
        if (!m)continue;
        gf3d_model_mat_parse(m,item);
        gf3d_model_mat_set_matrix(m);
        gfc_list_append(w->model_list,m);
    }
    
    modelName = sj_get_string_value(sj_object_get_value(wjson,"sky"));
    if (!modelName)
    {
        slog("world data (%s) has no sky",filename);
    }
    else
    {
        w->sky = gf3d_model_load(modelName);
    }
    sj_value_as_vector3d(sj_object_get_value(wjson,"skyScale"),&skyScale);
    gfc_matrix_identity(w->skyMat);
    gfc_matrix_scale(w->skyMat,skyScale);

    modelName = sj_get_string_value(sj_object_get_value(wjson,"backgroundMusic"));
    if (modelName)
    {
        w->backgroundMusic = Mix_LoadMUS(modelName);
        if (!w->backgroundMusic)
        {
            slog("failed to open background music %s",modelName);
        }
        else
        {
            Mix_PlayMusic(w->backgroundMusic,-1);
        }
    }

    list = sj_object_get_value(wjson,"lights");
    c = sj_array_get_count(list);
    for (i = 0; i < c; i++)
    {
        item = sj_array_get_nth(list,i);
        if (!item)continue;
        str = sj_get_string_value(sj_object_get_value(item,"type"));
        if (!str)continue;
        vector3d_set(globalDir,0,0,0);
        sj_value_as_vector3d(sj_object_get_value(item,"direction"),&globalDir);
        vector4d_set(globalColor,0,0,0,0);
        sj_value_as_vector4d(sj_object_get_value(item,"color"),&globalColor);
        vector3d_set(position,0,0,0);
        sj_value_as_vector3d(sj_object_get_value(item,"position"),&position);
        if (strcmp(str,"global")==0)
        {
            gf3d_lights_set_global_light(globalColor,vector4d(globalDir.x,globalDir.y,globalDir.z,1));;
            w->theSun = gf3d_particle(vector3d(-10000,-10000,0), gfc_color_from_vector4f(globalColor), 10000);
        }
    }
    item = sj_object_get_value(wjson,"camera");
    if (item)
    {
        sj_value_as_vector3d(sj_object_get_value(item,"position"),&position);
        vector3d_copy(w->cameraPosition,position);
        sj_value_as_vector3d(sj_object_get_value(item,"rotation"),&rotation);
        camera_entity_new(position,rotation);
    }
    item = sj_object_get_value(wjson,"sounds");
    if (item)
    {
        w->sounds = gfc_sound_pack_parse(item);
    }
    sj_object_get_value_as_uint32(wjson,"hourTime",&w->hourTime);
    
    sj_value_as_vector3d(sj_object_get_value(wjson,"gateApproach"),&w->gateApproach);
    sj_value_as_vector3d(sj_object_get_value(wjson,"gateEgress"),&w->gateEgress);

    
    w->entity_list = gfc_list_new();
    list = sj_object_get_value(wjson,"entities");
    c = sj_array_get_count(list);
    for (i = 0; i < c; i++)
    {
        item = sj_array_get_nth(list,i);
        if (!item)continue;
        ent = spawn_call(item);
        if (!ent)continue;
        gfc_list_append(w->entity_list,ent);
    }
    w->parking_spots = gfc_list_new();
    item = sj_object_get_value(wjson,"parking");
    if (item)
    {
        sj_value_as_vector3d(sj_object_get_value(item,"position"),&w->parkingStart);
        sj_value_as_vector3d(sj_object_get_value(item,"delta"),&w->parkingDelta);
        sj_value_as_vector3d(sj_object_get_value(item,"approach"),&w->parkingApproach);
        sj_value_as_vector3d(sj_object_get_value(item,"egress"),&w->parkingEgress);
    }
    
    list = sj_object_get_value(wjson,"combat_zones");
    if (list)
    {
        w->combatZones = gfc_list_new();
        c = sj_array_get_count(list);
        for (i = 0;i < c;i++)
        {
            item = sj_array_get_nth(list,i);
            if (!item)continue;
            combatZone = combat_zone_parse(item);
            if (!combatZone)continue;
            gfc_list_append(w->combatZones,combatZone);
        }
    }
    sj_free(json);
    the_world = w;
    return w;
}

CombatZone *world_get_combat_zone_by_name(const char *name)
{
    CombatZone *combatZone = NULL;
    int i,c;
    if (!the_world)return NULL;
    if (!name)return NULL;
    c = gfc_list_get_count(the_world->combatZones);
    for (i = 0;i < c; i++)
    {
        combatZone = gfc_list_get_nth(the_world->combatZones,i);
        if (!combatZone)continue;
        if (gfc_line_cmp(name,combatZone->name)==0)return combatZone;
    }
    return NULL;
}

void get_date_of(char *output,Uint32 day)
{
    if (!output)return;
    gfc_line_sprintf(output,"%02i/%02i/%04i",(day%360)%30+1,(day%360)/30+1,2280 + (day / 360));
}

void get_date(char *output)
{
    int day;
    if (!output)return;
    day = player_get_day();
    get_date_of(output,day);
}

void get_datetime_of(char *output,Uint32 day,Uint32 hour)
{
    if (!output)return;
    gfc_line_sprintf(output,"%02i/%02i/%04i %02i:00 Day: %i",(day%360)%30+1,(day%360)/30+1,2280 + (day / 360),hour,day);
}

void get_datetime(char *output)
{
    int day,hour;
    if (!output)return;
    day = player_get_day();
    hour = player_get_hour();
    get_datetime_of(output,day,hour);
}

ModelMat *world_get_model_mat(World *world,Uint32 index)
{
    if (!world)return NULL;
    return gfc_list_get_nth(world->model_list,index);
}

void world_draw(World *world)
{
    int i,c;
    ModelMat *m;
    if (!world)return;
    gf3d_model_draw_sky(world->sky,world->skyMat,gfc_color(1,1,1,1));
    c = gfc_list_get_count(world->model_list);
    for (i = 0; i < c; i++)
    {
        m = gfc_list_get_nth(world->model_list,i);
        if (!m)continue;
        gf3d_model_draw(m->model,0,m->mat,vector4d(1,.9,.5,1),vector4d(1,1,1,1),vector4d(1,1,1,0));
    }
    gf3d_particle_draw(world->theSun);
}

void world_delete(World *world)
{
    int i,c;
    ModelMat *m;
    Entity *entity;
    if (!world)return;
    c = gfc_list_get_count(world->model_list);
    for (i = 0; i < c; i++)
    {
        m = gfc_list_get_nth(world->model_list,i);
        if (!m)continue;
        gf3d_model_mat_free(m);
    }
    gfc_list_delete(world->model_list);
    c = gfc_list_get_count(world->entity_list);
    for (i = 0; i < c; i++)
    {
        entity = gfc_list_get_nth(world->entity_list,i);
        if (!entity)continue;
        gf3d_entity_free(entity);
    }
    gfc_list_delete(world->entity_list);
    gfc_list_foreach(world->combatZones,free);
    if (world->combatZones)gfc_list_delete(world->combatZones);
    gfc_sound_pack_free(world->sounds);
    free(world);
    the_world = NULL;
}

void world_run_updates(World *w)
{
    Uint32 now;
    if (!w)return;
    now = SDL_GetTicks();
    if (now > (w->lastHour + w->hourTime))
    {
        w->lastHour = now;
        player_hour_advance();
    }
}

Vector3D world_get_parking_approach()
{
    if (!the_world)return vector3d(0,0,0);
    return the_world->parkingApproach;
}

Vector3D world_get_parking_egress()
{
    if (!the_world)return vector3d(0,0,0);
    return the_world->parkingEgress;
}

Vector3D world_get_gate_approach()
{
    if (!the_world)return vector3d(0,0,0);
    return the_world->gateApproach;
}

Vector3D world_get_gate_egress()
{
    if (!the_world)return vector3d(0,0,0);
    return the_world->gateEgress;
}

void world_play_sound(const char *sound)
{
    if ((!the_world)||(!sound))return;
    gfc_sound_pack_play(the_world->sounds, sound,0,1,-1,-1);
}

Vector3D world_parking_claim_spot(Vector3D spot)
{
    Vector3D *location;
    if (!the_world)
    {
        slog("no world");
        return vector3d(-1,-1,-1);
    }
    if (!world_parking_spot_get_by_location(spot))
    {
        location = gfc_allocate_array(sizeof(Vector3D),1);
        location->x = spot.x;
        location->y = spot.y;
        location->z = spot.z;
        gfc_list_append(the_world->parking_spots,location);
        return spot;
    }
    // spot is already taken, find another one
    return world_parking_get_spot();
}

Vector3D *world_parking_spot_get_by_location(Vector3D spot)
{
    int i,c;
    Vector3D *location;
    if (!the_world)return NULL;
    c = gfc_list_get_count(the_world->parking_spots);
    for (i = 0; i < c; i++)
    {
        location = gfc_list_get_nth(the_world->parking_spots,i);
        if (!location)continue;
        if ((location->x == spot.x)&&
            (location->y == spot.y)&&
            (location->z == spot.z))
        {
            return location;
        }
    }
    return NULL;
}

Vector3D world_parking_get_spot()
{
    int i,j;
    Vector3D spot,*location;
    if (!the_world)return vector3d(-1,-1,-1);
    vector3d_copy(spot,the_world->parkingStart);
    for (j = 0; j < PARKING_DEPTH;j++)
    {
        vector3d_copy(spot,the_world->parkingStart);
        spot.x += the_world->parkingDelta.x * j;
        for (i = 0; i < PARKING_WIDTH;i++)
        {
            spot.y = the_world->parkingStart.y + (the_world->parkingDelta.y * i);
            if (!world_parking_spot_get_by_location(spot))
            {
                //this location is in use
                location = gfc_allocate_array(sizeof(Vector3D),1);
                vector3d_copy((*location),spot);
                gfc_list_append(the_world->parking_spots,location);
                return spot;
            }
        }
    }
    return vector3d(-1,-1,-1);
}

void world_parking_vacate_spot(Vector3D spot)
{
    Vector3D *location;
    if (!the_world)return;
    location = world_parking_spot_get_by_location(spot);
    if (!location)return;
    gfc_list_delete_data(the_world->parking_spots,location);
}

/*eol@eof*/
