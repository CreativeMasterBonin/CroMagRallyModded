/****************************/
/*   	PATH ITEMS.C      */
/****************************/


/***************/
/* EXTERNALS   */
/***************/

#include "game.h"


/****************************/
/*    PROTOTYPES            */
/****************************/


/****************************/
/*    CONSTANTS             */
/****************************/



#define	PATH_SCAN_RANGE		(TERRAIN_SUPERTILE_UNIT_SIZE * 1.0f)

/**********************/
/*     VARIABLES      */
/**********************/

long			gNumPaths = 0;
PathDefType	    **gPathList = nil;

SuperTilePathGridType	**gSuperTilePathGrid = nil;

// constants containing predefined positions for tracks that have no map path points
// no y coords, as that is determined by vehicle and terrain conditions
// DO NOT USE THESE: they are completely pointless as cpu pathing does not work like this
static const float stoneHengeXPoints[] = {102610,93757,77103,56174,39925,26889,29850,49320,59200,63775,86503,93769,98560};
static const float stoneHengeZPoints[] = {63213,77214,88467,91295,71705,48099,31672,32800,40050,48645,53133,62966,72794};
static const Byte stoneHengePointFlags[] = {PATH_FLAGS_PRIMARY,nil,nil,nil,nil,nil,nil,nil,nil,nil,nil,nil,PATH_FLAGS_YCLOSE};

static float GetPointsForTrack(short trackNum,bool getX,int selectedPointIndex){
	if(getX){
		switch(trackNum){
			case TRACK_NUM_STONEHENGE:{
				if(selectedPointIndex > 13 || selectedPointIndex < 0){
					selectedPointIndex = 0;
				}
				return stoneHengeXPoints[selectedPointIndex];
			}
			default:{
				return 0.0f;
			}
		}
	}
	else{
		switch(trackNum){
			case TRACK_NUM_STONEHENGE:{
				if(selectedPointIndex > 13 || selectedPointIndex < 0){
					selectedPointIndex = 0;
				}
				return stoneHengeZPoints[selectedPointIndex];
			}
			default:{
				return 0.0f;
			}
		}
	}
}


/********************** ASSIGN PATH VECTORS TO SUPERTILE GRID **************************/
//
// The path line segments are assigned to a list maintained by each supertile grid slot.
// That way it is very easy to determine which path vectors are associated for
// any given supertile.
//
// Once we have created these new lists, we can dispose of the terrain file's path
// data since it isnt needed anymore.
//
// The supertiles dont need to have been created yet since this allocates a separate array.
//

