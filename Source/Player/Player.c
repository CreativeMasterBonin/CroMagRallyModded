/****************************/
/*   	PLAYER.C   			*/
/* (c)2000 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/****************************/
/*    EXTERNALS             */
/****************************/

#include "game.h"

/****************************/
/*    PROTOTYPES            */
/****************************/



/****************************/
/*    CONSTANTS             */
/****************************/


/*********************/
/*    VARIABLES      */
/*********************/


short		gNumRealPlayers = 1;                // # of actual human players (on computer and/or network)
short       gNumTotalPlayers = 0;               // # of total player characters in game (real + CPU)
short		gNumLocalPlayers = 1;				// 2 if split-screen, otherwise 1

ObjNode		*gCurrentPlayer;
short		gCurrentPlayerNum = 0;				// current player number of player being processed (via MovePlayer et. al.)
short		gMyNetworkPlayerNum = 0;			// not the same as the ClientID, just a player number that the host assigns.  The host is always player #0

PlayerInfoType	gPlayerInfo[MAX_PLAYERS];

OGLColorRGB	gTagColor;

bool super_sub_mode = false; // enable super sub mode for non-water tracks for fun

/******************** INIT PLAYER INFO ***************************/
//
// Called once at beginning of game (after network has been setup if needed).
//

void InitPlayerInfo_Game(void)
{
short	i;

	memset(gPlayerInfo, 0, sizeof(gPlayerInfo));		// init everything to 0

	for (i = 0; i < MAX_PLAYERS; i++)
	{
		gPlayerInfo[i].objNode			= nil;

		gPlayerInfo[i].sex 				= i&1;			// altername male/female

		gPlayerInfo[i].startX 			= 0;
		gPlayerInfo[i].startZ 			= 0;
		gPlayerInfo[i].coord.x 			= 0;
		gPlayerInfo[i].coord.y 			= 0;
		gPlayerInfo[i].coord.z 			= 0;
		gPlayerInfo[i].onThisMachine 	= false;

		gPlayerInfo[i].wheelObj[0] 		=
		gPlayerInfo[i].wheelObj[1] 		=
		gPlayerInfo[i].wheelObj[2] 		=
		gPlayerInfo[i].wheelObj[3] 		=
		gPlayerInfo[i].headObj = nil;



		gPlayerInfo[i].splitPaneNum		= -1;


		gPlayerInfo[i].team = i % 2;			// set team for capture the flag mode


		if (gGameMode == GAME_MODE_CAPTUREFLAG)
		{
			gPlayerInfo[i].skin = gPlayerInfo[i].team == RED_TEAM ? CAVEMAN_SKIN_RED : CAVEMAN_SKIN_GREEN;
		}
		else
		{
			gPlayerInfo[i].skin = i % NUM_CAVEMAN_SKINS;
		}


				/* CAR SPECS INFO */

		gPlayerInfo[i].steering					= 0;




			/* AI */

		gPlayerInfo[i].isComputer 	    = true;


	}

			/* NETWORK GAME */

	if (gNetGameInProgress)
	{
		gPlayerInfo[gMyNetworkPlayerNum].onThisMachine = true;			// set the local player
		gPlayerInfo[gMyNetworkPlayerNum].splitPaneNum = 0;
	}

			/* LOCAL GAME */
	else
	{
		for (i = 0; i < gNumLocalPlayers; i++)
		{
			gPlayerInfo[i].onThisMachine = true;
			gPlayerInfo[i].splitPaneNum  = i;
		}
	}


	    /* REAL PLAYERS ARE NOT CONTROLLED BY CPU */

	for (i = 0; i < gNumRealPlayers; i++)
	{
		gPlayerInfo[i].isComputer 	    = false;        // these are real players, not computer players
    }


			/* SEE HOW MANY PLAYERS IN GAME */

	// if the game mode is in cpus mode, then all cpus can join in, otherwise, do not let cpus be in the game
	if(gGameModeIsForCPUs){
		if(gGameMode != GAME_MODE_MULTIPLAYERRACE){
			gNumTotalPlayers = MAX_PLAYERS;
		}
		else{
			gNumTotalPlayers = gNumRealPlayers;
		}
	}
	else{
		switch(gGameMode)
		{
			case	GAME_MODE_PRACTICE:
			case	GAME_MODE_TOURNAMENT:
	//		case	GAME_MODE_MULTIPLAYERRACE:
					gNumTotalPlayers = MAX_PLAYERS;                 // use them all
					break;

			default:
					gNumTotalPlayers = gNumRealPlayers;				// no CPU players in battle modes
					break;


		}
	}
}


