//
// triggers.h
//

#pragma once

#define	TriggerSides	Special[5]


		/* TRIGGER TYPES */
enum
{
	TRIGTYPE_POW,
	TRIGTYPE_INVISIBILITY,
	TRIGTYPE_TOKEN,
	TRIGTYPE_TRACTION,
	TRIGTYPE_SUSPENSION,
	TRIGTYPE_CACTUS,
	TRIGTYPE_SNOMAN,
	TRIGTYPE_CAMPFIRE,
	TRIGTYPE_TEAMTORCH,
	TRIGTYPE_TEAMBASE,
	TRIGTYPE_VASE,
	TRIGTYPE_CAULDRON,
	TRIGTYPE_GONG,
	TRIGTYPE_LANDMINE,
	TRIGTYPE_SEAMINE,
	TRIGTYPE_LAVA,
	TRIGTYPE_DRUID
};



		/* POWERUP / WEAPON TYPES */
enum
{
	POW_TYPE_NONE = -1,
	POW_TYPE_BONE = 0,
	POW_TYPE_OIL = 1,
	POW_TYPE_NITRO = 2,
	POW_TYPE_BIRDBOMB = 3,
	POW_TYPE_ROMANCANDLE = 4,
	POW_TYPE_BOTTLEROCKET = 5,
	POW_TYPE_TORPEDO = 6,
	POW_TYPE_FREEZE = 7,
	POW_TYPE_MINE = 8,
    POW_TYPE_ZAPPER = 9, // implemented using token map item in non-tournament mode
	POW_TYPE_WHIRLWIND = 10, // not implemented yet (the desert spinning wind)
	POW_TYPE_BEAM = 11, // not implemented yet (the statue zappy thing beam)
	POW_TYPE_INVISIBILITY = 12, // invisibility has no respawnable form so here it is
	POW_TYPE_SUPER_SUSPENSION = 13, // super suspension has no respawnable form so here it is
	POW_TYPE_STICKY_TIRES = 14, // sticky tires has no respawnable form so here it is
	POW_TYPE_CUSTOM = 15, // unused slot

	MAX_POW_TYPES
};


#define	TorchTeam		Flag[0]


//===============================================================================

extern	Boolean HandleTrigger(ObjNode *triggerNode, ObjNode *whoNode, Byte side);
extern	Boolean AddPOW(TerrainItemEntryType *itemPtr, long  x, long z);

Boolean AddToken(TerrainItemEntryType *itemPtr, long  x, long z);

Boolean AddStickyTiresPOW(TerrainItemEntryType *itemPtr, long  x, long z);

Boolean AddSuspensionPOW(TerrainItemEntryType *itemPtr, long  x, long z);

Boolean AddCactus(TerrainItemEntryType *itemPtr, long  x, long z);

Boolean AddSnoMan(TerrainItemEntryType *itemPtr, long  x, long z);

Boolean AddCampFire(TerrainItemEntryType *itemPtr, long  x, long z);

Boolean AddTeamTorch(TerrainItemEntryType *itemPtr, long  x, long z);
Boolean AddTeamBase(TerrainItemEntryType *itemPtr, long  x, long z);
void PlayerDropFlag(ObjNode *theCar);

Boolean AddInvisibilityPOW(TerrainItemEntryType *itemPtr, long  x, long z);

Boolean AddVase(TerrainItemEntryType *itemPtr, long  x, long z);

Boolean AddCauldron(TerrainItemEntryType *itemPtr, long  x, long z);
Boolean AddGong(TerrainItemEntryType *itemPtr, long  x, long z);
Boolean AddSeaMine(TerrainItemEntryType *itemPtr, long  x, long z);
Boolean AddDruid(TerrainItemEntryType *itemPtr, long  x, long z);
