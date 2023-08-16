/**************************************************************************************************************
This is to certify that this project is my own work, based on my personal efforts in studying and
applying the concepts learned. I have constructed the functions and their respective algorithms
and corresponding code by myself. The program was run, tested, and debugged by my own
efforts. I further certify that I have not copied in part or whole or otherwise plagiarized the work
of other students and/or persons.
CANICON, Jan Ambro P. ( 11936258 )
**************************************************************************************************************/

/*	References:
	Universal console clear - https://stackoverflow.com/questions/2347770/
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/*=========================     ALIAS DEFINITIONS     =========================*/
#define DELAYSCALE 1	//Set to 0 to remove all program printing delays
						//Higher int value will make the program run slower

typedef char	str30[31];
typedef char	str5[6];
typedef struct {
	char	cID;
	str30	sName;
	int		nPos;
} player;
typedef struct{
	int		nTileNum;
	str5	sObject;
	int		nStartTile;
} tile;

/*=========================     FUNCTION PROTOTYPES     =========================*/
//Functions for game state
void initTiles (tile board[10][10]);
int startMenu (int* nPlayerCount);
void setSeed (int nGamemode);
void initPlayers (player data[], int nPlayerCount);
void chooseDifficulty (int* nDifficulty);
void prepPlayers (player data[], int nPlayerCount);
void sortTurns (player data[], int nPlayerCount);
void genBoard (tile board[10][10], int nDifficulty);
int loadSave (tile board[10][10], player data[], int *nDifficulty);
void saveGame (tile board[10][10], player data[], int nTurn, int nPlayerCount);
int isConflicted (int nTile, tile objects[], int nObjectCount);

//Functions of game mechanics
int genInt (int nLower, int nUpper);
int chance (int nChance);
void takeTurn (tile board[10][10], player *data, int* nOldPos, int mode);
int checkTile (tile board[10][10], int nPos, int *nRow, int *nCol);
void changePos (tile board[10][10], player *data, int* nLastPos, int nRow, int nCol, int nDifficulty);
int checkWinner (player data[], int nPlayerCount);

//Display functions
void clear (void);
void waitEnter (void);
void ellipses (int nTime);
void displayBoard (tile board[10][10], player playerData[], int nPlayerCount);

//Test mode functions
void turnMode (tile board[10][10], player *data, int* nOldPos);
int setRoll (int *nRoll);

/*=========================     MAIN PROGRAM     =========================*/
int main (void) {
	char	cChoice, cDump;
	int		nPlayerCount = 0, nDifficulty, nDump,
			nGamemode, nObjectState,
			i, nRow, nCol, nWinner = -1;
	tile	board[10][10];
	
	//Program preparation
	initTiles (board);
	nGamemode = startMenu (&nPlayerCount);
	setSeed (nGamemode);
	
	//Player data initialization
	player	playerData[nPlayerCount], *playerDataPtr[nPlayerCount];
	int		nOldPos[nPlayerCount], nLastPos[nPlayerCount];
	initPlayers (playerData, nPlayerCount);
	
	//Game state initialization
	if (nGamemode) {
		chooseDifficulty (&nDifficulty);
		prepPlayers (playerData, nPlayerCount);
		sortTurns (playerData, nPlayerCount);
		genBoard (board, nDifficulty);
	}
	else
		if (loadSave (board, playerData, &nDifficulty)) {
			chooseDifficulty (&nDifficulty);
			genBoard (board, nDifficulty);
			printf ("\n   Resetting player positions due to board regeneration");
			ellipses (750);
			for (i = 0; i < nPlayerCount; i++)
				playerData[i].nPos = 0;
		}
		
	//Creates pointer variables for player data structures
	for (i = 0; i < nPlayerCount; i++) {
		playerDataPtr[i] = &playerData[i];
		nOldPos[i] = 0;
	}
	
	//Game initialization
	clear ();
	printf ("\n   Loading board");
	ellipses (750);
	printf ("\n\n   Game loaded!");
	sleep (1 * DELAYSCALE);
	clear ();
	displayBoard (board, playerData, nPlayerCount);
	printf("\n");
	waitEnter ();
	
	do { //Game loop
		for (i = 0; i < nPlayerCount && nWinner == -1; i++) { //Loops through all players
			//Asks for player movement method depending on game mode
			if (nGamemode == 2)
				turnMode (board, playerDataPtr[i], &nOldPos[i]);
			else
				takeTurn (board, playerDataPtr[i], &nOldPos[i], 0);
			nWinner = checkWinner(playerData, nPlayerCount);
			//Checks for objects if there are no winners
			if (nWinner == -1) {
				nLastPos[i] = nOldPos[i];
				//Continually checks for objects until the player is on a free tile
				do {
					nObjectState = checkTile (board, playerData[i].nPos, &nRow, &nCol);
					if (nObjectState)
						changePos (board, playerDataPtr[i], &nLastPos[i], nRow, nCol, nDifficulty);
					else
						printf ("\n\n\tNo obstacles at %d.", playerData[i].nPos);
				} while (nObjectState);
			}
			//Delays or wait for input buffer depending on game speed
			if (DELAYSCALE != 0)
				sleep (1 * DELAYSCALE);
			else
				waitEnter ();
			clear ();
			displayBoard (board, playerData, nPlayerCount);
			printf ("\n\n\t%s moved from tile %d to tile %d.", playerData[i].sName, nOldPos[i], playerData[i].nPos);
			saveGame (board, playerData, i + 1, nPlayerCount);
			waitEnter ();
		}
	} while (nWinner == -1);
	
	//Winner declaration
	printf ("\n\n\n\t%s is the winner! Congratulations!\n", playerData[nWinner].sName);
	do {
		printf ("\n   Would you like to play again? (Y/N): ");
		scanf ("%c%c", &cChoice, &cDump);
		if (cChoice != 'Y' && cChoice != 'y' && cChoice != 'N' && cChoice != 'n')
			printf ("\n   Invalid choice.");
	} while (cChoice != 'Y' && cChoice != 'y' && cChoice != 'N' && cChoice != 'n');
	
	if (cChoice == 'Y' || cChoice == 'y') {
		printf ("\n   Starting new game");
		ellipses (500);
		clear ();
		nDump = main ();
	}
	
	return 0;
}