/******************* INIT PLAYERS AT START OF LEVEL ********************/
//
// Initializes player stuff at the beginning of each track.
//

void InitPlayersAtStartOfLevel(void)
{
int		i,j,type;
Boolean	taken[NUM_LAND_CAR_TYPES];

	gWorstHumanPlace = 0;
    // give each player a unique id so that they can be recognized later
    for(i = 0; i < gNumTotalPlayers; i++){
        gPlayerInfo[i].playerID = i;
    }
	
	super_sub_mode = gGamePrefs.superSubMode; // repetitive, but not changing lower code


		/* FIRST MARK WHICH CAR TYPES THE HUMANS HAVE */

	for (i = 0; i < NUM_LAND_CAR_TYPES; i++)						// first mark all unused
		taken[i] = false;

	for (i = 0; i < gNumTotalPlayers; i++)
	{
		if (!gPlayerInfo[i].isComputer)								// check for human player
		{
			taken[gPlayerInfo[i].vehicleType] = true;				// mark this used
		}
	}


	i = GetNumAgesCompleted();
	if (i > 2)														// dont get extra cars after winning, so pin @ 2
		i = 2;
	type = 6 + (i * 2)-1;											// start @ end of usable cars so it will pick best cars


			/* SET SOME GLOBALS */

	float multiplierForPosXZ = 1.0;
	
	for (i = 0; i < gNumTotalPlayers; i++)
	{
        // mod: set CPU vehicle types per difficulty
        // mod: allow CPU vehicles to be cheatsy more often to give them an advantage due to bad AI logic
		// mod: in battle mode, cpus can check to see if they are faster or slower than the player, and pick the car that fits best for the scenario, with randomization
		if (gPlayerInfo[i].isComputer){
			gPlayerInfo[i].carStats = gPlayerInfo[i].carStatsCopy; // reset car stats everytime cpus are loaded
			if(gGameModeIsForCPUs){
				if(gGamePrefs.difficulty >= DIFFICULTY_MEDIUM){
					// randomize car stats in battle mode
					gPlayerInfo[i].carStats.acceleration += RandomRange(200,300);
					gPlayerInfo[i].carStats.maxSpeed += RandomRange(100,200);
					gPlayerInfo[i].carStats.suspension += RandomRange(50,90);
					gPlayerInfo[i].carStats.tireTraction += RandomRange(25,45);
					gPlayerInfo[i].carStats.airFriction = 0;//RandomRange(0,700); // no friction?
					gPlayerInfo[i].carStats.minPlaningAngle = 15;
					gPlayerInfo[i].carStats.minPlaningSpeed = 1;
					
					if(gPlayerInfo[0].vehicleType < CAR_TYPE_LOG){
						gPlayerInfo[i].vehicleType = RandomRange(CAR_TYPE_MAMMOTH,CAR_TYPE_GEODE);
					}
					else if(gPlayerInfo[0].vehicleType >= CAR_TYPE_LOG && gPlayerInfo[0].vehicleType < CAR_TYPE_TROJANHORSE){
						gPlayerInfo[i].vehicleType = RandomRange(CAR_TYPE_LOG,CAR_TYPE_ROCK);
					}
					else{
						gPlayerInfo[i].vehicleType = RandomRange(CAR_TYPE_TROJANHORSE,CAR_TYPE_CHARIOT);
					}
				}
				else{
					if(gGamePrefs.difficulty == DIFFICULTY_EASY){
						gPlayerInfo[i].vehicleType = RandomRange(CAR_TYPE_MAMMOTH,CAR_TYPE_TURTLE);
					}
					else{
						gPlayerInfo[i].vehicleType = CAR_TYPE_MAMMOTH;
					}
				}
			}
			else{
				if (gGamePrefs.difficulty == DIFFICULTY_HARD){
					gPlayerInfo[i].vehicleType = RandomRange(6, type); // duplicate cars
					// even more fun, make the cpus check if the first player has a chariot, and if so, take the advantage and copy it-ish
					if(gPlayerInfo[0].vehicleType == CAR_TYPE_CHARIOT){
						gPlayerInfo[i].vehicleType = RandomRange(CAR_TYPE_CATAPULT,CAR_TYPE_CHARIOT);
					}
					// modify the CPU's cars to be more fun
					gPlayerInfo[i].carStats.acceleration += RandomRange(17,35);
					gPlayerInfo[i].carStats.maxSpeed += RandomRange(20,36);
					gPlayerInfo[i].carStats.suspension += RandomRange(10,15);
					gPlayerInfo[i].carStats.tireTraction += RandomRange(14,27);
				}
				else if(gGamePrefs.difficulty == DIFFICULTY_MEDIUM){
					gPlayerInfo[i].vehicleType = RandomRange(3, type); // only level 3+ cars allowed
					gPlayerInfo[i].carStats.suspension += RandomRange(20,50);
					gPlayerInfo[i].carStats.tireTraction += RandomRange(27,39);
				}
				else if(gGamePrefs.difficulty == DIFFICULTY_EASY){
					gPlayerInfo[i].vehicleType = RandomRange(0, 5); // only simplistic to level 5 cars allowed
				}
				else if(gGamePrefs.difficulty == DIFFICULTY_SIMPLISTIC){
					gPlayerInfo[i].vehicleType = RandomRange(0, 1); // only the first 2 simplistic cars allowed
				} // mod: not sure how you'd get another difficulty mode, but if so, pick vehicles that haven't been taken yet...
				else{
					while(taken[type])										// skip over vehicles already used by Humans
						type--;

					gPlayerInfo[i].vehicleType = type--;
				}
			}
		}
		
		// correct positioning for extra players that do not have a default starting position in the map
		if(gPlayerInfo[i].playerID >= 6 && gPlayerInfo[i].isComputer){
			gPlayerInfo[i].coord.x = gPlayerInfo[0].startX - (100 * multiplierForPosXZ); // each new player should be offset by an amount
			gPlayerInfo[i].coord.z = gPlayerInfo[0].startZ - (100 * multiplierForPosXZ); // otherwise they stack in the same position (which is automatically corrected anyways, but this is better than nothing)
		}
		multiplierForPosXZ += 1.27351f; // update multiplier after the player check

		gPlayerInfo[i].coord.y = GetTerrainY(gPlayerInfo[i].startX,gPlayerInfo[i].startX);

			/* CREATE THE CAR MODEL */

		if (gTrackNum == TRACK_NUM_ATLANTIS)
			InitPlayer_Submarine(i, &gPlayerInfo[i].coord, gPlayerInfo[i].startRotY);
		else
            //InitPlayer_Car(i, &gPlayerInfo[i].coord, gPlayerInfo[i].startRotY);
            /* SUPER SUB MODE CHECK INITIALIZE */
            if(!gIsSelfRunningDemo){
                if(super_sub_mode){
                    if(!gPlayerInfo[i].isComputer){
                        InitPlayer_Submarine(i, &gPlayerInfo[i].coord, gPlayerInfo[i].startRotY);
                        gPlayerInfo[i].vehicleType = CAR_TYPE_SUB;
                    }
                    else{ // it is super sub mode. can cpus be subs?
						if(gGamePrefs.cpusAreSubs){
							InitPlayer_Submarine(i, &gPlayerInfo[i].coord, gPlayerInfo[i].startRotY);
						}
						else{
							InitPlayer_Car(i, &gPlayerInfo[i].coord, gPlayerInfo[i].startRotY);
						}
                    }
                }
                else{// not super sub mode, but can cpus can be subs?
					if(gGamePrefs.cpusAreSubs){
						InitPlayer_Submarine(i, &gPlayerInfo[i].coord, gPlayerInfo[i].startRotY);
					}
					else{
						InitPlayer_Car(i, &gPlayerInfo[i].coord, gPlayerInfo[i].startRotY);
					}
                }
            }
            else{
                InitPlayer_Car(i, &gPlayerInfo[i].coord, gPlayerInfo[i].startRotY);
            }
        // moving on...
        
        gPlayerInfo[i].canMoveEarly = false;

		gPlayerInfo[i].objNode->InvincibleTimer = 0;

		gPlayerInfo[i].controlBits		= 0;
		gPlayerInfo[i].controlBits_New	= 0;
		gPlayerInfo[i].analogSteering.x	= 0;
		gPlayerInfo[i].analogSteering.y	= 0;

		gPlayerInfo[i].distToFloor				= 0;
		gPlayerInfo[i].skidDot					= 0;
		gPlayerInfo[i].mostRecentFloorY 		= 0;

		gPlayerInfo[i].onWater			 		= false;
		gPlayerInfo[i].waterY 					= 0;

		gPlayerInfo[i].lapNum					= -1;			// start @ -1 since we cross the finish line @ start
		gPlayerInfo[i].checkpointNum			= gNumCheckpoints-1;
		gPlayerInfo[i].place					= i;
		gPlayerInfo[i].distToNextCheckpoint		= 0;
		gPlayerInfo[i].raceComplete				= false;
		gPlayerInfo[i].cheated					= false;

		gPlayerInfo[i].snowParticleGroup		= -1;
		gPlayerInfo[i].snowTimer				= 0;
		gPlayerInfo[i].frozenTimer				= 0;


		for (j = 0; j < MAX_CHECKPOINTS; j++)				// start with all checkpoints tagged to trick lapNum @ start of race
			gPlayerInfo[i].checkpointTagged[j] 	= true;

		gPlayerInfo[i].currentThrust	= 0;
		gPlayerInfo[i].gasPedalDown		= false;
		gPlayerInfo[i].accelBackwards	= false;
		gPlayerInfo[i].movingBackwards	= false;
		gPlayerInfo[i].braking			= false;
		gPlayerInfo[i].isPlaning		= false;
		gPlayerInfo[i].greasedTiresTimer = 0;
		gPlayerInfo[i].wrongWay			= false;
		gPlayerInfo[i].steering			= 0;
		gPlayerInfo[i].currentRPM		= 0;
		gPlayerInfo[i].submarineImmobilized = 0;
		gPlayerInfo[i].bumpSoundTimer	= 0;
        gPlayerInfo[i].submarineAdditionalSpeed = 0;

			/* DRAG DEBRIS */

		gPlayerInfo[i].tiresAreDragging		= false;
		gPlayerInfo[i].alwaysDoDrag			= false;
		gPlayerInfo[i].dragDebrisTimer		= 0;
		gPlayerInfo[i].dragDebrisParticleGroup = -1;
		gPlayerInfo[i].dragDebrisMagicNum 	= 0;
		gPlayerInfo[i].dragDebrisTexture	= 0;

		gPlayerInfo[i].lastSkidSegCoord.x 	=
		gPlayerInfo[i].lastSkidSegCoord.y 	= 0;
		gPlayerInfo[i].lastSkidVector.x 	=
		gPlayerInfo[i].lastSkidVector.y 	= 0;
		gPlayerInfo[i].skidSmokeParticleGroup = -1;
		gPlayerInfo[i].skidSmokeMagicNum 	= 0;
		gPlayerInfo[i].skidSmokeTimer		= 0;
		gPlayerInfo[i].makingSkid			= false;
		gPlayerInfo[i].skidChannel			= -1;
		gPlayerInfo[i].skidSoundTimer		= 0;

		gPlayerInfo[i].skidColor.r 			=
		gPlayerInfo[i].skidColor.g 			=
		gPlayerInfo[i].skidColor.b 			=
		gPlayerInfo[i].skidColor.a 			= 0;

				/* POWERUP */

		gPlayerInfo[i].nitroTimer			= 0;
		gPlayerInfo[i].stickyTiresTimer		= 0;
		gPlayerInfo[i].superSuspensionTimer	= 0;
		gPlayerInfo[i].numTokens			= 0;
		gPlayerInfo[i].invisibilityTimer	= 0;
        gPlayerInfo[i].zappedTimer          = 0; // reset zapped timer INIT
		gPlayerInfo[i].flamingTimer			= 0;

				/* BATTLE MODES */

		gPlayerInfo[i].tagTimer				= TAG_TIME_LIMIT;
		gPlayerInfo[i].tagOccilation		= 0;
		gPlayerInfo[i].isIt					= false;
		gPlayerInfo[i].isEliminated			= false;
		gPlayerInfo[i].health				= 1.0;
		gPlayerInfo[i].impactResetTimer		= 0;

			/* GROUND TILE INFO */

		gPlayerInfo[i].groundTraction		= 1.0;
		gPlayerInfo[i].groundFriction		= 1.0;
		gPlayerInfo[i].groundSteering		= 1.0;
		gPlayerInfo[i].groundAcceleration	= 1.0;
		gPlayerInfo[i].noSkids				= false;


				/* WEAPON INFO */

		gPlayerInfo[i].powType				= POW_TYPE_NONE;
		gPlayerInfo[i].powTypeBeingThrown	= POW_TYPE_NONE;
		gPlayerInfo[i].powQuantity			= 0;

				/* CAMERA */

		gPlayerInfo[i].cameraRingRot = 0;
		gPlayerInfo[i].cameraUserRot = 0;

		gPlayerInfo[i].camera.cameraLocation.x = 0;
		gPlayerInfo[i].camera.cameraLocation.y = 0;
		gPlayerInfo[i].camera.cameraLocation.z = 0;
		gPlayerInfo[i].camera.pointOfInterest.x = 0;
		gPlayerInfo[i].camera.pointOfInterest.y = 0;
		gPlayerInfo[i].camera.pointOfInterest.z = 0;
		gPlayerInfo[i].camera.upVector.x = 0;
		gPlayerInfo[i].camera.upVector.y = 1;
		gPlayerInfo[i].camera.upVector.z = 0;

		gPlayerInfo[i].cameraMode = CAMERA_MODE_NORMAL1 + 1; // init camera mode


			/* AI */

		gPlayerInfo[i].oldPositionTimer	= 0;
		gPlayerInfo[i].oldPosition.x	= 0;
		gPlayerInfo[i].oldPosition.y	= 0;
		gPlayerInfo[i].oldPosition.z	= 0;
		gPlayerInfo[i].reverseTimer		= 0;
		gPlayerInfo[i].attackTimer		= 2;					// dont attack for the first few seconds
		gPlayerInfo[i].targetedPlayer	= -1;					// no players targeted yet
		gPlayerInfo[i].targetingTimer	= 0;
		gPlayerInfo[i].pathVec.x	= 0;
		gPlayerInfo[i].pathVec.y	= 0;



			/* SOUND */

		gPlayerInfo[i].engineChannel = -1;


			/* SCORING */

		memset(gPlayerInfo[i].lapTimes, 0, sizeof(gPlayerInfo[i].lapTimes));


			/* RESET CAR PHYSICS INFO */

		SetPhysicsForVehicleType(i);
	}


	SetDefaultCameraModeForAllPlayers();
}

