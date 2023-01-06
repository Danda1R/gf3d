#ifndef __STATION_FACILITY_H__
#define __STATION_FACILITY_H__

#include "simple_json.h"

#include "gfc_text.h"
#include "gfc_list.h"

#include "gf3d_model.h"
#include "mission.h"

typedef struct
{
    TextLine    name;           //its name identifier
    TextLine    displayName;    //its name as displayed to the user (including ID number)
    Uint32      id;
    TextLine    facilityType;  
    Vector2D    position;       //for planet side facilities
    ModelMat    mat;
    float       damage;         //keeps track of damage.  Damaged facilities lave lower output  0 is no damage, 100 is destroyed anything else can be repaired.
    Bool        working;      // if true, the facility is working
    Mission    *mission;        // if a mission has been assigned to this facility
    float       productivity;   //how efficient a facility is at running.  due to factors like damage, energy, and staffing
    Uint32      lastProduction; //last time this facility produced
    int         operatingCost;  //cost of running due to staff and office wages
    int         income;         //if this facility directly produces income, this is the amount
    int         housing;        //how much housing is provided by this facility
    float       crimeRate;      //how much crime is generated as a byproduct of this facility
    float       opportunities;  //how many business and employment opportunities are provided by the facility
    float       commerce;       //how much taxable commerce is conducted by the facility
    float       entertainment;  //how much public entertainment is provided by the facility
    int         staffRequired;  //how many people are needed to run the facility at a minimum
    int         staffAssigned;  //how many people are actually hired to do so
    int         staffPositions; //how many people are CAN be hired to do so
    int         energyDraw;     //how much energy is needed for the facility to run
    int         energyOutput;   //how much energy is produced by the facility
    int         inactive;       //if the facility cannot run
    int         disabled;       //if the player has chosen to turn it off
    int         storage;        // how many commodities can be stored here
    TextLine    officer;        //if there is an officer assigned to help run this facility
    List        *upkeep;        //what resources are needed to keep this running every update
    List        *produces;      //what this produces every update cycle
}StationFacility;

/**
 * @brief allocate and initialize a new station facility
 * @return NULL on error or the station facility
 */
StationFacility *station_facility_new();

/**
 * @brief draw the facility
 * @param facility it
 */
void station_facility_draw(StationFacility *facility);
void station_facility_draw_highlight(StationFacility *facility);

/**
 * @brief remove a facility from its location and delete it
 * @param facility the facility to remove
 */
void station_facility_remove(StationFacility *facility);

/**
 * @brief create a new default facility based on its definition name.
 * @param name the name of the facility in question
 * @param id if -1, it will be given a new id automatically, otherwise it will be given the one provided
 * @return NULL if not found or other error, the newly created facility otherwise
 */
StationFacility *station_facility_new_by_name(const char *name,int id);

/**
 * @brief build a new facility and set up its construction mission
 * @param name the name of the facility to build
 * @param position if its planetary, where on the planet
 * @param parentList the station section or planet facility list to add this to
 * @param staff the staff assigned to build it
 */
void station_facility_build(const char *name,Vector2D position,List *parentList,Uint32 staff);

/**
 * @brief load a station facility based on config
 * @param config the config containing the data
 * @return NULL on error or the facility loaded
 */
StationFacility *station_facility_load(SJson *config);

/**
 * @brief get a facility from a list of facilities by its position
 */
StationFacility *station_facility_get_by_position(List *list,Vector2D position);

/**
 * @brief free from memory a station facility
 * @param facility the facility to free
 */
void station_facility_free(StationFacility *facility);

/**
 * @brief save a station facility to config
 * @param facility the facility to write to config
 * @return NULL on error a json object containing facility data
 */
SJson *station_facility_save(StationFacility *facility);

/**
 * @brief free a list of facilities
 * @param list a list containing pointers to facilities
 */
void station_facility_free_list(List *list);

/**
 * @brief get the display name, given a facility's name
 * @param name the name id
 * @return NULL if not found, the name otherwise
 */
const char *station_facility_get_display_name(const char *name);

/**
 * @brief check if a facility supports a directing officer
 * @param name the name of the facility
 * @return 0 if not, 1 if it does
 */
int station_facility_supports_officer(const char *name);

/**
 * @brief get the id name, given a facility's display name
 * @param name the name id
 * @return NULL if not found, the name otherwise
 */
const char *station_facility_get_name_from_display(const char *display);

/**
 * @brief figure out how long it will take for the the facility to complete its work
 */
Uint32 station_facility_get_work_time(const char *name);

/**
 * @brief figure out how long it will take for the the facility to complete building
 */
Uint32 station_facility_get_build_time(const char *name);

/**
 * @brief run the upkeep on the facility
 * @param facility the facility
 * @param energySupply how much energy the station has left
 */
void station_facility_update(StationFacility *facility,float *energySupply);

/**
 * @brief run checks to see if the facility can be enabled
 * @note does not evaluate energy requirements
 * @param facility the facility to check
 */
void station_facility_check(StationFacility *facility);

/**
 * @brief check if a station facility is a unique one (required and only one)
 * @note unique ones cannot be bought or sold
 * @param facility the facility to check
 * @return 0 if not or error, 1 if it is unique
 */
int station_facility_is_unique(StationFacility *facility);

/**
 * @brief check if a station facility is a singleton (only allowed to have one copy of it)
 * @note singletons can be sold or purchased, but you can only have one at a time
 * @param name the name of the facility
 * @return 0 if not or error, 1 if it is a singleton
 */
int station_facility_is_singleton(const char *name);

/**
 * @brief Assign / Remove staff from a facility
 * @param facility the facility
 * @param amount how many to hire / fire (you should probably do this one at a time)
 * @return 0 if okay or not facility, 1 if there were too many to hire, -1 too many to fire
 */
int station_facility_change_staff(StationFacility *facility,int amount);

/**
 * @brief get the list of resources list for the costs associated with the facility
 * @param name the name of the facility to check
 * @param resource_type "cost" "upkeep" or "provides" all else will return NULL
 * @return a list of resources if the facility has costs associated with the resource type.  NULL on any error
 */
List *station_facility_get_resource_cost(const char *name,const char *resource_type);

/**
 * @brief get a list of facilities with the types from the list provided
 * @param list a list of strings to filter by
 * @return NULL if error, or a list of strings of facility names
 */
List *station_facility_get_possible_from_list(List *list);

/**
 * @brief set a facility to repaired
 * @param facility the facility to repair
 */
void station_facility_repair(StationFacility *facility);

/**
 * @brief search a list of facilities for the first one matching the name
 * @param facilityList the list of facilities
 * @param name the name to search for
 * @return NULL on error or not found.  The facility otherwise
 */
StationFacility *station_facility_get_by_name(List *facilityList, const char *name);

/**
 * @brief search a list of facilities for the one matching the name and id
 * @param facilityList the list of facilities
 * @param name the name to search for
 * @param id with this id
 * @return NULL on error or not found.  The facility otherwise
 */
StationFacility *station_facility_get_by_name_id(List *facilityList, const char *name,Uint32 id);


#endif