/*=========================     FUNCTION DEFINITIONS     =========================*/
/*===============     Functions for game state     ===============*/
/*	Numbers the board 1 to 100 in a winding order
	Pre-condition: None
	@param	board contains tile data
*/
void initTiles (tile board[10][10]) {
	int		nHolder,
			i, j, k, nTile = 100;
	
	//Generates tile data
	for (i = 0; i < 10; i++)
		for (j = 0; j < 10; j++) {
			board[i][j].nTileNum = nTile;
			strcpy (board[i][j].sObject, "\0");
			board[i][j].nStartTile = 0;
			nTile--;
		}
	
	//Reverses number order of alternating lines on the board
	for (i = 1; i < 10; i += 2)
		for (k = 0; k < 10; k++)
			for (j = 1; j < 10; j++)
				if (board[i][j].nTileNum < board[i][j - 1].nTileNum) {
					nHolder = board[i][j].nTileNum;
					board[i][j].nTileNum = board[i][j - 1].nTileNum;
					board[i][j - 1].nTileNum = nHolder;
				}
}

/*	Lets the player choose between starting a new game and continuing a previous game
	Pre-condition: None
	@param	nPlayerCount is the number of people playing
	@return	1 if starting a new game, else 0
*/
int startMenu (int* nPlayerCount) {
	char	cChoice;
	int		nReturn;
	FILE	*fp;
	player	playerDump;
	
	printf ("\n   Welcome to Dogs, Ladders, Slides, and U-turns!\n");
	printf ("   Set to fullscreen windowed for a optimal experience.\n");
	printf ("\t[N] Start a new game\n");
	printf ("\t[C] Continue an existing game\n");
	printf ("\t[T] Test mode\n\n");
	printf ("   Choice: ");
	scanf ("%c", &cChoice);
	
	switch (cChoice) {
		case 'C': case 'c': { //Continued game
			fp = fopen ("player.csv", "r");
			//Starts new game if no save file is found, else counts players in save file
			if (fp == NULL) {
				printf ("\n   No game save found.");
				printf ("\n   Opting to start new game");
				ellipses (500);
				nReturn = 1;
			}
			else {
				while (fscanf (fp, "%c,%[^,],%d\n"
								 , &playerDump.cID, playerDump.sName, &playerDump.nPos) != EOF)
					(*nPlayerCount)++;
				nReturn = 0;
			}
			fclose (fp);
		} break;
		
		case 'N': case 'n':	case 'T': case 't': { //New game
			do {
				printf ("\n   Enter number of players (1 - 10 players): ");
				scanf ("%d", nPlayerCount);
				if (*nPlayerCount < 1)
					printf ("\tInvalid player count.\n");
				else if (*nPlayerCount == 1)
					printf ("\tToo few players.\n");
				else if (*nPlayerCount > 10)
					printf ("\tToo many players.\n");
			} while (*nPlayerCount < 2 || *nPlayerCount > 10);
			if (cChoice == 'T' || cChoice == 't')
				nReturn = 2;
			else if (cChoice == 'N' || cChoice == 'n')
				nReturn = 1;
		} break;
		
		default: { //Invalid input
			printf ("\tInvalid input.");
			sleep (1 * DELAYSCALE);
			clear ();
			nReturn = startMenu (nPlayerCount);
		}
	}
	return nReturn;
}

/*	Asks for a randomizer seed if game is in test mode, otherwise generates a random seed
	Pre-condition: nGamemode is 1 or 2
	@param	mode is the selected game mode
*/
void setSeed (int mode) {
	int		nSeed;
	FILE	*fp_debug;
	
	if (mode == 2) {
		printf ("\n   Randomizer seed (Set to 0 for random seed): ");
		scanf ("%d", &nSeed);
		if (nSeed == 0)
			nSeed = time (NULL);
	}
	else if (mode == 1)
		nSeed = time (NULL);
	srand (nSeed);
	
	fp_debug = fopen ("debug_log.txt", "w");
		fprintf (fp_debug, "Seed: %d\n", nSeed);
	fclose (fp_debug);
}

/*	Initializes player structures to have empty fields
	Pre-condition: The array size of data and the value of nPlayerCount is equal
	@param	data contains player data
	@param	nPlayerCount is the number of people playing
*/
void initPlayers (player data[], int nPlayerCount) {
	int		i;
	
	for (i = 0; i < nPlayerCount; i++) {
		data[i].cID = '\0';
		strcpy (data[i].sName, "\0");
		data[i].nPos = 0;
	}
}

/*	Lets a player choose a difficulty of the game
	Pre-condition: None
	@param	nDifficulty represents the difficulty level of the game
*/
void chooseDifficulty (int* nDifficulty) {	
	do {
		clear ();
		printf ("\n   Choose a difficulty:\n");
		printf ("\t[1] Novice\n");
		printf ("\t[2] Intermediate\n");
		printf ("\t[3] Expert\n\n");
		printf ("   Choice: ");
		scanf ("%d", nDifficulty);
		if (*nDifficulty < 1 || *nDifficulty > 3) {
			printf ("\tDifficulty does not exist.");
			sleep (1 * DELAYSCALE);
		}
	} while (*nDifficulty < 1 || *nDifficulty > 3);
	(*nDifficulty)--;
}

/*	Takes player names and dice rolls for turn ordering
	Pre-condition: None
	@param	data contains player data
	@param	nPlayerCount is the number of people playing
*/
void prepPlayers (player data[], int nPlayerCount) {
	char	cDump;
	int		nDice[2],
			i;

	for (i = 0; i < nPlayerCount; i++) {
		//Takes player name
		do {
			clear ();
			printf ("\n   Enter the name of player %d: ", i + 1);
			scanf (" %[^\n]%c", data[i].sName, &cDump);
			if (strlen (data[i].sName) > 30) {
				printf ("\tPlayer name is too long.");
				sleep (1 * DELAYSCALE);
			}
		} while (strlen (data[i].sName) > 30);
		data[i].cID = 65 + i;
		
		//Rolls dice for player order
		printf ("   Press [Enter] to roll the dice.");
		fflush (stdin);
		while (getchar () != '\n');
		printf ("\n\tRolling dice");
		ellipses (500);
		
		nDice[0] = genInt (1, 6);
		nDice[1] = genInt (1, 6);
		data[i].nPos = nDice[0] + nDice[1];
		
		//Action messages
		printf ("\tRolled %d! (%d and %d)", data[i].nPos, nDice[0], nDice[1]);
		sleep (1 * DELAYSCALE);
		printf ("\n   Recording name and dice roll...");
		sleep (1 * DELAYSCALE);
		printf ("\n");
		waitEnter ();
	}
}