#pragma mark -

/********* SET PLAYER PARMS FROM TILE ATTRIBUTES *****************/
//
// INPUT: 	flags = tile attribute flags
//

void SetPlayerParmsFromTileAttributes(short playerNum, uint16_t flags)
{
			/* IF ON WATER */

	if (gPlayerInfo[playerNum].onWater)
	{
		gPlayerInfo[playerNum].groundTraction = .3;
		gPlayerInfo[playerNum].groundFriction = .3;
		gPlayerInfo[playerNum].groundSteering = .3;
		gPlayerInfo[playerNum].groundAcceleration = .5;
		gPlayerInfo[playerNum].noSkids			= true;
		gPlayerInfo[playerNum].dragDebrisTexture = PARTICLE_SObjType_Splash;
		gPlayerInfo[playerNum].alwaysDoDrag 	= true;
		return;
	}

			/* SEE IF ON ICE */

	if (flags & TILE_ATTRIB_ICE)
	{
		gPlayerInfo[playerNum].groundTraction = .05f;
		gPlayerInfo[playerNum].groundFriction = .05f;
		gPlayerInfo[playerNum].groundSteering = .2f;
		gPlayerInfo[playerNum].groundAcceleration = .3;
		gPlayerInfo[playerNum].noSkids			= true;
		gPlayerInfo[playerNum].dragDebrisTexture = -1;
		gPlayerInfo[playerNum].alwaysDoDrag 	= false;
	}

			/* SEE IF ON SNOW */

	else
	if (flags & TILE_ATTRIB_SNOW)
	{
		gPlayerInfo[playerNum].groundTraction = .3f;
		gPlayerInfo[playerNum].groundFriction = .5f;
		gPlayerInfo[playerNum].groundSteering = .6f;
		gPlayerInfo[playerNum].groundAcceleration = .7;
		gPlayerInfo[playerNum].noSkids			= false;
		gPlayerInfo[playerNum].skidColor.r		= 1.0f;
		gPlayerInfo[playerNum].skidColor.g		= 1.0f;
		gPlayerInfo[playerNum].skidColor.b		= 1.0f;
		gPlayerInfo[playerNum].dragDebrisTexture = PARTICLE_SObjType_SnowDust;
		gPlayerInfo[playerNum].alwaysDoDrag 	= true;
	}

		/* SET DEFAULTS */

	else
	{
		gPlayerInfo[playerNum].groundTraction = 1.0;
		gPlayerInfo[playerNum].groundFriction = 1.0;
		gPlayerInfo[playerNum].groundSteering = 1.0;
		gPlayerInfo[playerNum].groundAcceleration = 1.0;
		gPlayerInfo[playerNum].noSkids			= false;
		gPlayerInfo[playerNum].skidColor.r		= .0f;
		gPlayerInfo[playerNum].skidColor.g		= .0f;
		gPlayerInfo[playerNum].skidColor.b		= .0f;
		gPlayerInfo[playerNum].dragDebrisTexture = PARTICLE_SObjType_Dirt;
		gPlayerInfo[playerNum].alwaysDoDrag 	= false;
	}


}