void AssignPathVectorsToSuperTileGrid(void)
{
int     		i,row,col,n;
PathPointType   *pointList;
int             j,p;

			/* ALLOC MEMORY FOR PATH GRID */

	Alloc_2d_array(SuperTilePathGridType, gSuperTilePathGrid, gNumSuperTilesDeep, gNumSuperTilesWide);


        /* INITIALIZE EACH GRID LOCATION - NO PATHS */

    for (row = 0; row < gNumSuperTilesDeep; row++)
    {
        for (col = 0; col < gNumSuperTilesWide; col++)
        {
            gSuperTilePathGrid[row][col].numVectors = 0;
            gSuperTilePathGrid[row][col].vectors    = nil;
        }
    }


			/**************************************/
			/* CONVERT PATH POINTS TO UNIT COORDS */
			/**************************************/
	
    for (i = 0; i < gNumPaths; i++)
    {
		n = (*gPathList)[i].numPoints;                          // get # points in the list
		pointList = *(*gPathList)[i].pointList;                 // dereference the handle (its locked, so a ptr is okay)
		for (p = 0; p < n; p++)
		{
			pointList[p].x *= MAP2UNIT_VALUE;
			pointList[p].z *= MAP2UNIT_VALUE;
		}
	}



        /****************************************************/
        /* SCAN THE PATH LISTS AND ADD PATH VECTORS TO GRID */
        /****************************************************/
	
    for (i = 0; i < gNumPaths; i++)
    {

            /* POINT TO THIS PATH'S POINT LIST */
		n = (*gPathList)[i].numPoints;                          // get # points in the list
		if (n == 1)												// skip 1 point paths since they are garbage
			continue;
		pointList = *(*gPathList)[i].pointList;                 // dereference the handle (its locked, so a ptr is okay)


			/***********************************************************/
	 		/* CREATE VECTORS FROM POINTS AND ASSIGN TO SUPERTILE GRID */
			/***********************************************************/

        for (p = 0; p < n; p++)
        {
			OGLVector2D		v = {0, 0};
			float			x,z;

			x 	= pointList[p].x;									// get point coord
			z 	= pointList[p].z;
			
			//printf("x: %.2f | z: %.2f \n",x,z);

        	if (p < (n-1))											// for last point, keep previous vector (since theres no next point to create a new vector)
        	{
        		v.x	= pointList[p+1].x - x;							// calc vector
        		v.y	= pointList[p+1].z - z;
		        FastNormalizeVector2D(v.x, v.y, &v, true);			// normalize the vector
		 	}

			col = x / TERRAIN_SUPERTILE_UNIT_SIZE;					// convert to supertile row,col
			row = z / TERRAIN_SUPERTILE_UNIT_SIZE;
			
			//printf("SUPERTILE row: %u | col: %u \n",row,col);
			
			/*if(gDebugMode > 1){
				printf("SUPERTILE CONVERTED COL AND ROW: col: %u row: %u\n",col,row);
			}*/

			if ((col < 0) || (col >= gNumSuperTilesWide))			// see if out of bounds
				DoFatalAlert("AssignPathVectorsToSuperTileGrid: col out of bounds");
			if ((row < 0) || (row >= gNumSuperTilesDeep))
				DoFatalAlert("AssignPathVectorsToSuperTileGrid: row out of bounds");

					/* SEE IF NEED TO ALLOC MEMORY FOR THE VECTOR LIST */

			if (gSuperTilePathGrid[row][col].vectors == nil)
			{
				gSuperTilePathGrid[row][col].vectors = (PathVectorType *)AllocPtr(sizeof(PathVectorType) * MAX_VECTORS_PER_SUPERTILE);

				if (gSuperTilePathGrid[row][col].vectors == nil)
					DoFatalAlert("AssignPathVectorsToSuperTileGrid: AllocPtr failed");
			}

					/* ADD THIS VECTOR TO THE LIST */

			j = gSuperTilePathGrid[row][col].numVectors;		// get # already in list
			if (j >= MAX_VECTORS_PER_SUPERTILE)					// see if too many
				DoFatalAlert("AssignPathVectorsToSuperTileGrid: numVectors >= MAX_VECTORS_PER_SUPERTILE");

			gSuperTilePathGrid[row][col].vectors[j].vx = v.x;	// copy coord info into list (normalized vector & origin coord)
			gSuperTilePathGrid[row][col].vectors[j].vz = v.y;
			gSuperTilePathGrid[row][col].vectors[j].ox = x;
			gSuperTilePathGrid[row][col].vectors[j].oz = z;
			
			/*printf("Row: %u - Col: %u \n",row,col);
			printf("VX: %.2f \n",gSuperTilePathGrid[row][col].vectors[j].vx);
			printf("VZ: %.2f \n",gSuperTilePathGrid[row][col].vectors[j].vz);
			printf("OX: %.2f \n",gSuperTilePathGrid[row][col].vectors[j].ox);
			printf("OZ: %.2f \n",gSuperTilePathGrid[row][col].vectors[j].oz);&*/

			// this data is non-existant in battle/custom maps
			if(gTrackNum != TRACK_NUM_STONEHENGE){
				gSuperTilePathGrid[row][col].flags[j] = (*gPathList)[i].flags;
			}

			gSuperTilePathGrid[row][col].numVectors++;			// inc count
		}
	}


			/***************************/
			/* NUKE ORIGINAL PATH DATA */
			/***************************/
	// nuke only in tracks where actual path data is used
	if(gTrackNum != TRACK_NUM_STONEHENGE){
		for (i = 0; i < gNumPaths; i++)
		{
			DisposeHandle((Handle)(*gPathList)[i].pointList);		// nuke point list
		}
		DisposeHandle((Handle)gPathList);
	}
	else{ // not sure how safe this is since gNumPaths and gPathList never actually was used for its intended purpose in battle/custom maps
		pointList = nil;
		gPathList = nil;
		DisposeHandle((Handle)gPathList); // dispose of this still since it is used
	}
}



/********************** DISPOSE PATH GRID ***********************/

void DisposePathGrid(void)
{
int row,col;

    if (gSuperTilePathGrid == nil)
    	return;

			/* DELETE VECTOR LIST IN EACH GRID LOCATION */

    for (row = 0; row < gNumSuperTilesDeep; row++)
    {
        for (col = 0; col < gNumSuperTilesWide; col++)
        {
            if (gSuperTilePathGrid[row][col].vectors)           // see if there is a list to nuke
                SafeDisposePtr((Ptr)gSuperTilePathGrid[row][col].vectors);
        }
    }

			/* NUKE THE GRID ITSELF */

    Free_2d_array(gSuperTilePathGrid);
    gSuperTilePathGrid = nil;
}


#pragma mark -

Boolean FindClosestPointAndMakeVector(float x, float y, float z, OGLVector2D *outDec){
	float			dist;
	Byte			*flagData;
	
	int countValues = 0;
	bool gotVector = false;
	
	if(gTrackNum == TRACK_NUM_STONEHENGE){
		countValues = 13;
	}
	
	if(countValues <= 0){
		return(false);
	}
	
	Byte flags;
	
	if(gTrackNum == TRACK_NUM_STONEHENGE){
		for (int i = 0; i < countValues; i++){
			flagData = stoneHengePointFlags[i];
			outDec->x += stoneHengeXPoints[i];							// add in this vector
			outDec->y += stoneHengeZPoints[i];
			gotVector = true;
		}
	}
	if (gotVector){
		FastNormalizeVector2D(outDec->x, outDec->y, outDec, true);
		return(true);
	}
	return(false);
}


/************************** CALC PATH VECTOR FROM COORD ***************************/
//
// Given a world x,z coordinate, calculate a path vector based on all of the path information
// available for the nearby supertiles.
//
// Returns FALSE if no paths are found.
//