/*	Sorts players according to dice rolls in ascending order
	Pre-condition: None
	@param	data contains player data
	@param	nPlayerCount is the number of people playing
*/
void sortTurns (player data[], int nPlayerCount) {
	int		i, j;
	player	strucHolder;
	
	clear ();
	printf ("\n   Determining player order");
	ellipses (1000);
	
	//Sorts players according to previous dice rolls
	for (i = 0; i < nPlayerCount - 1; i++)
		for (j = 0; j < nPlayerCount - 1; j++)
			if (data[j].nPos > data[j + 1].nPos) {
				strucHolder = data[j];
				data[j] = data[j + 1];
				data[j + 1] = strucHolder;				
			}
	
	//Displays player turn order
	printf ("\n");
	for (i = 0; i < nPlayerCount; i++) {
		printf ("\n\tRolled %d\t| %c: %s", data[i].nPos, data[i].cID, data[i].sName);
		data[i].nPos = 0;
		usleep (750000 * DELAYSCALE);
	}
	
	printf ("\n");
	waitEnter ();
}

/*	Randomizes board objects and saves them into external file
	Pre-condition: None
	@param	board contains tile data
	@param	nDifficulty represents the difficulty level of the game
*/
void genBoard (tile board[10][10], int nDifficulty) {
	int		nSize[2], //Stores generated size of object
			nObjects[3][4] = { //Object count by difficulty
				{10, 12, 5, 5}, //[0]Dog, [1]Ladder
				{10, 8, 8, 10}, //[2]Slide, [3]U-Turn
				{8, 5, 12, 12}
			},
			nFrequency[3][2][4] = { //Frequency of ladders/slides by difficulty
				{{3, 1, 5, 3}, {1, 0, 2, 2}}, //[0]Long and steep, [1]long and wide
				{{2, 0 ,4, 2}, {2, 0, 3, 3}}, //[2]Average, [3]short and steep
				{{1, 0, 3, 1}, {3, 1, 5, 3}}
			},
			nRange[3][2] = { //Length range of ladder and slide
				{7, 8},
				{4, 5},
				{1, 2}
			},
			nDirH, nAccept, nCondition[4],
			i, j, k, nRow, nCol, nCount, nObjectCount = 0, nConflict[2];
	FILE	*fp_board, *fp_debug;
	
	fp_debug = fopen ("debug_log.txt", "a");
	fprintf (fp_debug, "Difficulty: %d\n", nDifficulty);
	
	//Creates array that will store objects created
	for (i = 0; i < 4; i++)
		nObjectCount += nObjects[nDifficulty][i];
	fprintf (fp_debug, "Objects: %d\n", nObjectCount);
	//Creates array that will store tile numbers that will be occupied by objects
	nObjectCount += nObjects[nDifficulty][1];
	nObjectCount += nObjects[nDifficulty][2];
	tile	objects[nObjectCount];
	
	fprintf (fp_debug, "Occupied tiles: %d\n", nObjectCount);
	fprintf (fp_debug, "Dogs: %d\n", nObjects[nDifficulty][0]);
	fprintf (fp_debug, "Ladders: %d (%d tiles)\n", nObjects[nDifficulty][1], 2 * nObjects[nDifficulty][1]);
	fprintf (fp_debug, "Slides: %d (%d tiles)\n", nObjects[nDifficulty][2], 2 * nObjects[nDifficulty][2]);
	fprintf (fp_debug, "U-Turns: %d\n\n\n", nObjects[nDifficulty][3]);
	nObjectCount = 0;
	
	fp_board = fopen ("config.csv", "w");
	//Ladder randomizing
	nCount = 0;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < nFrequency[nDifficulty][0][i]; j++) {
			//Generates object in unconflicted random tile
			do {
				//Sets horizontal direction of ladder
				if (chance (50))
					nDirH = 1;
				else
					nDirH = -1;
				
				//Generates column and row size
				if (i == 0) {
					nSize[0] = genInt (nRange[0][0], nRange[0][1]);
					nSize[1] = genInt (nRange[2][0], nRange[2][1]);
				}
				else if (i == 1) {
					nSize[0] = genInt (nRange[0][0], nRange[0][1]);
					nSize[1] = genInt (nRange[0][0], nRange[0][1]);
				}
				else if (i == 2) {
					nSize[0] = genInt (nRange[1][0], nRange[1][1]);
					nSize[1] = genInt (nRange[1][0], nRange[1][1]);
				}
				else if (i == 3) {
					nSize[0] = genInt (nRange[2][0], nRange[2][1]);
					nSize[1] = genInt (nRange[2][0], nRange[2][1]);
				}
				//Debug log
				if (nDirH == 1)
					fprintf (fp_debug, "Ladder %d: Right", nCount + 1);
				else
					fprintf (fp_debug, "Ladder %d: Left", nCount + 1);
				fprintf (fp_debug, ", %dx%d (HxW); ", nSize[0], nSize[1]);
				
				//Generates column and row number within possible region
				nRow = genInt (nSize[0], 9);
				if (nDirH == 1)
					nCol = genInt (0, 9 - nSize[1]);
				else
					nCol = genInt (nSize[1], 9);
				nSize[1] *= nDirH;
				//Debug log
				fprintf (fp_debug, "%ds, ", board[nRow][nCol].nTileNum);
				fprintf (fp_debug, "%de\n", board[nRow - nSize[0]][nCol + nSize[1]].nTileNum);
				
				//Checks if start and end tiles are in conflict with other tiles
				nConflict[0] = isConflicted (board[nRow][nCol].nTileNum, objects, nObjectCount);
				nConflict[1] = isConflicted (board[nRow - nSize[0]][nCol + nSize[1]].nTileNum, objects, nObjectCount);
				nCondition[0] = !(nConflict[0] + 1);
				nCondition[1] = (nRow != 9 || nCol != 0);
				nCondition[2] = !(nConflict[1] + 1);
				nCondition[3] = (nRow - nSize[0] != 0 || nCol + nSize[1] != 0);
				nAccept = 1;
				for (k = 0; k < 4; k++)
					nAccept *= nCondition[k];
				//Debug log
				if (nAccept)
					fprintf (fp_debug, "Accepted\n\n");
				else {
					fprintf (fp_debug, "Rejected: ");
					if (!nCondition[0])
						fprintf (fp_debug, "\n   - Start tile conflicting with %s"
										 , objects[nConflict[0]].sObject);
					if (!nCondition[1])
						fprintf (fp_debug, "\n   - Start tile on tile 0");
					if (!nCondition[2])
						fprintf (fp_debug, "\n   - End tile conflicting with %s"
										 , objects[nConflict[1]].sObject);
					if (!nCondition[3])
						fprintf (fp_debug, "\n   - End tile on tile 100");
					fprintf (fp_debug, "\n\n");
				}
			} while (!nAccept);
			
			//Sets object name
			strcpy (objects[nObjectCount].sObject, "L");
			if (nCount < 9) {
				objects[nObjectCount].sObject[1] = '1' + nCount;
				objects[nObjectCount].sObject[2] = '\0';
			}
			else {
				objects[nObjectCount].sObject[1] = '1';
				objects[nObjectCount].sObject[2] = '0' + (nCount - 9);
				objects[nObjectCount].sObject[3] = '\0';
			}
			
			//Transfers start tile data
			strcpy (board[nRow][nCol].sObject, objects[nObjectCount].sObject);
			strcat (board[nRow][nCol].sObject, "s");
			board[nRow][nCol].nStartTile = 1;
			//Saves start tile data
			objects[nObjectCount].nTileNum = board[nRow][nCol].nTileNum;
			fprintf (fp_board, "%s,%d,", objects[nObjectCount].sObject, objects[nObjectCount].nTileNum);
			
			//Transfers end tile data
			strcpy (board[nRow - nSize[0]][nCol + nSize[1]].sObject, objects[nObjectCount].sObject);
			strcat (board[nRow - nSize[0]][nCol + nSize[1]].sObject, "e");
			strcat (objects[nObjectCount].sObject, "s");
			//Saves end tile data
			nObjectCount++;
			objects[nObjectCount].nTileNum = board[nRow - nSize[0]][nCol + nSize[1]].nTileNum;
			fprintf (fp_board, "%d\n", objects[nObjectCount].nTileNum);
			strcpy (objects[nObjectCount].sObject, board[nRow - nSize[0]][nCol + nSize[1]].sObject);

			nObjectCount++;
			nCount++;
		}
	}
	//Debug log
	fprintf (fp_debug, "\n\n");
		
	//Slide randomizing
	nCount = 0;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < nFrequency[nDifficulty][1][i]; j++) {
			//Generates object in unconflicted random tile
			do {
				//Sets horizontal direction of ladder
				if (chance (50))
					nDirH = 1;
				else
					nDirH = -1;
				
				//Generates column and row size	
				if (i == 0) {
					nSize[0] = genInt (nRange[0][0], nRange[0][1]);
					nSize[1] = genInt (nRange[2][0], nRange[2][1]);
				}
				else if (i == 1) {
					nSize[0] = genInt (nRange[0][0], nRange[0][1]);
					nSize[1] = genInt (nRange[0][0], nRange[0][1]);
				}
				else if (i == 2) {
					nSize[0] = genInt (nRange[1][0], nRange[1][1]);
					nSize[1] = genInt (nRange[1][0], nRange[1][1]);
				}
				else if (i == 3) {
					nSize[0] = genInt (nRange[2][0], nRange[2][1]);
					nSize[1] = genInt (nRange[2][0], nRange[2][1]);
				}	
				//Debug log
				if (nDirH == 1)
					fprintf (fp_debug, "Slide %d: Right", nCount + 1);
				else
					fprintf (fp_debug, "Slide %d: Left", nCount + 1);
				fprintf (fp_debug, ", %dx%d (HxW); ", nSize[0], nSize[1]);
				
				//Generates column and row number within possible region			
				nRow = genInt (0, 9 - nSize[0]);
				if (nDirH == 1)
					nCol = genInt (0, 9 - nSize[1]);
				else
					nCol = genInt (nSize[1], 9);
				nSize[1] *= nDirH;
				//Debug log
				fprintf (fp_debug, "%ds, ", board[nRow][nCol].nTileNum);
				fprintf (fp_debug, "%de\n", board[nRow + nSize[0]][nCol + nSize[1]].nTileNum);
				
				//Checks if start and end tiles are in conflict with other tiles
				nConflict[0] = isConflicted (board[nRow][nCol].nTileNum, objects, nObjectCount);
				nConflict[1] = isConflicted (board[nRow + nSize[0]][nCol + nSize[1]].nTileNum, objects, nObjectCount);
				nCondition[0] = !(nConflict[0] + 1);
				nCondition[1] = (nRow != 0 || nCol != 0);
				nCondition[2] = !(nConflict[1] + 1);
				nCondition[3] = (nRow + nSize[0] != 9 || nCol + nSize[1] != 0);
				nAccept = 1;
				for (k = 0; k < 4; k++)
					nAccept *= nCondition[k];
				//Debug log
				if (nAccept)
					fprintf (fp_debug, "Accepted\n\n");
				else {
					fprintf (fp_debug, "Rejected: ");
					if (!nCondition[0])
						fprintf (fp_debug, "\n   - Start tile conflicting with %s\n"
										 , objects[nConflict[0]].sObject);
					if (!nCondition[1])
						fprintf (fp_debug, "\n   - Start tile on tile 100\n");
					if (!nCondition[2])
						fprintf (fp_debug, "\n   - End tile conflicting with %s\n"
										 , objects[nConflict[1]].sObject);
					if (!nCondition[3])
						fprintf (fp_debug, "\n   - End tile on tile 0\n");
					fprintf (fp_debug, "\n");
				}
			} while (!nAccept);
			
			//Sets object name
			strcpy (objects[nObjectCount].sObject, "S");
			if (nCount < 9) {
				objects[nObjectCount].sObject[1] = '1' + nCount;
				objects[nObjectCount].sObject[2] = '\0';
			}
			else {
				objects[nObjectCount].sObject[1] = '1';
				objects[nObjectCount].sObject[2] = '0' + (nCount - 9);
				objects[nObjectCount].sObject[3] = '\0';
			}
			
			//Transfers start tile data
			strcpy (board[nRow][nCol].sObject, objects[nObjectCount].sObject);
			strcat (board[nRow][nCol].sObject, "s");
			board[nRow][nCol].nStartTile = 1;
			//Saves start tile data
			objects[nObjectCount].nTileNum = board[nRow][nCol].nTileNum;
			fprintf (fp_board, "%s,%d,", objects[nObjectCount].sObject, objects[nObjectCount].nTileNum);
			
			//Transfers end tile data
			strcpy (board[nRow + nSize[0]][nCol + nSize[1]].sObject, objects[nObjectCount].sObject);
			strcat (board[nRow + nSize[0]][nCol + nSize[1]].sObject, "e");
			strcat (objects[nObjectCount].sObject, "s");
			//Saves end tile data
			nObjectCount++;
			objects[nObjectCount].nTileNum = board[nRow + nSize[0]][nCol + nSize[1]].nTileNum;
			fprintf (fp_board, "%d\n", objects[nObjectCount].nTileNum);
			strcpy (objects[nObjectCount].sObject, board[nRow + nSize[0]][nCol + nSize[1]].sObject);

			nObjectCount++;
			nCount++;
		}
	}	
	//Debug log
	fprintf (fp_debug, "\n\n");
	
	//Dog randomizing
	for (i = 0; i < nObjects[nDifficulty][0]; i++) {
		//Generates object in unconflicted random tile
		do {
			nRow = rand () % 10;
			nCol = rand () % 10;
			//Debug log
			fprintf (fp_debug, "Dog %d: %d\n", i + 1, board[nRow][nCol].nTileNum);
			
			//Checks if object is in conflict with other tiles
			nConflict[0] = isConflicted (board[nRow][nCol].nTileNum, objects, nObjectCount);
			nCondition[0] = !(nConflict[0] + 1);
			nCondition[1] = (nRow != 0 || nCol != 0);
			nCondition[2] = (nRow != 9 || nCol != 0);
			nAccept = 1;
			for (j = 0; j < 3; j++)
				nAccept *= nCondition[j];
			//Debug log
			if (nAccept) 
				fprintf (fp_debug, "Accepted\n\n");
			else {
				fprintf (fp_debug, "Rejected; ");
				if (!nCondition[0])
					fprintf (fp_debug, "Object conflicting at %s\n\n", objects[nConflict[0]].sObject);
				else if (!nCondition[1])
					fprintf (fp_debug, "Object on tile 100\n\n");
				else if (!nCondition[2])
					fprintf (fp_debug, "Object on tile 0\n\n");
			}
		} while (!nAccept);
		objects[nObjectCount].nTileNum = board[nRow][nCol].nTileNum;
		strcpy (objects[nObjectCount].sObject, "D");
		if (i < 9) {
			objects[nObjectCount].sObject[1] = '1' + i;
			objects[nObjectCount].sObject[2] = '\0';
		}
		else {
			objects[nObjectCount].sObject[1] = '1';
			objects[nObjectCount].sObject[2] = '0' + (i - 9);
			objects[nObjectCount].sObject[3] = '\0';
		}
		strcpy (board[nRow][nCol].sObject, objects[nObjectCount].sObject);
		fprintf (fp_board, "%s,%d,0\n", board[nRow][nCol].sObject, objects[nObjectCount].nTileNum);
		nObjectCount++;
	}
	//Debug log
	fprintf (fp_debug, "\n\n");
	
	//U-turn randomizing
	for (i = 0; i < nObjects[nDifficulty][3]; i++) {
		do {
		//Generates object in unconflicted random tile
			nRow = rand () % 10;
			nCol = rand () % 10;
			//Debug log
			fprintf (fp_debug, "U-Turn %d: %d\n", i + 1, board[nRow][nCol].nTileNum);
			
			//Checks if object is in conflict with other tiles
			nConflict[0] = isConflicted (board[nRow][nCol].nTileNum, objects, nObjectCount);
			nCondition[0] = !(nConflict[0] + 1);
			nCondition[1] = (nRow != 0 || nCol != 0);
			nCondition[2] = (nRow != 9 || nCol != 0);
			nAccept = 1;
			for (j = 0; j < 3; j++)
				nAccept *= nCondition[j];
			//Debug log
			if (nAccept) 
				fprintf (fp_debug, "Accepted\n\n");
			else {
				fprintf (fp_debug, "Rejected; ");
				if (!nCondition[0])
					fprintf (fp_debug, "Object conflicting at %s\n\n", objects[nConflict[0]].sObject);
				else if (!nCondition[1])
					fprintf (fp_debug, "Object on tile 100\n\n");
				else if (!nCondition[2])
					fprintf (fp_debug, "Object on tile 0\n\n");
			}
		} while (!nAccept);
		objects[nObjectCount].nTileNum = board[nRow][nCol].nTileNum;
		strcpy (objects[nObjectCount].sObject, "U");
		if (i < 9) {
			objects[nObjectCount].sObject[1] = '1' + i;
			objects[nObjectCount].sObject[2] = '\0';
		}
		else {
			objects[nObjectCount].sObject[1] = '1';
			objects[nObjectCount].sObject[2] = '0' + (i - 9);
			objects[nObjectCount].sObject[3] = '\0';
		}
		strcpy (board[nRow][nCol].sObject, objects[nObjectCount].sObject);
		fprintf (fp_board, "%s,%d,0\n", board[nRow][nCol].sObject, objects[nObjectCount].nTileNum);
		nObjectCount++;
	}
	fclose (fp_board);
	fclose (fp_debug);
}

