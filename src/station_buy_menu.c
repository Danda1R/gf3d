#include <stdio.h>

#include "simple_logger.h"

#include "gfc_types.h"
#include "gfc_input.h"

#include "gf2d_windows.h"
#include "gf2d_elements.h"
#include "gf2d_element_list.h"
#include "gf2d_element_label.h"
#include "gf2d_element_button.h"
#include "gf2d_draw.h"
#include "gf2d_mouse.h"
#include "gf2d_windows_common.h"
#include "gf2d_message_buffer.h"

#include "station_def.h"
#include "station.h"
#include "station_buy_menu.h"

typedef struct
{
    StationSection *parent;
    List *list;
}StationBuyMenuData;

int station_buy_menu_free(Window *win)
{
    StationBuyMenuData *data;
    if (!win)return 0;
    if (!win->data)return 0;
    data = win->data;
    gf2d_window_close_child(win->parent,win);
    free(data);
    return 0;
}

int station_buy_menu_draw(Window *win)
{
//    StationBuyMenuData *data;
    if (!win)return 0;
    if (!win->data)return 0;
//    data = win->data;
    return 0;
}

int station_buy_menu_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
    StationBuyMenuData* data;
    if (!win)return 0;
    if (!updateList)return 0;
    data = (StationBuyMenuData*)win->data;
    if (!data)return 0;
        
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (e->index >= 1000)
        {
            message_printf("selecting item %s",e->name);
            return 0;
        }
    }
    if (gfc_input_command_released("cancel"))
    {
            gf2d_window_free(win);
            return 1;
    }
    if ((gf2d_mouse_button_state(0))||(gf2d_mouse_button_state(1)))
    {
        if (!gf2d_window_mouse_in(win))
        {
            gf2d_window_free(win);
            return 1;
        }
    }
    return gf2d_window_mouse_in(win);
}

void station_buy_menu_set_list(Window *win)
{
    const char *str;
    Element *button;
    Element *item_list;
    int i,c;
    StationBuyMenuData *data;
    if (!win)return;
    if (!win->data)return;
    data = win->data;
    c = gfc_list_get_count(data->list);
    item_list = gf2d_window_get_element_by_name(win,"item_list");
    for (i = 0; i < c; i++)
    {
        str = gfc_list_get_nth(data->list,i);
        if (!str)continue;
        str = station_def_get_display_name(str);
        button = gf2d_button_new_label_simple(win,1000+1,str,gfc_color8(255,255,255,255));
        if (!button)continue;
        gf2d_element_list_add_item(item_list,button);
    }
}

Window *station_buy_menu(Window *parent,StationSection *parentSection,List *list)
{
    Window *win;
    StationBuyMenuData* data;
    win = gf2d_window_load("menus/section_buy.menu");
    if (!win)
    {
        slog("failed to load station buy menu");
        return NULL;
    }
    win->update = station_buy_menu_update;
    win->free_data = station_buy_menu_free;
    win->draw = station_buy_menu_draw;
    data = (StationBuyMenuData*)gfc_allocate_array(sizeof(StationBuyMenuData),1);
    win->data = data;
    win->parent = parent;
    data->list = list;
    station_buy_menu_set_list(win);
    return win;
}


/*eol@eof*/