#pragma mark -

/******************** FIND CLOSEST PLAYER ****************************/
//
// Returns -1 if no other players in range
//
// Ignore thePlayer if != nil
//

short FindClosestPlayer(ObjNode *thePlayer, float x, float z, float range, Boolean allowCPUCars, float *dist)
{
short	p,bestP = -1;
ObjNode	*target;
float	bestDist = 10000000000;
float	d;

	for (p = 0; p < gNumTotalPlayers; p++)
	{
		target = gPlayerInfo[p].objNode;

		if (target == thePlayer)
			continue;

		if (!allowCPUCars)
		{
			if (gPlayerInfo[p].isComputer)
				continue;
		}

		d = CalcDistance(x,z, target->Coord.x, target->Coord.z);
		if (d > range)
			continue;

		if (d < bestDist)
		{
			bestDist = d;
			bestP = p;
		}
	}

	*dist = bestDist;
	return(bestP);
}



/******************** FIND CLOSEST PLAYER IN FRONT ****************************/
//
// Returns -1 if no other players in range
//
// INPUT:  angle, 0 = full 180 degrees in front, 1.0 = none
//

short FindClosestPlayerInFront(ObjNode *theNode, float range, Boolean allowCPUCars, float *dist, float angle)
{
short	p,bestP = -1;
ObjNode	*target;
float	bestDist = 10000000000;
float	x,z,d, r, dot;
OGLVector2D	aimVec, toVec;

	x = theNode->Coord.x;
	z = theNode->Coord.z;

	r = theNode->Rot.y;							// calc aim vector
	aimVec.x = -sin(r);
	aimVec.y = -cos(r);


	for (p = 0; p < gNumTotalPlayers; p++)
	{
		target = gPlayerInfo[p].objNode;

		if (target == theNode)
			continue;

		if (!allowCPUCars)
		{
			if (gPlayerInfo[p].isComputer)
				continue;
		}

		d = CalcDistance(x,z, target->Coord.x, target->Coord.z);		// calc dist & check range
		if (d > range)
			continue;

		toVec.x = target->Coord.x - x;
		toVec.y = target->Coord.z - z;
		FastNormalizeVector2D(toVec.x, toVec.y, &toVec, true);			// calc normal to target

		dot = OGLVector2D_Dot(&aimVec, &toVec);							// dot = angle
		if (dot < angle)													// if in back, then skip
			continue;

		if (d < bestDist)
		{
			bestDist = d;
			bestP = p;
		}
	}

	*dist = bestDist;
	return(bestP);
}