/*	Loads a game save and loads board config from file
	Pre-condition: player.csv was found in startMenu function
	@param	board contains tile data
	@param	data contains player data
	@return	1 if save is not found, otherwise 0
*/
int loadSave (tile board[10][10], player data[], int *nDifficulty) {
	int		nStartTile, nEndTile, nObjects[3] = {32, 36, 37}, nReturn = 0,
			nObjectCount = 0,
			i, j;
	str30	sObject;
	FILE	*fp_player, *fp_board;
	
	//Attempts to load save files
	fp_player = fopen ("player.csv", "r");
		
	fp_board = fopen ("config.csv", "r");
	if (fp_board == NULL) {
		printf ("\n   Failed to locate board configuration file.");
		sleep (1 * DELAYSCALE);
		printf ("\n   Opting to generate new board");
		ellipses (500);
		clear ();
		nReturn = 1;
	}
	
	//Loads player data
	if (fp_player != NULL)
		for (i = 0; fscanf (fp_player, "%c,%[^,],%d\n", &data[i].cID, data[i].sName, &data[i].nPos) != EOF; i++);
	fclose (fp_player);
	
	//Loads board configuration
	if (fp_board != NULL) {
		while (fscanf (fp_board, "%4[^,],%d,%d\n", sObject, &nStartTile, &nEndTile) != EOF) {
			for (i = 0; i < 10; i++)
				for (j = 0; j < 10; j++)
					if (board[i][j].nTileNum == nStartTile) {
						strcpy (board[i][j].sObject, sObject);
						if (sObject[0] == 'L' || sObject[0] == 'S') {
							strcat (board[i][j].sObject, "s");
							board[i][j].nStartTile = 1;
						}
					}
					else if (board[i][j].nTileNum == nEndTile) {
						strcpy (board[i][j].sObject, sObject);
						if (sObject[0] == 'L' || sObject[0] == 'S')
							strcat (board[i][j].sObject, "e");
					}
			nObjectCount++;
		}
	}
	fclose (fp_board);
	
	for (i = 0; i < 3; i++)
		if (nObjectCount == nObjects[i])
			*nDifficulty = i;
	return nReturn;
}