Boolean CalcPathVectorFromCoord(float x, float y, float z, OGLVector2D *outVec)
{
float			minX,maxX,minZ,maxZ,dist;
int				startCol,endCol,startRow,endRow;
int				row,col,i;
PathVectorType	*vecData;
Boolean			gotVector = false;
Byte			flags;
Byte			*flagData;

	minX = x - PATH_SCAN_RANGE;
	maxX = x + PATH_SCAN_RANGE;
	minZ = z - PATH_SCAN_RANGE;
	maxZ = z + PATH_SCAN_RANGE;

	startCol 	= minX * TERRAIN_SUPERTILE_UNIT_SIZE_Frac;
	endCol 		= maxX * TERRAIN_SUPERTILE_UNIT_SIZE_Frac;
	startRow 	= minZ * TERRAIN_SUPERTILE_UNIT_SIZE_Frac;
	endRow 		= maxZ * TERRAIN_SUPERTILE_UNIT_SIZE_Frac;

	if (startCol < 0)										// check bounds
		startCol = 0;
	else
	if (startCol >= gNumSuperTilesWide)
		startCol = gNumSuperTilesWide-1;

	if (endCol < 0)
		endCol = 0;
	else
	if (endCol >= gNumSuperTilesWide)
		endCol = gNumSuperTilesWide-1;

	if (startRow < 0)
		startRow = 0;
	else
	if (startRow >= gNumSuperTilesDeep)
		startRow = gNumSuperTilesDeep-1;

	if (endRow < 0)
		endRow = 0;
	else
	if (endRow >= gNumSuperTilesDeep)
		endRow = gNumSuperTilesDeep-1;



	outVec->x = outVec->y = 0;													// initialize the vector

	for (row = startRow; row <= endRow; row++)
	{
		for (col = startCol; col <= endCol; col++)
		{
			vecData = gSuperTilePathGrid[row][col].vectors;						// get ptr to vector list
			flagData = gSuperTilePathGrid[row][col].flags;
			if (vecData)
			{
						/**************************************/
						/* SCAN EACH VECTOR ON THIS SUPERTILE */
						/**************************************/
				
				for (i = 0; i < gSuperTilePathGrid[row][col].numVectors; i++)
				{
						/* SEE IF VECTOR ORIGIN IN WITHIN OUR RANGE */

					if ((vecData[i].ox >= minX) && (vecData[i].ox <= maxX) &&
						(vecData[i].oz >= minZ) && (vecData[i].oz <= maxZ))
					{
						flags = flagData[i];									// get path flags

						if (flags & PATH_FLAGS_YCLOSE)							// see if must be close in y to use this
						{
							if (fabs(GetTerrainY(x,z) - y) > 50.0f)
								continue;
						}

						dist = CalcDistance(vecData[i].ox, vecData[i].oz, x, z);	// calc dist to this vec
					    dist = 1.0f / dist; 										// calc inverse dist to vector origin

						outVec->x += vecData[i].vx * dist;							// add in this vector
						outVec->y += vecData[i].vz * dist;
						gotVector = true;
					}
				}
			}
		}
	}
	
	float pointX = 0.0f;
	float pointZ = 0.0f;
	
	// ended in nothing happening
	/*if(gGameModeIsForCPUs){
		outVec->x = outVec->y = 0;
		if(gTrackNum == TRACK_NUM_STONEHENGE){
			for(int stonehenge = 0; stonehenge < 13; stonehenge++){
				pointX = GetPointsForTrack(TRACK_NUM_STONEHENGE,true,stonehenge);
				pointZ = GetPointsForTrack(TRACK_NUM_STONEHENGE,false,stonehenge);
				
				if ((pointX / 2 >= minX) && (pointX / 2 <= maxX) &&
					(pointZ / 2 >= minZ) && (pointZ / 2 <= maxZ))
				{
					if (fabs(GetTerrainY(x,z) - y) > 50.0f){
						continue;
					}

					dist = CalcDistance(pointX,pointZ, x, z);	// calc dist to this vec
					dist = 1.0f / dist; 										// calc inverse dist to vector origin

					outVec->x += pointX * dist;							// add in this vector
					outVec->y += pointZ * dist;

					gotVector = true;
				}
			}
			if(gotVector){
				FastNormalizeVector2D(outVec->x, outVec->y, outVec, true);
			}
			else{
				outVec->x = gPlayerInfo[0].coord.x + (-100.0f + RandomFloat() * 100.0f);
				outVec->y = gPlayerInfo[0].coord.z + (-100.0f + RandomFloat() * 100.0f);
				FastNormalizeVector2D(outVec->x, outVec->y, outVec, true);
			}
		}
		printf("vecx: %.2f, vecz: %.2f -> VECTOR GET?: %b \n",outVec->x,outVec->y,gotVector);
	}*/

		/* NORMALIZE THE RESULTING VECTOR TO "AVERAGE" IT */

	if (gotVector)
	{
		FastNormalizeVector2D(outVec->x, outVec->y, outVec, true);
	}

	return(gotVector);
}