/******************** FIND CLOSEST PLAYER IN BACK ****************************/
//
// Returns -1 if no other players in range
//
// INPUT:  angle, 0 = full 180 degrees, -1.0 = none
//

short FindClosestPlayerInBack(ObjNode *theNode, float range, Boolean allowCPUCars, float *dist, float angle)
{
short	p,bestP = -1;
ObjNode	*target;
float	bestDist = 10000000000;
float	x,z,d, r, dot;
OGLVector2D	aimVec, toVec;

	x = theNode->Coord.x;
	z = theNode->Coord.z;

	r = theNode->Rot.y;							// calc aim vector
	aimVec.x = -sin(r);
	aimVec.y = -cos(r);


	for (p = 0; p < gNumTotalPlayers; p++)
	{
		target = gPlayerInfo[p].objNode;

		if (target == theNode)
			continue;

		if (!allowCPUCars)
		{
			if (gPlayerInfo[p].isComputer)
				continue;
		}

		d = CalcDistance(x,z, target->Coord.x, target->Coord.z);		// calc dist & check range
		if (d > range)
			continue;

		toVec.x = target->Coord.x - x;
		toVec.y = target->Coord.z - z;
		FastNormalizeVector2D(toVec.x, toVec.y, &toVec, true);			// calc normal to target

		dot = OGLVector2D_Dot(&aimVec, &toVec);							// dot = angle
		if (dot > angle)												// if in front, then skip
			continue;

		if (d < bestDist)
		{
			bestDist = d;
			bestP = p;
		}
	}

	*dist = bestDist;
	return(bestP);
}