/*	Saves player details and board config
	Pre-condition: None
	@param	board contains tile data
	@param	data contains player data
	@param	nTurn is the next player supposed to take a turn
	@param	nPlayerCount is the number of people playing
*/
void saveGame (tile board[10][10], player data[], int nTurn, int nPlayerCount) {
	int		i;
	FILE	*fp;
	
	//Writes player.csv based on current turn order
	fp = fopen ("player.csv", "w");
	for (i = nTurn; i < nPlayerCount; i++)
		fprintf (fp, "%c,%s,%d\n", data[i].cID, data[i].sName, data[i].nPos);
	for (i = 0; i < nTurn; i++)
		fprintf (fp, "%c,%s,%d\n", data[i].cID, data[i].sName, data[i].nPos);
	fclose (fp);
}

/*	Checks if the most recently generated object conflicts with existing objects
	Pre-condition: None
	@param	nTile is the tile number of the most recently generated object
	@param	nObjectPos are the occupied tiles
	@param	nObjectCount is the number of objects generated
	@return	Index of conflict if found, else -1
*/
int isConflicted (int nTile, tile objects[], int nObjectCount) {
	int		i;
	
	for (i = 0; i < nObjectCount; i++)
		if (nTile == objects[i].nTileNum)
			return i;
	return -1;
}

