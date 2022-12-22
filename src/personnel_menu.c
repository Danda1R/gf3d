#include "simple_logger.h"

#include "gfc_list.h"
#include "gfc_input.h"
#include "gfc_callbacks.h"

#include "gf3d_camera.h"

#include "gf2d_mouse.h"
#include "gf2d_elements.h"
#include "gf2d_element_list.h"
#include "gf2d_element_label.h"
#include "gf2d_element_entry.h"
#include "gf2d_item_list_menu.h"
#include "gf2d_message_buffer.h"
#include "gf2d_windows_common.h"

#include "entity.h"
#include "camera_entity.h"
#include "resources.h"
#include "station_def.h"
#include "station.h"
#include "player.h"
#include "station_extension_menu.h"
#include "station_buy_menu.h"
#include "facility_menu.h"
#include "personnel_menu.h"

typedef struct
{
    int updated;
}PersonnelMenuData;

void personnel_menu_update_resources(Window *win);

int personnel_menu_free(Window *win)
{
    PersonnelMenuData *data;
    if (!win)return 0;
    gf2d_window_close_child(win->parent,win);
    if (!win->data)return 0;
    data = win->data;
    free(data);
    return 0;
}

int personnel_menu_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
    PersonnelMenuData *data;
    if (!win)return 0;
    if (!updateList)return 0;
    data = (PersonnelMenuData*)win->data;

    if (data->updated != player_get_day())
    {
        personnel_menu_update_resources(win);
    }

    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
    }
    if (gfc_input_command_released("cancel"))
    {
        gf2d_window_free(win);
        return 1;
    }
    return 0;
}


int personnel_menu_draw(Window *win)
{
//    PersonnelMenuData *data;
    if ((!win)||(!win->data))return 0;
//    data = win->data;
    return 0;
}

void personnel_menu_update_resources(Window *win)
{
    TextLine buffer;
    PlayerData *player;
    StationData *station;
    PersonnelMenuData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    player = player_get_data();
    station = player_get_station_data();
    if ((!player)||(!station))return;

    gfc_line_sprintf(buffer,"Population : %i",player->population);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"population"),buffer);

    gfc_line_sprintf(buffer,"Housing : %i",station->housing);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"housing"),buffer);

    gfc_line_sprintf(buffer,"Total Staff : %i",player->staff + station->staffAssigned);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"staff_total"),buffer);

    gfc_line_sprintf(buffer,"Staff Unassigned : %i",player->staff);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"staff"),buffer);
    gfc_line_sprintf(buffer,"Assigned : %i",station->staffAssigned);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"assigned"),buffer);
        
    data->updated = player_get_day();
}

Window *personnel_menu(Window *parent)
{
    Window *win;
    PersonnelMenuData *data;
    win = gf2d_window_load("menus/personnel.menu");
    if (!win)
    {
        slog("failed to load personnel window");
        return NULL;
    }
    data = gfc_allocate_array(sizeof(PersonnelMenuData),1);
    if (!data)
    {
        gf2d_window_free(win);
        return NULL;
    }
    win->parent = parent;
    win->data = data;
    win->update = personnel_menu_update;
    win->free_data = personnel_menu_free;
    win->draw = personnel_menu_draw;
    personnel_menu_update_resources(win);
    message_buffer_bubble();
    return win;
}