#pragma mark -


/******************* CHOOSE TAGGED PLAYER *********************/

void ChooseTaggedPlayer(void)
{
short	i,j;

	i = j = RandomRange(0, gNumTotalPlayers-1);

	while(gPlayerInfo[i].isEliminated)
	{
		i++;
		if (i >= gNumTotalPlayers)			// wrap around
			i = 0;
		if (i == j)							// error check
			DoFatalAlert("ChooseTaggedPlayer: all players have been eliminated");
	}

	gPlayerInfo[i].isIt = true;
	gWhoIsIt = i;

}

/*********************** UPDATE TAG MARKER **********************/
//
// Occilatets the filter color on the tagged player
//

void UpdateTagMarker(void)
{
short	p;
float	o,r,g,b;
ObjNode	*obj;

	for (p = 0; p < gNumTotalPlayers; p++)					// scan all players to update their colors
	{
		if (p == gWhoIsIt)
		{
			o = gPlayerInfo[p].tagOccilation += gFramesPerSecondFrac * 2.0f;

			gTagColor.r = r = fabs(sin(o));
			gTagColor.g = g = fabs(cos(o));
			gTagColor.b = b = fabs(tan(o));

			obj = gPlayerInfo[p].objNode;
			while(obj)
			{
				obj->ColorFilter.r = r;
				obj->ColorFilter.g = g;
				obj->ColorFilter.b = b;
				obj = obj->ChainNode;
			}
		}
		else
		{
			obj = gPlayerInfo[p].objNode;
			while(obj)
			{
				obj->ColorFilter.r =
				obj->ColorFilter.g =
				obj->ColorFilter.b = 1.0;
				obj = obj->ChainNode;
			}
		}
	}
}