/*===============     Functions of game mechanics     ===============*/
/*	Returns a random value within a certain range
	Pre-condition: nLower is less than nUpper, and both are positive signed integers
	@param	nLower is the lower limit of the RNG
	@param	nUpper is the upper limit of the RNG
	@return	Randomly generated number within range
*/
int genInt (int nLower, int nUpper) {
	return ((rand () % (nUpper - nLower + 1)) + nLower);
}

/*	Probability generator
	Pre-condition: nChance ranges from 1 to 100
	@param	nChance is the chance (in %) to return 1
	@return	1 if RNG gives value within probability, else 0
*/
int chance (int nChance) {
	int		nRand = rand () % 100;
	
	if (nRand > 0 && nRand < nChance)
		return 1;
	else
		return 0;
}

/*	Lets player with the turn roll a dice to move position
	Pre-condition: None
	@param	board contains tile data
	@param	*data is a pointer to player data
	@param	*nOldPos is the previous position of the player taking the turn
	@param	mode defines if the function will be called in test mode
*/
void takeTurn (tile board[10][10], player *data, int* nOldPos, int mode) {
	char	cChoice;
	int		nRoll = 0, nObject[2];
	
	if (mode)
		mode = setRoll (&nRoll);
		
	//Player turn buffer for rolling dice
	clear ();
	printf ("\n   %s's turn:", data->sName);
	printf ("\n   Currently on tile %d.", data->nPos);
	//Dice roll modes
	if (!mode) {
		printf ("\n   Press [Enter] to roll the dice.");
		fflush (stdin);
		while (getchar () != '\n');
		printf ("\n	Rolling dice");
		ellipses (500);
	
		//Player dice rolls
		nRoll = genInt (1, 6);
		if (data->nPos < 94)
			nRoll += genInt (1, 6);
	}
	
	//Updates player positioning
	*nOldPos = data->nPos;
	data->nPos += nRoll;

	//Prints action messages
	printf ("	Rolled %d!", nRoll);
	if (nRoll == 12)
		printf (" You rolled a [Double Six]!");
	sleep (1 * DELAYSCALE);
	printf ("\n\n   Advancing from %d by %d tiles.", *nOldPos, nRoll);
	sleep (1 * DELAYSCALE);
	
	//Double-six roll scenario
	if (nRoll == 12 && data->nPos < 100) {
		printf ("\n\n\n   ");
		if (checkTile (board, data->nPos, &nObject[0], &nObject[1]))
			printf ("You are standing on an obstacle (%s).\n   ", board[nObject[0]][nObject[1]].sObject);
		else
			printf ("No obstacles on current tile.\n   ");
		printf ("Would you like to roll again? (Y/N): ");
		scanf (" %c", &cChoice);
		if (cChoice == 'Y' || cChoice == 'y') {
			if (mode)
				mode = setRoll (&nRoll);
			if (!mode) {
				printf ("	Rolling dice");
				ellipses (500);
	
				nRoll = genInt (1, 6);
				if (data->nPos < 94)
					nRoll += genInt (1, 6);
			}
			
			data->nPos += nRoll;
			printf ("	Rolled %d!", nRoll);
			sleep (1 * DELAYSCALE);
			printf ("\n\n   Advancing from %d by %d tiles.", data->nPos - nRoll, nRoll);
			sleep (1 * DELAYSCALE);
		}
	}
	
	//Exceeding rolls scenario
	if (data->nPos > 100) {
		printf ("\n\n   You went past tile 100 by %d tiles. ", data->nPos - 100);
		data->nPos = 100 - (data->nPos - 100);
		printf ("Moving back to %d.", data->nPos);
	}
}

