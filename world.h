#ifndef WORLD_H
#define WORLD_H
#include "core.h"

typedef struct world {
	char*  name;
	ushort dimensions[3];
	VECTOR spawnVec;
	ANGLE  spawnAng;
	uint   size;
	char*  data;
} WORLD;

WORLD* World_Create(char* name, ushort dx, ushort dy, ushort dz);
uint World_GetOffset(WORLD* world, ushort x, ushort y, ushort z);
int World_SetBlock(WORLD* world, ushort x, ushort y, ushort z, int id);
int World_GetBlock(WORLD* world, ushort x, ushort y, ushort z);
void World_GenerateFlat(WORLD* world);
int World_Load(WORLD* world);
int World_Save(WORLD* world);
#endif