/***************** PLAYER LOSE HEALTH ************************/

void PlayerLoseHealth(short p, float damage)
{

	if (gTrackCompleted)
		return;

	if (gPlayerInfo[p].isEliminated)
		return;

	gPlayerInfo[p].health -= damage;

			/* SEE IF DEAD */

	if (gPlayerInfo[p].health <= 0.0f)
	{
		gPlayerInfo[p].health = 0;
		gPlayerInfo[p].isEliminated = true;
		gNumPlayersEliminated++;
		
		// special case for cpu play (you have to defeat the cpus to win)
		if(gGameModeIsForCPUs){
			if(!gPlayerInfo[p].isComputer){
				if(gNumPlayersEliminated < (gNumTotalPlayers-1)){
					ShowWinLose(p,0,0);
				}
				else{
					ShowWinLose(p,1,0);
				}
			}
			return;
		}

		if (gNumPlayersEliminated < (gNumTotalPlayers-1))		// if more than 1 player remaining, then post ELIMINATED message
		{
			if(!gPlayerInfo[p].isComputer){
				ShowWinLose(p, 0, 0);								// this player is eliminated
			}
		}
	}
}

#pragma mark -


/******************** SET INVISIBILITY*************************/

void SetInvisibility(short playerNum)
{
	if(gPlayerInfo[playerNum].invisibilityTimer <= 0.0f){
		gPlayerInfo[playerNum].invisibilityTimer = RandomRange(10,30);                // set duration of invisibility
	}
	else{
		gPlayerInfo[playerNum].invisibilityTimer += 2.0;
	}
}