/*	Returns 1 if the current player is standing on a tile with an object
	Pre-condition: None
	@param	board contains tile data
	@param	nPos is the tile the player is on
	@param	*nRow will hold first index of board corresponding to the tile the object is on
	@param	*nCol will hold second index of board corresponding to the tile the object is on
	@return	1 if an object is found, else 0
*/
int checkTile (tile board[10][10], int nPos, int *nRow, int *nCol) {
	char	cObject;
	int		nObjectState,
			i, j;
			
	if (nPos == 0)
		return 0;
	for (i = 0; i < 10; i++)
		for (j = 0; j < 10; j++)
			if (nPos == board[i][j].nTileNum) {
				cObject = board[i][j].sObject[0];
				if (cObject == 'D' || cObject == 'U')
					nObjectState = 1;
				else if ((cObject == 'L' || cObject == 'S') && board[i][j].nStartTile)
					nObjectState = 1;
				else
					nObjectState = 0;
				
				if (nObjectState) {
					*nRow = i;
					*nCol = j;
					return 1;
				}
				else
					return 0;
				}
	return 0;
}

/*	Moves the player to another position on the board depending on the object encountered
	Pre-condition: None
	@param	board contains tile data
	@param	*data is a pointer to player data
	@param	*nLastPos is the previous position of the player
	@param	nRow is the first index of the board corresponding to the tile the object is on
	@param	nCol is the second index of the board corresponding to the tile the object is on
	@param	nDifficulty represents the difficulty level of the game
*/
void changePos (tile board[10][10], player *data, int* nLastPos, int nRow, int nCol, int nDifficulty) {
	char	cObject = board[nRow][nCol].sObject[0];
	int		nHolder, nStrLength = strlen(board[nRow][nCol].sObject) - 1,
			nChances[3] = {80, 50, 20},
			i, j;
	str30	sObject = "\0";
	
	//Player encounters a dog tile
	if (cObject == 'D') {
		//Action messages
		printf ("\n\n	[%s ENCOUNTERED A DOG]", data->sName);
		sleep (1 * DELAYSCALE);
		
		//Updates previous player position and generates random tile for dog to run to as new player position
		*nLastPos = data->nPos;
		if (chance (nChances[nDifficulty]))
			data->nPos = genInt (data->nPos + 1, 99);
		else
			data->nPos = genInt (1, data->nPos - 1);
		
		//Action messages
		printf ("\n   The dog ran to tile %d!", data->nPos);
		sleep (1 * DELAYSCALE);
		printf ("\n	[%s MOVED FROM TILE %d to TILE %d]", data->sName, *nLastPos, data->nPos);
		sleep (1 * DELAYSCALE);
	}
	
	//Player encounters a ladder start tile
	else if (cObject == 'L') {
		//Action messages and update to previous player position
		printf ("\n\n	[%s ENCOUNTERED A LADDER]", data->sName);
		sleep (1 * DELAYSCALE);
		*nLastPos = data->nPos;
		
		//Looks for a ladder end tile
		strncpy (sObject, board[nRow][nCol].sObject, nStrLength);
		strcat (sObject, "e");
		for (i = 0; i < 10; i++)
			for (j = 0; j < 10; j++)
				if (strcmp (sObject, board[i][j].sObject) == 0)
					data->nPos = board[i][j].nTileNum;
		
		//Action messages and update to previous player position
		printf ("\n	[%s CLIMBED FROM TILE %d TO TILE %d]", data->sName, *nLastPos, data->nPos);
		*nLastPos = data->nPos;
		sleep (1 * DELAYSCALE);
	}
	
	//Player encounters a slide start tile
	else if (cObject == 'S') {
		//Action messages and update to previous player position
		printf ("\n\n	[%s ENCOUNTERED A SLIDE]", data->sName);
		sleep (1 * DELAYSCALE);
		
		//Looks for a slide end tile
		*nLastPos = data->nPos;
		strncpy (sObject, board[nRow][nCol].sObject, nStrLength);
		strcat (sObject, "e");
		for (i = 0; i < 10; i++)
			for (j = 0; j < 10; j++)
				if (strcmp (sObject, board[i][j].sObject) == 0)
					data->nPos = board[i][j].nTileNum;
					
		//Action messages and update to previous player position
		printf ("\n	[%s FELL FROM TILE %d TO TILE %d]", data->sName, *nLastPos, data->nPos);
		*nLastPos = data->nPos;
		sleep (1 * DELAYSCALE);
	}
	
	//Player encounters a u-turn tile
	else if (cObject == 'U') {
		//Action messages
		printf ("\n\n	[%s ENCOUNTERED A U-TURN]", data->sName);
		sleep (1 * DELAYSCALE);
		
		//Returns player to old position
		nHolder = *nLastPos;
		*nLastPos = data->nPos;
		data->nPos = nHolder;

		//Action messages
		printf ("\n	[%s WENT BACK FROM %d to TILE %d]", data->sName, *nLastPos, data->nPos);
		sleep (1 * DELAYSCALE);
	}
}

/*	Checks if a player is on tile 100
	Pre-condition: None
	@param	data contains player data
	@param	nPlayerCount is the number of people playing
	@return	Index of the winner if found, else -1
*/
int checkWinner (player data[], int nPlayerCount) {
	int		i;
	
	for (i = 0; i < nPlayerCount; i++)
		if (data[i].nPos == 100)
			return i;
	return -1;
}

/*===============     Display functions     ===============*/
/*	Clears screen
	Pre-condition:None
*/
void clear (void) {
    #if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        system("clear");
    #endif

    #if defined(_WIN32) || defined(_WIN64)
        system("cls");
    #endif
}

/*	Waits for an newline user input
	Pre-condition: None
*/
void waitEnter (void) {
	printf ("\n\tPress [Enter] to continue...");
	fflush (stdin);
	while (getchar () != '\n');
}
/*	Prints delayed "loading" ellipses
	Pre-condition: None
	@param	nTime is the time (in milliseconds) the printing will be delayed
*/	
void ellipses (int nTime) {
	int		i;
	
	nTime *= 1000;
	usleep (nTime * DELAYSCALE);
	for (i = 0; i < 3; i++) {
		printf(" .");
		usleep (nTime * DELAYSCALE);
	}
	printf ("\n");
}

/*	Displays the whole board
	Pre-condition: None
	@param	board contains tile data
	@param	data contains player data
	@param	nPlayerCount is the number of people playing
*/
void displayBoard (tile board[10][10], player data[], int nPlayerCount) {	
	int		nSpaces,
			i, j, k;
	
	//Prints horizontal line
	printf ("\n\t");
		for (j = 0; j < 81; j++)
			if (j % 8)
				printf ("-");
			else
				printf ("+");
		printf ("\n");
	
	//Prints row
	for (i = 0; i < 10; i++) {
		printf("\t|"); //Left margin
		//Prints top level with tile number and object type
		for (j = 0; j < 10; j++) {
			nSpaces = 7;
			printf("%d", board[i][j].nTileNum);
			if (board[i][j].nTileNum > 0)
				nSpaces--;
			if (board[i][j].nTileNum > 9)
				nSpaces--;
			if (board[i][j].nTileNum == 100)
				nSpaces--;
			if (strlen (board[i][j].sObject) > 0)
				nSpaces -= strlen (board[i][j].sObject);	
			for (k = 0; k < nSpaces; k++)
				printf (" ");
			printf ("%s", board[i][j].sObject);
			printf ("|");
		}
		printf ("\n");
		
		//Prints middle level with player ID
		printf("\t|"); //Left margin
		for (j = 0; j < 10; j++) { //Cell
			printf (" ");
			nSpaces = 6;
			for (k = 0; k < nPlayerCount && k < 5; k++)
				if (data[k].nPos == board[i][j].nTileNum) {
					printf ("%c", data[k].cID);
					nSpaces--;
				}
			while (nSpaces > 0) {
				printf (" ");
				nSpaces--;
			}
			printf ("|");	
		}
		printf ("\n");
		
		//Prints bottom level with player ID
		printf ("\t|"); //Left margin
		for (j = 0; j < 10; j++) { //Cell
			printf (" ");
			nSpaces = 6;
			for (k = 5; k < nPlayerCount; k++)
				if (data[k].nPos == board[i][j].nTileNum) {
					printf ("%c", data[k].cID);
					nSpaces--;
				}
			while (nSpaces > 0) {
				printf (" ");
				nSpaces--;
			}
			printf ("|");	
		}
		printf ("\n");
		
		//Prints bottom horizontal line
		printf ("\t");
		for (j = 0; j < 81; j++)
			if (j % 8)
				printf ("-");
			else
				printf ("+");
		printf ("\n");
	}
}

/*	Test mode dice roll options
	Pre-condition: None
	@param	board contains tile data
	@param	*data is a pointer to player data
	@param	*nOldPos is the previous position of the player taking the turn
*/
void turnMode (tile board[10][10], player *data, int* nOldPos) {
	int		nChoice, nChange, nValid = 0;
	
	printf ("\n\n\n   [%s]", data->sName);
	printf ("\n   MOVEMENT OPTIONS");
	printf ("\n\t[1] Roll dice");
	printf ("\n\t[2] Set dice roll value");
	printf ("\n\t[3] Set player tile position");
	printf ("\n   Choose mode: ");
	scanf ("%d", &nChoice);
	
	switch (nChoice) {
		case 1: takeTurn (board, data, nOldPos, 2); break;
		case 2: {
			while (!nValid) {
				printf ("   Enter roll value: ");
				scanf ("%d", &nChange);
				if (nChange < 1 || nChange > 12)
					printf ("Invalid roll value.\n");
				else if (data->nPos + nChange > 100)
					printf ("Roll value exceeds tile 100.\n");
				else
					nValid = 1;
			}
			*nOldPos = data->nPos;
			data->nPos += nChange;
		} break;
		case 3: {
			while (!nValid) {
				printf ("   Enter tile number: ");
				scanf ("%d", &nChange);
				if (nChange < 0 || nChange > 100)
					printf ("Invalid tile number.\n");
				else
					nValid = 1;
			}
			*nOldPos = data->nPos;
			data->nPos = nChange;
		}
	}
}

/*===============     Test mode functions     ===============*/
/*	Test mode dice value setter
	Pre-condition: None
	@param	nRoll contains roll value
	@return	1 if player assigns roll value, 0 otherwise
*/
int setRoll (int *nRoll) {
	int		nChoice;
	
	printf ("\n   Select dice roll mode.");
	printf ("\n\t[1] Randomized");
	printf ("\n\t[2] Assigned value");
	do {
		printf ("\n   Choice: ");
		scanf ("%d", &nChoice);
		if (nChoice != 1 && nChoice != 2)
			printf ("\n   Invalid mode.");
	} while (nChoice != 1 && nChoice != 2);
	
	nChoice--;
	
	if (nChoice)
		do {
				printf ("\n   Enter roll value: ");
				scanf ("%d", nRoll);
		} while (*nRoll < 1 || *nRoll > 12);
		
	return nChoice;
}