/******************** SET STICKY TIRES *************************/

void SetStickyTires(short playerNum)
{
	SetTractionPhysics(&gPlayerInfo[playerNum].carStats, 3.0);
    if(gPlayerInfo[playerNum].stickyTiresTimer <= 0.0f){
        gPlayerInfo[playerNum].stickyTiresTimer = RandomRange(10,20);                // set duration of sticky tires
    }
    else{
        gPlayerInfo[playerNum].stickyTiresTimer += 5.0;
    }
}

/******************** SET SUSPENSION POW *************************/

void SetSuspensionPOW(short playerNum)
{
	SetSuspensionPhysics(&gPlayerInfo[playerNum].carStats, 3.0);
    if(gPlayerInfo[playerNum].superSuspensionTimer <= 0.0f){
        gPlayerInfo[playerNum].superSuspensionTimer = RandomRange(10,20);       // set duration
    }
    else{
        gPlayerInfo[playerNum].superSuspensionTimer += 5.0;                // set duration
    }
}


/****************** SET CAR STATUS BITS *******************/

void SetCarStatusBits(short	playerNum, uint32_t bits)
{
ObjNode *obj;

	obj = gPlayerInfo[playerNum].objNode;
	while(obj)
	{
		obj->StatusBits |= bits;
		obj = obj->ChainNode;
	}
}

/****************** CLEAR CAR STATUS BITS *******************/

void ClearCarStatusBits(short	playerNum, uint32_t bits)
{
ObjNode *obj;

	obj = gPlayerInfo[playerNum].objNode;
	while(obj)
	{
		obj->StatusBits &= ~bits;
		obj = obj->ChainNode;
	}
}





















