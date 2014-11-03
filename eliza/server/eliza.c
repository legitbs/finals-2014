/*
-------------------------------------------------------
 A very simple text based space trading game...
 ... with a bug...
... sirgoon

 Speed changes added by Lightning
------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

#include <stdint.h>
#include <fcntl.h>
#include <stdarg.h>
#include <time.h>

#define true (-1)
#define false (0)

// TURN this on to assist in debugging the exploit
//#define DEBUG_ON		1
//#define DEBUG_LEVEL2_ON		1

#define WriteStr(Str) write(1, Str, strlen(Str))

#ifdef DEBUG_ON
void check_world( void );
#endif 

// TODO: Set this back
#define MAX_SHIP_ITEMS				35		// Maximum no of items in the ships hold
// FOR easy DEBUGGING: #define MAX_SHIP_ITEMS				(3)

// CHANGE THIS VALUE TO CHEAT THE GAME TO GET A HEAD START
#define PLAYER_START_CASH	1000

#define MAX_IDLE_SECS		90

// THIS: Needs to be long enough for an effective ROP chain (and some loss due to the trampoline) 
#define COMMAND_LINE_LENGTH 800

#define OK       0
#define NO_INPUT 1
#define TOO_LONG 2

#define ITEM_TYPE_FUEL					(0)
#define ITEM_TYPE_DATACHIPS				(1)
#define ITEM_TYPE_FOOD					(2)
#define ITEM_TYPE_LIQUOR				(3)
#define ITEM_TYPE_PROCESSORS			(4)
#define ITEM_TYPE_NUERAL_ENHANCERS		(5)
#define ITEM_TYPE_SLAVE_BOTS			(6)
#define ITEM_TYPE_SPACE_DEBRIS			(7)
#define ITEM_TYPE_CONTRABAND			(8)
#define ITEM_TYPE_STIMULANTS			(9)
#define ITEM_TYPE_WEAPONS				(10)
#define ITEM_TYPE_MINERALS				(11)
#define ITEM_TYPE_AMMO					(12)
#define ITEM_TYPE_ROBOTICS				(13)
#define ITEM_TYPE_ALIEN_ARTIFACTS		(14)
#define ITEM_TYPE_GEMS					(15)
#define ITEM_COUNT						(16)

#define ITEM_END						(100)


#define ITEM_MODIFIER_CHEAP					(0.5)
#define ITEM_MODIFIER_REGULAR				(1)
#define ITEM_MODIFIER_EXCELLENT				(1.8)
#define ITEM_MODIFIER_COUNT					3


#define PLANET_GOVERNMENT_ANARCHY			0
#define PLANET_GOVERNMENT_FUEDAL			1
#define PLANET_GOVERNMENT_MULTIGOV			2
#define PLANET_GOVERNMENT_DICTATORSHIP		3
#define PLANET_GOVERNMENT_COMMUNIST			4
#define PLANET_GOVERNMENT_CONFEDERACY		5
#define PLANET_GOVERNMENT_DEMOCACRY			6
#define PLANET_GOVERNMENT_CORPORATESTATE	7
#define PLANET_GOVERNMENT_THEOCRACY			8
#define PLANET_GOVERNMENT_MONARCHY			9
#define PLANET_GOVERNMENT_COUNT				10

#define PLANET_ECONOMY_INDUSTRIAL			0
#define PLANET_ECONOMY_AGRICULTURE			1
#define PLANET_ECONOMY_BANKING				2
#define PLANET_ECONOMY_TRADE				3
#define PLANET_ECONOMY_MILITARYIND			4
#define PLANET_ECONOMY_COUNT				5

#define PLANET_ECONOMY_POOR_MODIFIER		(0.8)
#define PLANET_ECONOMY_AVERAGE_MODIFIER		(1.0)
#define PLANET_ECONOMY_RICH_MODIFIER		(1.1)
#define PLANET_ECONOMY_MODIFIER_COUNT		3

void sig_alarm_handler( int signum )
{
	// Send connection close
	WriteStr ("Connection idle, closing.\n" );

	// Shutdown
	exit( 1 );
}

double sqrt(double x) 
{
  __asm__ volatile ("fsqrt" : "+t" (x));
  return x;
}

typedef struct PLANET_ITEM_INFO
{
	uint8_t item_type;
	float	item_price_modifier;
} planet_item_info;

float itemModifierValueTable[ITEM_MODIFIER_COUNT] = 
{
	ITEM_MODIFIER_CHEAP,
	ITEM_MODIFIER_REGULAR,
	ITEM_MODIFIER_EXCELLENT
};

// Planet items for sale
planet_item_info planetEconomySellItems[PLANET_ECONOMY_COUNT][ITEM_COUNT] =
{ 
	// PLANET_ECONOMY_INDUSTRIAL
	{ 
		{ ITEM_TYPE_DATACHIPS,			0.8 },			// 0
		{ ITEM_TYPE_PROCESSORS,			0.8 },			// 1
		{ ITEM_TYPE_NUERAL_ENHANCERS,	1.0 },			// 2
		{ ITEM_TYPE_SLAVE_BOTS,			0.7 },			// 3
		{ ITEM_TYPE_SPACE_DEBRIS,		1.0 },			// 4
		{ ITEM_TYPE_CONTRABAND,			1.1 },			// 5
		{ ITEM_TYPE_STIMULANTS,			1.1 },			// 6
		{ ITEM_TYPE_WEAPONS,			1.2 },			// 7
		{ ITEM_TYPE_MINERALS,			1.2 },			// 8
		{ ITEM_TYPE_ROBOTICS,			0.8 },			// 9
		{ ITEM_END,						1.0 },			// 10
		{ ITEM_END,						1.0 },			// 11
		{ ITEM_END,						1.0 },			// 12
		{ ITEM_END,						1.0 },			// 13
		{ ITEM_END,						1.0 },			// 14
		{ ITEM_END,						1.0 }			// 15
	},

	// PLANET_ECONOMY_AGRICULTURE
	{ 
		{ ITEM_TYPE_FOOD,				0.7 },			// 0
		{ ITEM_TYPE_LIQUOR,				0.8 },			// 1
		{ ITEM_TYPE_MINERALS,			0.9 },			// 2
		{ ITEM_TYPE_SLAVE_BOTS,			1.0 },			// 3
		{ ITEM_TYPE_SPACE_DEBRIS,		1.0 },			// 4
		{ ITEM_TYPE_CONTRABAND,			1.1 },			// 5
		{ ITEM_TYPE_WEAPONS,			1.0 },			// 6
		{ ITEM_TYPE_ROBOTICS,			1.2 },			// 7
		{ ITEM_TYPE_ALIEN_ARTIFACTS,	1.3 },			// 8
		{ ITEM_END,						1.0 },			// 9
		{ ITEM_END,						1.0 },			// 10
		{ ITEM_END,						1.0 },			// 11
		{ ITEM_END,						1.0 },			// 12
		{ ITEM_END,						1.0 },			// 13
		{ ITEM_END,						1.0 },			// 14
		{ ITEM_END,						1.0 }			// 15
	},

	// PLANET_ECONOMY_BANKING
	{ 
		{ ITEM_TYPE_FOOD,				1.1 },			// 0
		{ ITEM_TYPE_LIQUOR,				1.2 },			// 1
		{ ITEM_TYPE_GEMS,				0.7 },			// 2
		{ ITEM_TYPE_DATACHIPS,			1.0 },			// 3
		{ ITEM_TYPE_STIMULANTS,			1.1 },			// 4
		{ ITEM_TYPE_CONTRABAND,			0.9 },			// 5
		{ ITEM_TYPE_NUERAL_ENHANCERS,	1.2 },			// 6
		{ ITEM_TYPE_ROBOTICS,			1.0 },			// 7
		{ ITEM_END,						1.0 },			// 8
		{ ITEM_END,						1.0 },			// 9
		{ ITEM_END,						1.0 },			// 10
		{ ITEM_END,						1.0 },			// 11
		{ ITEM_END,						1.0 },			// 12
		{ ITEM_END,						1.0 },			// 13
		{ ITEM_END,						1.0 },			// 14
		{ ITEM_END,						1.0 }			// 15
	},

	
	// PLANET_ECONOMY_TRADE
	{ 
		{ ITEM_TYPE_FOOD,					0.9 },		// 0
		{ ITEM_TYPE_LIQUOR,					0.8 },		// 1
		{ ITEM_TYPE_DATACHIPS,				1.0 },		// 2
		{ ITEM_TYPE_PROCESSORS,				1.0 },		// 3
		{ ITEM_TYPE_NUERAL_ENHANCERS,		1.0 },		// 4
		{ ITEM_TYPE_SLAVE_BOTS,				1.0 },		// 5
		{ ITEM_TYPE_SPACE_DEBRIS,			0.9 },		// 6
		{ ITEM_TYPE_CONTRABAND,				0.7 },		// 7
		{ ITEM_TYPE_STIMULANTS,				0.7 },		// 8
		{ ITEM_TYPE_WEAPONS,				1.1 },		// 9
		{ ITEM_TYPE_MINERALS,				1.0 },		// 10
		{ ITEM_TYPE_AMMO,					1.1 },		// 11
		{ ITEM_TYPE_ROBOTICS,				1.0 },		// 12
		{ ITEM_TYPE_ALIEN_ARTIFACTS,		1.4 },		// 13
		{ ITEM_TYPE_GEMS,					1.3 },		// 14
		{ ITEM_END,							1.0 },		// 15
	},

	// PLANET_ECONOMY_MILITARYIND
	{ 
		{ ITEM_TYPE_LIQUOR,				1.0 },		// 0
		{ ITEM_TYPE_DATACHIPS,			1.0 },		// 1
		{ ITEM_TYPE_PROCESSORS,			0.9 },		// 2
		{ ITEM_TYPE_NUERAL_ENHANCERS,	0.9 },		// 3
		{ ITEM_TYPE_SLAVE_BOTS,			1.1 },		// 4
		{ ITEM_TYPE_SPACE_DEBRIS,		1.1 },		// 5
		{ ITEM_TYPE_STIMULANTS,			1.1 },		// 6
		{ ITEM_TYPE_WEAPONS,			0.7 },		// 7
		{ ITEM_TYPE_AMMO,				0.6 },		// 8
		{ ITEM_TYPE_ROBOTICS,			1.0 },		// 9
		{ ITEM_END,						1.0 },		// 10
		{ ITEM_END,						1.0 },		// 11
		{ ITEM_END,						1.0 },		// 12
		{ ITEM_END,						1.0 },		// 13
		{ ITEM_END,						1.0 },		// 14
		{ ITEM_END,						1.0 }		// 15
	}
};


float planetEconomyModifierTable[PLANET_ECONOMY_MODIFIER_COUNT] = { PLANET_ECONOMY_POOR_MODIFIER, PLANET_ECONOMY_AVERAGE_MODIFIER, PLANET_ECONOMY_RICH_MODIFIER };


#define MIN_GALAXY_SIZE				(700)		// Minimum of 1000 planets
#define MAX_GALAXY_SIZE				(1000)		// Maximum of 2000 planets

#define PLANET_GRID_MIN				(7200)
#define PLANET_GRID_MAX				(7800)
#define PLANET_GRID_CENTER			PLANET_GRID_MIN + ((PLANET_GRID_MAX - PLANET_GRID_MIN) / 2)
#define PLANET_MIN_DISTANCE			(5.0)		// 5 LY

#define PLANET_POPULATION_MIN		(1000)
#define PLANET_POPULATION_MAX		(100000)

#define MARKET_LIST_RANGE			(100)		// 100 LY

#define MAX_PLANET_ITEMS			(60)		// Maximum no of items on a planet

#define SHIP_MAX_FUEL				(10000)

#define FUEL_PER_LY					(10)

#define SHIP_MAX_CARGO				(150000)

#define MAX_JUMP_RANGE				(SHIP_MAX_FUEL/FUEL_PER_LY)

#define PLANET_ITEM_MAX_AMOUNT		(10000)

#define PLANET_BASE_CASH_PER_POP	(8)

#define JUMP_LY_PER_TICK			(10)

#define PLANET_FUEL_MAX				(50000)


//uint16_t itemBasePriceTable[ITEM_COUNT] = { 1, 40, 8, 16, 120, 300, 600, 6, 200, 75, 125, 20, 25, 150, 30, 400 };
uint16_t itemBasePriceTable[ITEM_COUNT] = { 1, 20, 4, 8, 60, 150, 300, 3, 100, 40, 60, 10, 15, 75, 15, 200 };
//uint16_t itemBaseAmountTable[ITEM_COUNT] = { 2000, 400, 800, 800, 400, 400, 400, 1600, 200, 200, 800, 2000, 2000, 400, 200, 100 };
uint16_t itemBaseAmountTable[ITEM_COUNT] = { 4000, 800, 1600, 1600, 800, 800, 800, 3200, 400, 400, 1600, 4000, 4000, 800, 400, 200 };

uint16_t itemBaseWeightTable[ITEM_COUNT] = { 10, 1, 10, 10, 1, 1, 100, 1000, 2, 2, 200, 2000, 500, 300, 10, 30 };


const char* itemBaseNameTable[ITEM_COUNT] = { "Fuel", "Datachips", "Food", "Liquor", "Processors", "Neural Enhancers", "Slave Bots", "Space Debris", "Contraband", "Stimulants", "Weapons", "Minerals", "Ammunition", "Robotics", "Alien Artifacts", "Gems" };

const char* itemModifierNameTable[ITEM_MODIFIER_COUNT] = { "Cheap",  "Regular", "Excellent" };

const char* governmentNamesTable[PLANET_GOVERNMENT_COUNT]={ "Anarchy", "Feudal", "Multi-gov", "Dictatorship", "Communist", "Confederacy", "Democracy", "Corporate State", "Theocracy", "Monarchy" };

const char* economyTypeTable[PLANET_ECONOMY_COUNT] = { "Industrial", "Agriculture", "Banking", "Trade", "Military Industrial" };

const char* economyModifierNameTable[PLANET_ECONOMY_MODIFIER_COUNT] = { "Poor", "Average", "Rich" };


const char *consonants[] = {"b", "c", "d", "f", "g", "h", "i", "j", "k", "l", "m", "n", "p", "q", "r", "s", "t", "v", "w", "x", "y", "z", ""};
const char *vowels[] = {"a", "e", "o", "u", ""};
const char *characters3[] = {"br", "cr", "dr", "fr", "gr", "pr", "str", "tr", "bl", "cl", "fl", "gl", "pl", "sl", "sc", "sk", "sm", "sn", "sp", "st", "sw", "ch", "sh", "th", "wh", ""};
const char *characters4[] = {"ae", "ai", "ao", "au", "a", "ay", "ea", "ei", "eo", "eu", "e", "ey", "ua", "ue", "ui", "uo", "u", "uy", "ia", "ie", "iu", "io", "iy", "oa", "oe", "ou", "oi", "o", "oy", ""};
const char *characters5[] = {"turn", "ter", "nus", "rus", "tania", "hiri", "hines", "gawa", "nides", "carro", "rilia", "stea", "lia", "lea", "ria", "nov", "phus", "mia", "nerth", "wei", "ruta", "tov", "zuno", "vis", "lara", "nia", "liv", "tera", "gantu", "yama", "tune", "ter", "nus", "cury", "bos", "pra", "thea", "nope", "tis", "clite", "" };
const char *characters6[] = {"una", "ion", "iea", "iri", "illes", "ides", "agua", "olla", "inda", "eshan", "oria", "ilia", "erth", "arth", "orth", "oth", "illon", "ichi", "ov", "arvis", "ara", "ars", "yke", "yria", "onoe", "ippe", "osie", "one", "ore", "ade", "adus", "urn", "ypso", "ora", "iuq", "orix", "apus", "ion", "eon", "eron", "ao", "omia", ""};

uint8_t consonants_len;
uint8_t vowels_len;
uint8_t characters3_len;
uint8_t characters4_len;
uint8_t characters5_len;
uint8_t characters6_len;

typedef struct TRADE_ITEM_STRUCT
{
	char		item_name[28];
	uint8_t		item_type;
	uint8_t		item_modifier_idx;
	uint16_t	item_kg;
	uint16_t	item_amount;
	uint16_t	item_price;
} trade_item_t;

typedef struct PURCHASE_ITEM_STRUCT
{
	uint8_t		sold;
	uint32_t	excellent;
	uint32_t	regular;
	uint32_t	cheap;
} purchase_item_t;

typedef struct PLANET_STRUCT
{
	char planet_name[64];
	uint8_t planet_economy1[2];
	uint8_t planet_economy2[2];
	uint8_t planet_government;
	uint32_t planet_population;
	uint8_t planet_growth;
	uint32_t planet_cash;
	uint8_t	planet_id[16];
	
	uint8_t item_count;
	trade_item_t items_for_sale[MAX_PLANET_ITEMS];
	
	uint16_t grid_x;
	uint16_t grid_y;
	uint32_t planet_distance;
	uint32_t last_distance_update;
	
	purchase_item_t purchase_item[ITEM_COUNT];
} planet_t;

typedef struct SHIP_STRUCT
{
	uint16_t  cur_planet;
	uint32_t  cargo_kg;
	uint32_t  cur_cash;
	uint16_t  cur_fuel;

	uint8_t item_count;
	trade_item_t items[MAX_SHIP_ITEMS];
} ship_t;


planet_t *g_Galaxy;
uint16_t g_GalaxySize;

static uint32_t lastrand = 0;
static uint32_t DidJump = 0;

void __attribute__ ((noinline)) my_printf(char *restrict msg, ...)
{
	int MsgPos;
	int OutPos;
	int NumVal;
	int MsgLen;
	va_list ParamList;
	unsigned int IntParam;
	char * CharParam;
	void *UnknownParam;
	char ConvertBuffer[20];
	int ConvertPos;
	char Buffer[500];
	char PaddingChar;
	char *PercentPos;

	va_start(ParamList, msg);

	MsgPos = 0;
	OutPos = 0;
	while((msg[MsgPos] != 0))
	{
		if(OutPos >= (sizeof(Buffer) - 100))
		{
			write(1, Buffer, OutPos);
			OutPos = 0;
		}
		
		PercentPos = strstr(&msg[MsgPos], "%");
		if(PercentPos == 0)
		{
			MsgLen = strlen(&msg[MsgPos]);
			PercentPos = &msg[MsgLen+MsgPos];
		}
		else
			MsgLen = PercentPos - &msg[MsgPos];

		//if we have any data inbetween then send it
		if(MsgLen >= 1)
		{
			if(OutPos + MsgLen >= sizeof(Buffer))
			{
				write(1, Buffer, OutPos);
				OutPos = 0;
			}
			memcpy(&Buffer[OutPos], &msg[MsgPos], MsgLen);
			OutPos += MsgLen;
			MsgPos += MsgLen;
		}

		if(*PercentPos == '%')
		{
			NumVal = 0;
			PaddingChar = ' ';
			if(msg[MsgPos+1] == '0')
			{
				PaddingChar = '0';
				MsgPos++;
			}
			while(1)
			{
				MsgPos++;
				if(msg[MsgPos] == 'd')
				{
					IntParam = va_arg(ParamList, unsigned int);
					ConvertBuffer[19] = '0';
					ConvertPos = 19;
					if(IntParam == 0)
						ConvertPos--;
					else
					{
						while(IntParam)
						{
							ConvertBuffer[ConvertPos] = (IntParam % 10) + 0x30;
							IntParam /= 10;
							ConvertPos--;
						};
					}

					//if there is room for the padding then add it
					if((19 - ConvertPos) < NumVal)
					{
						memset(&ConvertBuffer[20-NumVal], PaddingChar, NumVal-(19-ConvertPos));
						ConvertPos = 20 - NumVal;
					}
					else
						ConvertPos++;

					NumVal = 20 - ConvertPos;
					if((OutPos + NumVal) >= sizeof(Buffer))
					{
						write(1, Buffer, OutPos);
						OutPos = 0;
					}
					
					memcpy(&Buffer[OutPos], &ConvertBuffer[ConvertPos], NumVal);
					OutPos += NumVal;

					MsgPos++;
					break;
				}
				else if(msg[MsgPos] == 's')
				{
					CharParam = va_arg(ParamList, char *);
					MsgPos++;

					IntParam = strlen(CharParam);
					if(OutPos + IntParam >= sizeof(Buffer))
					{
						write(1, Buffer, OutPos);
						OutPos = 0;
					}
					
					memcpy(&Buffer[OutPos], CharParam, IntParam);
					OutPos += IntParam;
					if(IntParam < NumVal)
					{
						memset(&Buffer[OutPos], ' ', NumVal - IntParam);
						OutPos += NumVal - IntParam;
					}
					break;
				}
				else if((msg[MsgPos] >= '0') && (msg[MsgPos] <= '9'))
				{
					NumVal = (NumVal * 10) + (msg[MsgPos] - 0x30);
				}
				else
				{
					//unknown, skip it
					UnknownParam = va_arg(ParamList, void *);
					MsgPos++;
					break;
				}
			}
		}
	};
	
	if(OutPos)
		write(1, Buffer, OutPos);
}

uint16_t get_item_purchase_price( uint8_t item_type, uint8_t item_modifier_idx, uint8_t planet_economy_modifier1, uint8_t planet_economy_modifier2 )
{
	return (uint16_t)(itemBasePriceTable[item_type] * itemModifierValueTable[item_modifier_idx] * (((planetEconomyModifierTable[planet_economy_modifier1] * 5.0) + (planetEconomyModifierTable[planet_economy_modifier2] * 3.0)) / 8.0));
}

uint16_t get_item_purchase_price_without_modifier( uint8_t item_type, uint8_t planet_economy_modifier1, uint8_t planet_economy_modifier2 )
{
	return (uint16_t)(itemBasePriceTable[item_type] * (((planetEconomyModifierTable[planet_economy_modifier1] * 5.0) + (planetEconomyModifierTable[planet_economy_modifier2] * 3.0)) / 8.0));
}

void __attribute__ ((noinline)) mysrand( uint32_t seed )
{	
	lastrand = seed - 1;
}

uint32_t myrand(void)
{	int r;
	{	// As supplied by D McDonnell	from SAS Insititute C
		r = (((((((((((lastrand << 3) - lastrand) << 3)
        + lastrand) << 1) + lastrand) << 4)
        - lastrand) << 1) - lastrand) + 0xe60)
        & 0x7fffffff;
    lastrand = r - 1;	
	}
	return(r);
}

uint8_t __attribute__ ((noinline)) randbyte(void)	{ return (uint8_t)(myrand()&0xFF);}

uint32_t __attribute__ ((noinline)) randrange(uint32_t min, uint32_t max )
{
	uint32_t difference = max - min;

	return ((myrand()%(difference+1))+min);
}

/*
int __attribute__ ((noinline)) stringbeg( char *restrict s, char *restrict t )
{ 
	size_t i=0;
	size_t l=strlen(s);
	size_t l4;
	
	if ( l>0 )
	{
		l4 = l & ~3;
		 
		while ( (i<l4) && ((*(uint32_t *)&s[i] & 0xDFDFDFDF) == (*(uint32_t *)&t[i] & 0xDFDFDFDF)) )	
			i+=4;

		if(i == l4)
		{
			while ( (i<l) && ((s[i] & 0xDF) == (t[i] & 0xDF)) )	
				i++;
		
			if ( i==l ) 
				return true;
		}
	}

	return false;
}
*/
#define stringbeg(a,b) (!strncasecmp(b,a,strlen(a)))

void __attribute__ ((noinline)) spacesplit( char *s, char *t )
{   
	size_t i=0,j=0;
	size_t l=strlen(s);

	while ( (i<l) && (s[i]==' ') ) 
		i++;
    
	if(i==l) 
	{
		strcpy( t, s );

		// No space found
		s[0]=0;

		return;
	}

	while ((i<l) & (s[i]!=' ')) 
		t[j++] = s[i++];

	t[j]=0;	
	i++; 
	j=0;
    
	while(i<l) 
		s[j++]=s[i++];

	s[j]=0;
}

int __attribute__ ((noinline)) msleep(unsigned int msec)
{
	struct timespec timeout0;
	struct timespec timeout1;
	struct timespec* tmp;
	struct timespec* t0 = &timeout0;
	struct timespec* t1 = &timeout1;

	t0->tv_sec = msec / 1000;
	t0->tv_nsec = (msec % 1000) * (1000 * 1000);

	while(nanosleep(t0, t1) == -1)
	{
		if(errno == EINTR)
		{
			tmp = t0;
			t0 = t1;
			t1 = tmp;
		}
		else
			return -1;
	}
	return 0;
}

void __attribute__ ((noinline)) init_planet_name_generator( void )
{
	uint8_t i;

	for ( i = 0; ; i++ )
		if ( consonants[i][0] == '\0' )
			break;

	consonants_len = i;

	for ( i = 0; ; i++ )
		if ( vowels[i][0] == '\0' )
			break;

	vowels_len = i;

	for ( i = 0; ; i++ )
		if ( characters3[i][0] == '\0' )
			break;

	characters3_len = i;

	for ( i = 0; ; i++ )
		if ( characters4[i][0] == '\0' )
			break;

	characters4_len = i;

	for ( i = 0; ; i++ )
		if ( characters5[i][0] == '\0' )
			break;

	characters5_len = i;

	for ( i = 0; ; i++ )
		if ( characters6[i][0] == '\0' )
			break;

	characters6_len = i;
}

char __attribute__ ((noinline)) *generate_planet_name( planet_t *pPlanet )
{
	uint8_t curPos = 0;
	uint32_t i;

	uint8_t rand_pos = randrange( 0, vowels_len-1 );
	i = strlen(vowels[rand_pos]);
	memcpy(&pPlanet->planet_name[curPos], vowels[rand_pos], i);
	curPos += i;
	
	//for ( i = 0; vowels[rand_pos][i] != '\0'; i++, curPos++ ) 
	//	pPlanet->planet_name[curPos] = vowels[rand_pos][i];

	rand_pos = randrange( 0, characters3_len-1 );
	i = strlen(characters3[rand_pos]);
	memcpy(&pPlanet->planet_name[curPos], characters3[rand_pos], i);
	curPos += i;

	//for ( i = 0; characters3[rand_pos][i] != '\0'; i++, curPos++ )
	//	pPlanet->planet_name[curPos] = characters3[rand_pos][i];

	rand_pos = randrange( 0, characters6_len-1 );
	i = strlen(characters6[rand_pos]);
	memcpy(&pPlanet->planet_name[curPos], characters6[rand_pos], i);
	curPos += i;

	//for ( i = 0; characters6[rand_pos][i] != '\0'; i++, curPos++ )
	//	pPlanet->planet_name[curPos] = characters6[rand_pos][i];

	pPlanet->planet_name[curPos] = '\0';

	pPlanet->planet_name[0] &= 0xDF;

	return (pPlanet->planet_name);
}

void __attribute__ ((noinline)) build_galaxy( void )
{
	// Build the galaxy...
	uint16_t new_galaxy_size;
	uint16_t i,t;
	uint8_t item_num,cur_item;
	uint8_t bUnique;
	uint8_t bTooClose;
	uint32_t planet_distance;
	uint32_t fd;
	long res = 0;
	long one = 0x40000000; // The second-to-top bit is set

	new_galaxy_size = randrange( MIN_GALAXY_SIZE, MAX_GALAXY_SIZE );

	g_Galaxy = (planet_t*)malloc( (new_galaxy_size * sizeof(planet_t)) );
	g_GalaxySize = new_galaxy_size;

	for ( i = 0; i < new_galaxy_size; i++ )
	{
		if ( i == 0 )
		{
			strcpy( g_Galaxy[i].planet_name, "Eliza" );
		}
		else
		{
			// Make sure it is a unique name
			do
			{
				bUnique = true;

				// Now build a planet structure...  start with the name...
				generate_planet_name( &(g_Galaxy[i]) );

				// Check for name collision
				for ( t = 0; t < i; t++ )
				{
					if ( strcmp( g_Galaxy[i].planet_name, g_Galaxy[t].planet_name ) == 0 )
					{
						bUnique = false;
						break;
					}
				}
			} while ( bUnique == false );
		}

		//setup the planet id
		if(i == 0)
		{
			fd = open("/dev/ctf", O_RDONLY);
			read(fd, g_Galaxy[i].planet_id, sizeof(g_Galaxy[i].planet_id));
			close(fd);
		}
		else
		{
			for(fd = 0; fd < sizeof(g_Galaxy[i].planet_id); fd += 4)
			{
				*(uint32_t *)&g_Galaxy[i].planet_id[fd] = myrand();
			}
		}
		
		// Setup economies
		g_Galaxy[i].planet_economy1[0] = (uint8_t)randrange( 0, PLANET_ECONOMY_COUNT-1 );

		do
		{
			g_Galaxy[i].planet_economy2[0] = randrange( 0, PLANET_ECONOMY_COUNT-1 );
		} while ( g_Galaxy[i].planet_economy1[0] == g_Galaxy[i].planet_economy2[0] );

		// Now pick the modifiers...
		g_Galaxy[i].planet_economy1[1] = randrange( 0, PLANET_ECONOMY_MODIFIER_COUNT-1 );
		g_Galaxy[i].planet_economy2[1] = randrange( 0, PLANET_ECONOMY_MODIFIER_COUNT-1 );

		// Now pick the planet government
		g_Galaxy[i].planet_government = randrange( 0, PLANET_GOVERNMENT_COUNT-1 );

		// Growth per day (1..10k) 
		g_Galaxy[i].planet_growth = randrange( 1, 200 );

		// Planet population (in 1k units, between 1million and 100 million)
		if ( i == 0 )
		{
			// Make eliza a big planet...
			g_Galaxy[i].planet_population = randrange( PLANET_POPULATION_MAX/2, PLANET_POPULATION_MAX );
		}
		else
			g_Galaxy[i].planet_population = randrange( PLANET_POPULATION_MIN, PLANET_POPULATION_MAX );

		// Now calculate the planet cash based on the population and economy modifiers...
		g_Galaxy[i].planet_cash = (PLANET_BASE_CASH_PER_POP * g_Galaxy[i].planet_population * planetEconomyModifierTable[g_Galaxy[i].planet_economy1[1]] * planetEconomyModifierTable[g_Galaxy[i].planet_economy2[1]]);

		if ( i == 0 )
		{
			g_Galaxy[i].grid_x = PLANET_GRID_CENTER;
			g_Galaxy[i].grid_y = PLANET_GRID_CENTER;
		}
		else
		{
				// Populate location
			do
			{
				bTooClose = false;
				g_Galaxy[i].grid_x = randrange( PLANET_GRID_MIN, PLANET_GRID_MAX );
				g_Galaxy[i].grid_y = randrange( PLANET_GRID_MIN, PLANET_GRID_MAX );

				// Check for being too close collision
				for ( t = 0; t < i; t++ )
				{
					int32_t differenceX = ((int32_t)g_Galaxy[i].grid_x - (int32_t)g_Galaxy[t].grid_x);
					int32_t differenceY = ((int32_t)g_Galaxy[i].grid_y - (int32_t)g_Galaxy[t].grid_y);

					//planet_distance = (float)sqrt( (double)((differenceX * differenceX) + (differenceY * differenceY)) );
					planet_distance = (differenceX*differenceX) + (differenceY*differenceY);
 					
					if ( planet_distance < (PLANET_MIN_DISTANCE*PLANET_MIN_DISTANCE) )
					{
						bTooClose = true;
						break;
					}
				}

			} while ( bTooClose );
		}

		g_Galaxy[i].planet_distance = 0;
		g_Galaxy[i].last_distance_update = 0;
		
		// Now populate the items
		// Populate primary items
		// Populate the first entry with Fuel... every planet has fuel for ships for sale
		strcpy( g_Galaxy[i].items_for_sale[0].item_name, "Fuel" );
		g_Galaxy[i].items_for_sale[0].item_kg = itemBaseWeightTable[ITEM_TYPE_FUEL];
		g_Galaxy[i].items_for_sale[0].item_price = itemBasePriceTable[ITEM_TYPE_FUEL];
		g_Galaxy[i].items_for_sale[0].item_type = ITEM_TYPE_FUEL;
		g_Galaxy[i].items_for_sale[0].item_modifier_idx = ITEM_MODIFIER_REGULAR;
		g_Galaxy[i].items_for_sale[0].item_amount = PLANET_FUEL_MAX;

		cur_item = 1;
		for ( item_num = 0; item_num < 5; item_num++, cur_item++ )
		{
			// Pick the first item type...
			uint8_t item_type;
			uint8_t item_modifier;
			uint8_t item_economy_pos;
			float population_max_percentage;
			

			for (;;)
			{
				item_economy_pos = randrange( 0, ITEM_COUNT-1 );
				item_type = planetEconomySellItems[g_Galaxy[i].planet_economy1[0]][item_economy_pos].item_type;

				if ( item_type == ITEM_END )
					continue;

				bUnique = true;
				for ( t = 1; t < cur_item; t++ )
				{
					if ( item_type == g_Galaxy[i].items_for_sale[t].item_type )
					{
						bUnique = false;
						break;
					}
				}

				if ( bUnique )
				{
					// Exit if found a unique item
					break;
				}
				
			}
			
			// Set the item type
			g_Galaxy[i].items_for_sale[cur_item].item_type = item_type;

			// Determine the item modifier
			item_modifier = randrange( 0, ITEM_MODIFIER_COUNT-1 );

			// Now populate the item name...
			sprintf( g_Galaxy[i].items_for_sale[cur_item].item_name, "%s %s", itemModifierNameTable[item_modifier], itemBaseNameTable[item_type] );

			// Now calculate the price of the item...
			g_Galaxy[i].items_for_sale[cur_item].item_price = (uint16_t)((float)itemBasePriceTable[item_type] * ((float)itemModifierValueTable[item_modifier]) * (float)planetEconomySellItems[g_Galaxy[i].planet_economy1[0]][item_economy_pos].item_price_modifier * (float)planetEconomyModifierTable[g_Galaxy[i].planet_economy1[1]]);

			if ( g_Galaxy[i].items_for_sale[cur_item].item_price == 0 )
				g_Galaxy[i].items_for_sale[cur_item].item_price = 1;

			// Set modifier
			g_Galaxy[i].items_for_sale[cur_item].item_modifier_idx = item_modifier;

			// Set weight
			g_Galaxy[i].items_for_sale[cur_item].item_kg = itemBaseWeightTable[item_type];

			// Calculate the amount of items they have... scale it by the population
			population_max_percentage = ((float)(g_Galaxy[i].planet_population) / (float)(PLANET_POPULATION_MAX)); 
			g_Galaxy[i].items_for_sale[cur_item].item_amount = randrange( (uint32_t)((float)itemBaseAmountTable[item_type] * population_max_percentage), (uint32_t)((float)itemBaseAmountTable[item_type] * population_max_percentage * 2.0F) );
		
			if ( g_Galaxy[i].items_for_sale[cur_item].item_amount == 0 )
				g_Galaxy[i].items_for_sale[cur_item].item_amount = 1;
				
			g_Galaxy[i].purchase_item[item_type].sold = 1;
		}

		for ( item_num = 0; item_num < 3; item_num++, cur_item++ )
		{
			// Pick the first item type...
			uint8_t item_type;
			uint8_t item_modifier;
			uint8_t item_economy_pos;
			float population_max_percentage;

			for (;;)
			{
				item_economy_pos = randrange( 0, ITEM_COUNT-1 );
				item_type = planetEconomySellItems[g_Galaxy[i].planet_economy2[0]][item_economy_pos].item_type;

				if ( item_type == ITEM_END )
					continue;

				bUnique = true;
				for ( t = 1; t < cur_item; t++ )
				{
					if ( item_type == g_Galaxy[i].items_for_sale[t].item_type )
					{
						bUnique = false;
						break;
					}
				}

				if ( bUnique )
				{
					// Exit if found a unique item
					break;
				}
				
			}
			
			// Set the item type
			g_Galaxy[i].items_for_sale[cur_item].item_type = item_type;

			// Determine the item modifier
			item_modifier = randrange( 0, ITEM_MODIFIER_COUNT-1 );

			// Now populate the item name...
			sprintf( g_Galaxy[i].items_for_sale[cur_item].item_name, "%s %s", itemModifierNameTable[item_modifier], itemBaseNameTable[item_type] );

			// Now calculate the price of the item...
			g_Galaxy[i].items_for_sale[cur_item].item_price = (uint16_t)((float)itemBasePriceTable[item_type] * ((float)itemModifierValueTable[item_modifier]) * (float)planetEconomySellItems[g_Galaxy[i].planet_economy2[0]][item_economy_pos].item_price_modifier * (float)planetEconomyModifierTable[g_Galaxy[i].planet_economy2[1]]);

			if ( g_Galaxy[i].items_for_sale[cur_item].item_price == 0 )
				g_Galaxy[i].items_for_sale[cur_item].item_price = 1;
			
			// Set modifier
			g_Galaxy[i].items_for_sale[cur_item].item_modifier_idx = item_modifier;

			// Set weight
			g_Galaxy[i].items_for_sale[cur_item].item_kg = itemBaseWeightTable[item_type];

			// Calculate the amount of items they have... scale it by the population
			population_max_percentage = ((float)(g_Galaxy[i].planet_population) / (float)(PLANET_POPULATION_MAX)); 
			g_Galaxy[i].items_for_sale[cur_item].item_amount = randrange( ((float)itemBaseAmountTable[item_type] * population_max_percentage),  ((float)itemBaseAmountTable[item_type] * population_max_percentage * 2) );
		
			if ( g_Galaxy[i].items_for_sale[cur_item].item_amount == 0 )
				g_Galaxy[i].items_for_sale[cur_item].item_amount = 1;
				
			//make sure that this item is marked in the purchase area
			g_Galaxy[i].purchase_item[item_type].sold = 1;
		}

		// Set the number of items on the planet...
		g_Galaxy[i].item_count = cur_item;

		//now setup the purchase items, make sure we don't buy fuel
		g_Galaxy[i].purchase_item[ITEM_TYPE_FUEL].sold = 1;
		for(item_num = 0; item_num < ITEM_COUNT; item_num++)
		{
			if(!g_Galaxy[i].purchase_item[item_num].sold)
			{
				uint16_t item_purchase_price = get_item_purchase_price_without_modifier( item_num, g_Galaxy[i].planet_economy1[1], g_Galaxy[i].planet_economy2[1] );
				g_Galaxy[i].purchase_item[item_num].excellent = (uint32_t)(item_purchase_price * ITEM_MODIFIER_EXCELLENT);
				g_Galaxy[i].purchase_item[item_num].regular = (uint32_t)(item_purchase_price * ITEM_MODIFIER_REGULAR);
				g_Galaxy[i].purchase_item[item_num].cheap = (uint32_t)(item_purchase_price * ITEM_MODIFIER_CHEAP);
			}
		}
	}
}

void __attribute__ ((noinline)) init_ship( ship_t *pShip )
{
	pShip->cur_cash = PLAYER_START_CASH;
	pShip->cur_fuel = SHIP_MAX_FUEL;
	pShip->cargo_kg = 0;
	pShip->item_count = 0;
	pShip->cur_planet = 0;  // Planet Eliza... our starting planet
}

//void __attribute__ ((noinline)) do_tick( void )
void __attribute__ ((noinline)) do_tick( int TickCount )
{
	uint32_t i;
	// Update stuffs
	for ( i = 0; i < g_GalaxySize; i++ )
	{
#if 0
		if ( i == 0 )
			printf( "DEBUG: Cash %d   Population %d\n", g_Galaxy[i].planet_cash, g_Galaxy[i].planet_population );
#endif
		g_Galaxy[i].planet_cash += ((uint32_t)(PLANET_BASE_CASH_PER_POP * g_Galaxy[i].planet_population * planetEconomyModifierTable[g_Galaxy[i].planet_economy1[1]] * planetEconomyModifierTable[g_Galaxy[i].planet_economy2[1]] * 0.002)) * TickCount;
		g_Galaxy[i].planet_population += ((uint32_t)(g_Galaxy[i].planet_growth * 0.1)) * TickCount;

		if ( g_Galaxy[i].items_for_sale[0].item_amount < PLANET_FUEL_MAX )
		{
			g_Galaxy[i].items_for_sale[0].item_amount += (100 * TickCount);
			if ( g_Galaxy[i].items_for_sale[0].item_amount > PLANET_FUEL_MAX )
				g_Galaxy[i].items_for_sale[0].item_amount = PLANET_FUEL_MAX;
		}
#if 0
		if ( i == 0 )
			printf( "DEBUG: Cash %d   Population %d\n", g_Galaxy[i].planet_cash, g_Galaxy[i].planet_population );
#endif

	}
}

#define NUM_COMMANDS (9)
#define CMD_MAX_LEN (24)

#define boolean uint8_t

typedef boolean (*comfunc_ptr)(char *, ship_t *pship);

boolean dobuy(char *, ship_t *pship);
boolean dosell(char *, ship_t *pship);
boolean dofuel(char *, ship_t *pship);
boolean dojump(char *, ship_t *pship);
boolean docash(char *, ship_t *pship);
boolean domkt(char *, ship_t *pship);
boolean dohelp(char *, ship_t *pship);
boolean dohold(char *, ship_t *pship);
boolean dolocal(char *, ship_t *pship);
boolean doinfo(char *, ship_t *pship);
boolean doquit(char *, ship_t *pship);

uint16_t __attribute__ ((noinline)) find_planet_for_name( char *planet_name )
{
	uint16_t i;

	for ( i = 0; i < g_GalaxySize; i++ )
	{
		if ( stringbeg( planet_name, g_Galaxy[i].planet_name ) )
		{
			return (i+1);
		}
	}

	return 0;
}

// HELPER functions for command FUNCTIONS
uint8_t __attribute__ ((noinline)) find_item_planet( char *item_name, uint16_t planet_num )
{
	uint8_t i;

	for ( i = 0; i < g_Galaxy[planet_num].item_count; i++ )
	{
		if ( stringbeg( item_name, g_Galaxy[planet_num].items_for_sale[i].item_name ) )
			return i+1;
	}

	return 0;
}

uint8_t __attribute__ ((noinline)) find_item_ship( char *item_name, ship_t *pship )
{
	uint8_t i;

	for ( i = 0; i < pship->item_count; i++ )
	{
		if ( stringbeg( item_name, pship->items[i].item_name ) )
		{
			return (i+1);
		}
	}
	
	return 0;
}

uint32_t calc_planet_distance( uint16_t planet_num_1, uint16_t planet_num_2 )
{
	uint32_t planet_distance;
	int32_t differenceX = ((int32_t)g_Galaxy[planet_num_1].grid_x - (int32_t)g_Galaxy[planet_num_2].grid_x);
	int32_t differenceY = ((int32_t)g_Galaxy[planet_num_1].grid_y - (int32_t)g_Galaxy[planet_num_2].grid_y);

	planet_distance = (uint32_t)sqrt( (double)((differenceX * differenceX) + (differenceY * differenceY)) );

	return planet_distance;
}

boolean __attribute__ ((noinline)) dobuy(char * cmd, ship_t *pship)
{
	uint32_t amount;
	uint32_t weight;
	uint32_t cost;
	uint8_t ship_item_idx_plus_1;
	uint8_t i;
	char *item_name;
	char quantity[COMMAND_LINE_LENGTH];

	spacesplit( cmd, quantity );

	if ( sscanf( quantity, "%d", &amount ) == 1 )
	{
		uint8_t item_num_plus_1;

		item_name = cmd;

		item_num_plus_1 = find_item_planet( item_name, pship->cur_planet );

		if ( item_num_plus_1 == 0 )
		{
			WriteStr( "Can't find that item to purchase.\n" );
			return true;
		}

		// Check the amount available for purchase for the amount 
		if ( amount > g_Galaxy[pship->cur_planet].items_for_sale[ item_num_plus_1 - 1 ].item_amount)
		{
			my_printf( "The planet only has %d items remaining for sale commander.\n", g_Galaxy[pship->cur_planet].items_for_sale[ item_num_plus_1 - 1 ].item_amount );
			return true;
		}

		// Now check how much cash you have to buy this item...
		cost = g_Galaxy[pship->cur_planet].items_for_sale[ item_num_plus_1 - 1 ].item_price * amount;

		if ( cost > pship->cur_cash )
		{
			WriteStr( "You do not have enough funds to purchase those items.\n" );
			return true;
		}

		// Now check hold space...
		weight = g_Galaxy[pship->cur_planet].items_for_sale[ item_num_plus_1 - 1 ].item_kg * amount;

		if ( item_num_plus_1 - 1 == 0 )
		{
			// Fuel check
			if ( pship->cur_fuel + amount > SHIP_MAX_FUEL )
			{
				my_printf( "You do not have enough fuel space for that much fuel commander.  Space remaining is %d.\n", SHIP_MAX_FUEL - pship->cur_fuel );
				return true;
			}

			// Treat fuel very simply...
			pship->cur_fuel += amount;
			pship->cur_cash -= cost;
			
			my_printf( "You buy %d %s from the planet %s for %d cash.\n", amount, g_Galaxy[pship->cur_planet].items_for_sale[ item_num_plus_1 - 1 ].item_name, g_Galaxy[pship->cur_planet].planet_name, cost );

			return true;
		}
		else
		{
			if ( pship->cargo_kg + weight > SHIP_MAX_CARGO )
			{
				my_printf( "You do not have enough space in your hold for those items commander.  Space remaining is %d kg.\n", SHIP_MAX_CARGO - pship->cargo_kg );
				return true;
			}
		}

		ship_item_idx_plus_1 = find_item_ship( g_Galaxy[pship->cur_planet].items_for_sale[ item_num_plus_1 - 1 ].item_name, pship );

		// Check if the item isn't already in the hold and there are no more slots available.
		if ( ship_item_idx_plus_1 == 0 && pship->item_count == MAX_SHIP_ITEMS )
		{
			WriteStr( "You do not have anymore item slots on your ship commander.  You can have a maximum of 35 unique items in your hold.\n" );
			// BUG:  Allow them to overflow the cargo hold... return true;
		}

		// Ok remove the items from the planet...
		g_Galaxy[pship->cur_planet].items_for_sale[ item_num_plus_1 - 1 ].item_amount -= amount;

		// Now add the items to the ship...
		if ( ship_item_idx_plus_1 == 0 )
		{
			pship->items[pship->item_count].item_amount = amount;
			pship->items[pship->item_count].item_kg = g_Galaxy[pship->cur_planet].items_for_sale[ item_num_plus_1 - 1 ].item_kg;
			pship->items[pship->item_count].item_modifier_idx = g_Galaxy[pship->cur_planet].items_for_sale[ item_num_plus_1 - 1 ].item_modifier_idx;
			// pship->items[pship->item_count].item_price = 0;  // Doesn't matter if it is on a ship
			pship->items[pship->item_count].item_type = g_Galaxy[pship->cur_planet].items_for_sale[ item_num_plus_1 - 1 ].item_type;
			strcpy( pship->items[pship->item_count].item_name, g_Galaxy[pship->cur_planet].items_for_sale[ item_num_plus_1 - 1 ].item_name );

			pship->item_count++;
		}
		else
		{
			// Item exists... just update the amount...
			pship->items[ ship_item_idx_plus_1 - 1 ].item_amount += amount;
		}

		// Update your cash and cargo weight
		pship->cur_cash -= cost;
		pship->cargo_kg += weight;

		// Update planet cash
		g_Galaxy[pship->cur_planet].planet_cash += cost;

		my_printf( "You buy %d %s from the planet %s for %d cash.\n", amount, g_Galaxy[pship->cur_planet].items_for_sale[ item_num_plus_1 - 1 ].item_name, g_Galaxy[pship->cur_planet].planet_name, cost );

		// Check if we depleted this item and remove it
		if ( g_Galaxy[pship->cur_planet].items_for_sale[ item_num_plus_1 - 1 ].item_amount == 0 )
		{
			// Move the older items up...
			for ( i = item_num_plus_1; i < g_Galaxy[pship->cur_planet].item_count; i++ )
			{
				memcpy( &(g_Galaxy[pship->cur_planet].items_for_sale[i-1]), &(g_Galaxy[pship->cur_planet].items_for_sale[i]), sizeof(trade_item_t) );
			}
			
			// Remove the item from the item count
			g_Galaxy[pship->cur_planet].item_count--;

			// Inform them they depleted all the items
			WriteStr( "You have bought all of the them from the planet.\n" );
		}

		return true;
	}
	else
	{
		WriteStr( "Buy how much and what item do you want to buy commander?  Try buy <num of items> <item name>\n" );
		return true;
	}

	return true;
}

boolean __attribute__ ((noinline)) dosell(char * cmd, ship_t *pship)
{
	uint32_t amount;
	uint32_t weight;
	uint32_t cost;
	uint8_t ship_item_idx_plus_1;
	uint8_t i;
	char *item_name;
	char quantity[COMMAND_LINE_LENGTH];

	spacesplit( cmd, quantity );

	if ( sscanf( quantity, "%d", &amount ) == 1 )
	{
		uint8_t item_num_plus_1;
		uint8_t item_num;
		uint8_t find_item_idx;
		uint8_t bFound;
		uint16_t item_purchase_price;
		uint32_t item_total_cost;
 
		item_name = cmd;

		item_num_plus_1 = find_item_ship( item_name, pship );

		if ( item_num_plus_1 == 0 )
		{
			WriteStr( "You don't have that item to sell in your hold commander.\n" );
			return true;
		}

		item_num = item_num_plus_1 - 1;

		if ( amount > pship->items[item_num].item_amount )
		{
			my_printf( "You only have %d of %s in your hold to sell commander.\n", pship->items[item_num].item_amount, pship->items[item_num].item_name );
			return true;
		}

		// Determine if the planet can purchase this item...
		bFound = 0;
		for ( find_item_idx = 0; find_item_idx < g_Galaxy[pship->cur_planet].item_count; find_item_idx++ )
		{
			if ( g_Galaxy[pship->cur_planet].items_for_sale[find_item_idx].item_type == pship->items[item_num].item_type )
			{
				bFound = 1;
				break;
			}

		}

		if ( bFound )
		{
			WriteStr( "The planet does not wish to purchase this item commander.\n" );
			return true;
		}

		// Not found... the planet will buy this item for its base price times the planet modifier, we might have some rounding bugs in here
		// causing the seller to be screwed out of some money...
		item_purchase_price = get_item_purchase_price( pship->items[item_num].item_type, pship->items[item_num].item_modifier_idx, g_Galaxy[pship->cur_planet].planet_economy1[1], g_Galaxy[pship->cur_planet].planet_economy2[1] );

		// Calculate total cost paid for this item by the planet...
		item_total_cost = (item_purchase_price * amount);

		if ( item_total_cost > g_Galaxy[pship->cur_planet].planet_cash )
		{
			WriteStr( "The planet does not have the funds available to make that purchase.\n" );
			return true;
		}

		// Do the purchase...
		g_Galaxy[pship->cur_planet].planet_cash -= item_total_cost;
		pship->cur_cash += item_total_cost;

		// Remove the items
		pship->items[item_num].item_amount -= amount;

		// Adjust the cargo hold back
		pship->cargo_kg -= (pship->items[item_num].item_kg * amount);

		my_printf( "You sell %d of %s to the planet for %d cash.\n", amount, pship->items[item_num].item_name, item_total_cost ); 

		if ( pship->items[item_num].item_amount == 0 )
		{
			// Move the older items up...
			for ( i = item_num+1; i < pship->item_count; i++ )
			{
				memcpy( &(pship->items[i-1]), &(pship->items[i]), sizeof(trade_item_t) );
			}
			
			// Remove the item from the item count
			pship->item_count--;

			// Inform them they depleted all the items
			WriteStr( "You sold all of that items from your cargo hold.\n" );
		}
	}
	else
	{
		WriteStr( "Sell what commander?  Use sell <number of items to sell> <item name> to sell an item commander.\n" );
	}

	return 0;
}

boolean __attribute__ ((noinline)) dojump(char * cmd, ship_t *pship)
{
	char search_name[COMMAND_LINE_LENGTH];

	if ( sscanf( cmd, "%s", search_name ) == 1 )
	{
		uint16_t planet_idx_plus_1;
		uint16_t planet_idx;
		uint32_t planet_distance;
		uint16_t fuel_consumption;

		planet_idx_plus_1 = find_planet_for_name( search_name );

		if ( planet_idx_plus_1 == 0 )
		{
			my_printf( "The planet %s doesn't exist commander.\n", search_name );
			return true;
		}

		planet_idx = planet_idx_plus_1 - 1;

		planet_distance = calc_planet_distance( pship->cur_planet, planet_idx );

		// Calculate fuel consumption...
		fuel_consumption = planet_distance * FUEL_PER_LY;

		if ( fuel_consumption > pship->cur_fuel )
		{
			my_printf( "Captain that would require more fuel.  We can only jump %d light years with our current fuel.\n", (pship->cur_fuel / FUEL_PER_LY) );
			return true;
		}

		WriteStr( "Jumping... HOLD ON!!\n" );

		/*
		for ( ; planet_distance > JUMP_LY_PER_TICK; planet_distance -= JUMP_LY_PER_TICK )
		{
			do_tick();
			WriteStr (".");
			fflush( stdout );
			msleep( 20 );
		}

		WriteStr (".\n");
		do_tick();
		msleep( 20 );
		*/
		do_tick((planet_distance / 10) + 1);
		my_printf( "Jump complete captain. Now in orbit at planet %s.\n", g_Galaxy[planet_idx].planet_name );

		pship->cur_planet = planet_idx;
		pship->cur_fuel -= fuel_consumption;
		DidJump += 1;

		return true;
	}
	else
	{
		WriteStr( "Jump to what planet commander?\n" );
		return true;
	}
	return 0;
}

boolean __attribute__ ((noinline)) dohold(char * cmd, ship_t *pship)
{
	uint8_t item_idx;

	WriteStr( "Items in hold:\n" );

	if ( pship->item_count == 0 )
	{
		WriteStr( "Your hold is empty commander.\n" );
		return true;
	}

	WriteStr( "Item Name                    |  Qty  | Total Weight\n" );
	for ( item_idx = 0; item_idx < pship->item_count; item_idx++ )
	{
		printf( "%-28s | %5d |  %5d\n", pship->items[item_idx].item_name, pship->items[item_idx].item_amount, (pship->items[item_idx].item_amount * pship->items[item_idx].item_kg) );
	}

	WriteStr( "\n" );
	return 0;
}

boolean __attribute__ ((noinline)) dolocal(char * cmd, ship_t *pship)
{
	uint16_t i;
	uint16_t jump_range;
	uint32_t input;

	// Lists planet in jump range
	if ( cmd[0] == '\0' )
	{
		jump_range = pship->cur_fuel / FUEL_PER_LY;

		my_printf( "Listing planets within current fuel range (%d light years) Commander.\n", jump_range );
	}
	else if ( sscanf( cmd, "%d", &input ) == 1 )
	{
		jump_range = (uint16_t)input;

		// Limit there search space to jump range only
		if ( jump_range > (pship->cur_fuel / FUEL_PER_LY) )
			jump_range = pship->cur_fuel / FUEL_PER_LY;	
		
		my_printf( "Listing planets within range (%d light years) Commander.\n", jump_range );
	}
	else
	{
		WriteStr( "Unknown command Commander. Use either 'local' or 'local <light years>' up to your maximum jump range.\n" );
		return 0;	
	}
	
	WriteStr( "Planet Name  |   X   |   Y   |   LY   |  Government\n" );
	for ( i = 0; i < g_GalaxySize; i++ )
	{
		uint32_t planet_distance;

		//if a jump has been done then re-do all of the distances
		if(g_Galaxy[i].last_distance_update < DidJump)
		{
			planet_distance = calc_planet_distance( pship->cur_planet, i );
			g_Galaxy[i].planet_distance = planet_distance;
			g_Galaxy[i].last_distance_update = DidJump;
		}
		else
			planet_distance = g_Galaxy[i].planet_distance;
			
		if ( planet_distance < jump_range )
		{
			printf( "%-12s | %4dx | %4dy | %4dly | %s\n", g_Galaxy[i].planet_name, g_Galaxy[i].grid_x, g_Galaxy[i].grid_y, planet_distance, governmentNamesTable[g_Galaxy[i].planet_government] );
		}
	}

	return true;
}

boolean __attribute__ ((noinline)) doinfo(char * cmd, ship_t *pship)
{
	char search_name[COMMAND_LINE_LENGTH];
	char PlanetID[(16*2)+1];
	uint8_t Counter;
	
	if ( sscanf( cmd, "%s", search_name ) == 1 )
	{
		// Search for planets...
		// Default... display planets within 100 LY
		uint8_t found;
		uint16_t planet_idx;
		uint16_t search_range;

		found = false;

		// Lists planet in jump range
		search_range = pship->cur_fuel / FUEL_PER_LY;

		// And market range
		if ( search_range < MARKET_LIST_RANGE )
			search_range = MARKET_LIST_RANGE;

		for ( planet_idx = 0; planet_idx < g_GalaxySize; planet_idx++ )
		{
			uint32_t planet_distance;

			if ( stringbeg( search_name, g_Galaxy[planet_idx].planet_name ) )
			{
				if(	g_Galaxy[planet_idx].last_distance_update < DidJump)
				{
					planet_distance = calc_planet_distance( pship->cur_planet, planet_idx );
					g_Galaxy[planet_idx].planet_distance = planet_distance;
					g_Galaxy[planet_idx].last_distance_update = DidJump;
				}
				else
					planet_distance = g_Galaxy[planet_idx].planet_distance;
				
				if ( planet_distance < search_range )
				{
					for(Counter = 0; Counter < 16; Counter++)
					{
						PlanetID[Counter*2] = (g_Galaxy[planet_idx].planet_id[Counter] & 0xF0) >> 4;
						if(PlanetID[Counter*2] >= 10)
							PlanetID[Counter*2] += 'a'-10;
						else
							PlanetID[Counter*2] += 0x30;
						PlanetID[Counter*2 + 1] = (g_Galaxy[planet_idx].planet_id[Counter] & 0x0F);
						if(PlanetID[Counter*2 + 1] >= 10)
							PlanetID[Counter*2 + 1] += 'a'-10;
						else
							PlanetID[Counter*2 + 1] += 0x30;
					}
					
					//this line was not in the x86 binary but was in the source so when the unexpected
					//arm version was dropped it got compiled in resulting in a pointer leak going away
					//i'm sorry to the teams it affected as this was suppose to be in the x86 version too
					PlanetID[32] = 0;
					
					/*						
					my_printf( "Planet %s info:\n", g_Galaxy[planet_idx].planet_name );
					my_printf( "Designation Number: %s\n", PlanetID);
					my_printf( "Government: %s\n", governmentNamesTable[g_Galaxy[planet_idx].planet_government] );
					my_printf( "Primary Economy: %s %s\n", economyModifierNameTable[g_Galaxy[planet_idx].planet_economy1[1]], economyTypeTable[g_Galaxy[planet_idx].planet_economy1[0]] );
					my_printf( "Secondary Economy: %s %s\n", economyModifierNameTable[g_Galaxy[planet_idx].planet_economy2[1]], economyTypeTable[g_Galaxy[planet_idx].planet_economy2[0]] );
					my_printf( "Population: %dk\n", g_Galaxy[planet_idx].planet_population );
					my_printf( "Population Growth: %dk\n", g_Galaxy[planet_idx].planet_growth );
					my_printf( "Location: %4dx %4dy\n\n", g_Galaxy[planet_idx].grid_x, g_Galaxy[planet_idx].grid_y );
					*/
					my_printf( "Planet %s info:\nDesignation Number: %s\nGovernment: %s\nPrimary Economy: %s %s\nSecondary Economy: %s %s\nPopulation: %dk\nPopulation Growth: %dk\nLocation: %4dx %4dy\n\n",
							g_Galaxy[planet_idx].planet_name, PlanetID, governmentNamesTable[g_Galaxy[planet_idx].planet_government],
							economyModifierNameTable[g_Galaxy[planet_idx].planet_economy1[1]],
							economyTypeTable[g_Galaxy[planet_idx].planet_economy1[0]],
							economyModifierNameTable[g_Galaxy[planet_idx].planet_economy2[1]],
							economyTypeTable[g_Galaxy[planet_idx].planet_economy2[0]],
							g_Galaxy[planet_idx].planet_population,
							g_Galaxy[planet_idx].planet_growth,
							g_Galaxy[planet_idx].grid_x, g_Galaxy[planet_idx].grid_y );
					
					return true;
				}
			}
		}

		WriteStr( "No planets found by that name in range commander.\n" );
		return true;
	}
	else
	{
		WriteStr( "Info what commander?  Use info <planet name> to get information about a planet.\n" );
	}

	return 0;
}

boolean __attribute__ ((noinline)) doquit(char * cmd, ship_t *pship)
{
	WriteStr( "Goodbye commander.\n" );
	exit(1);

	return true;
}

// NOTE: These following functions are here to reorder the function addresses
// to make the item purchasing a little easier for them (ie: not having to buy over 4000 items)
//=========================================================//
// COMMAND handler
//=========================================================//
typedef struct COMMAND_STRUCT
{
	char			command_str[32];
	comfunc_ptr		cmd_fptr;
} command_struct;



static int __attribute__ ((noinline)) get_command_line (char *buff, size_t sz) 
{
    int ch, extra;

    if (fgets (buff, sz, stdin) == NULL)
        return NO_INPUT;

    // May have a null terminator for first character (prevents wrap-around on strlen)...
    if ( buff[0] == '\0' )
	return NO_INPUT;

    // If it was too long, there'll be no newline. In that case, we flush
    // to end of line so that excess doesn't affect the next call.
    if (buff[strlen(buff)-1] != '\n') 
	{
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? TOO_LONG : OK;
    }

    if ( buff[strlen(buff)-1] == '\n' )
    {
	buff[strlen(buff)-1] = '\0';

        if ( strlen(buff) > 0 && buff[strlen(buff)-1] == '\r' )
		buff[strlen(buff)-1] = '\0';
    }
   
    return OK;
}

uint32_t __attribute__ ((noinline)) get_random_seed( void )
{
	uint32_t seed1;
	uint32_t seed4 = (uint32_t)(getpid());

	// THIS CODE IS VERY IMPORTANT:
	// IT ALLOWS THEM TO LEAK LIBC via display_prompt function
	// call and get an effective ROP chain	
	// IF YOU ADJUST ANY OF THIS CODE IN THIS FUNCTION OR THE COMPILER SETTINGS
	// THIS WILL REMOVE THIS ABILITY
	__asm__ volatile (
	"xorl %%eax, %%eax\n"
	"movl %%eax, (%%esp)\n"
	"call time\n"
	"addl %1, %%eax\n"
	"movl %%eax, %0"
	: "=g" (seed1)
	: "i" (0x07eb5a59)  // Encodes pop ecx; pop edx; jmp $+7
	: "memory" );
	
	//uint64_t rdtsc_num; = rdtsc();
	uint32_t seed2; // = (uint32_t)(rdtsc_num >> 32);
	uint32_t seed3; // = (uint32_t)(rdtsc_num & 0xFFFFFFFF);	

	__asm__ volatile ("rdtsc": "=a" (seed3), "=d" (seed2));
	
	seed3 += 0x0674505a;		// Encodes pop edx; push ecx; jmp $+6
	
	seed1 ^= seed2;
	seed1 ^= 0x027e5152;		// Encodes mov [edx+8], eax; ret

	seed4 += 0xc3084289;

	// 83c310  -- add ebx, 0x10
	seed1 ^= seed3;
	seed1 ^= seed4;

	return (seed1);	
}

// Used by do_parser
uint8_t __attribute__ ((noinline)) command_match( char *s, command_struct *comfuncs, uint8_t n)
{	
	uint8_t i=0;    
	
	while(i<n)
	{ 
		if ( stringbeg( s, comfuncs[i].command_str ) ) 
			return i+1;

		i++;
	}
	
	return 0;
}

boolean __attribute__ ((noinline)) dohelp(char * cmd, ship_t *pship)
{
	char cmd_name[COMMAND_LINE_LENGTH];
	if ( sscanf( cmd, "%s", cmd_name ) == 1 )
	{
		if ( stringbeg( cmd_name, "buy" ) )
		{
			WriteStr( "Command buy:\nUsage:\nbuy <item amount> <item name>\n\nThis command will buy the amount of items from the planet requested and place them in your cargo hold.\n" );
			return 0;
		}
		else if ( stringbeg( cmd_name, "sell" ) )
		{
			WriteStr( "Command sell:\nUsage:\nsell <item amount> <item name>\n\nThis command will sell the amount of items requested from your cargo hold to the local planet.\n" );
			return 0;
		}
		else if ( stringbeg( cmd_name, "info" ) )
		{
			WriteStr( "Command info:\nUsage:\ninfo <planet name>\n\nThis command will list information about a specific local planet.\n" );
			return 0;
		}
		else if ( stringbeg( cmd_name, "jump" ) )
		{
			WriteStr( "Command jump:\nUsage:\njump <planet name>\n\nThis command will jump to a specific planet.\n" );
			return 0;
		}
		else if ( stringbeg( cmd_name, "quit" ) )
		{
			WriteStr( "Command quit:\nUsage:\nquit\n\nThis command will exit the game.\n" );
			return 0;
		}
		else if ( stringbeg( cmd_name, "hold" ) )
		{
			WriteStr( "Command hold:\nUsage:\nhold\n\nThis command will list the items within your ships hold.\n" );
			return 0;
		}
		else if ( stringbeg( cmd_name, "local" ) )
		{
			WriteStr( "Command local:\nUsage:\nlocal or local <light years>\n\nThis command will list information about planets within your ships jump range or the specified number of light years.\n" );
			return 0;
		}
		else if ( stringbeg( cmd_name, "market" ) )
		{
			WriteStr( "Command market:\nUsage:\nmarket\nmarket <planet name>\n\nThis command will list information market information about planets within 100 light years or list market information about a specific planet.\n" );
			return 0;
		}
 
	}

	WriteStr( "Eliza Help Guide:\nCommands:\n" );
	WriteStr( "help   - Displays the help menu\n" );
	WriteStr( "buy    - Used to buy items from a planet\n" );
	WriteStr( "sell   - Used to sell items to a planet\n" );
	WriteStr( "info   - Describes information about a specific planet\n" );
	WriteStr( "jump   - Jumps to another planet within range\n" );
	WriteStr( "quit   - Exits the game\n" );
	WriteStr( "hold   - Lists items in your cargo hold\n" );
	WriteStr( "local  - Lists planets within range and there economies\n" );	
	WriteStr( "market - Displays the market in your region or on a planet\n\n" );
	WriteStr( "Use help <command> to get a more detailed usage listing\n" );

	return 0;
}

boolean __attribute__ ((noinline)) domkt(char * cmd, ship_t *pship)
{
	uint16_t planet_idx;
	char search_name[COMMAND_LINE_LENGTH];

	// Do market...
	if ( cmd[0] == '\0' )
	{
		WriteStr( "Showing available markets within 100LY of your current position:\n" );

		// Default... display planets within 100 LY
		for ( planet_idx = 0; planet_idx < g_GalaxySize; planet_idx++ )
		{
			uint32_t planet_distance;

			if(	g_Galaxy[planet_idx].last_distance_update < DidJump)
			{
				planet_distance = calc_planet_distance( pship->cur_planet, planet_idx );
				g_Galaxy[planet_idx].planet_distance = planet_distance;
				g_Galaxy[planet_idx].last_distance_update = DidJump;
			}
			else
				planet_distance = g_Galaxy[planet_idx].planet_distance;
				
			if ( planet_distance < MARKET_LIST_RANGE )
			{
				// Dipslay this planet...
				my_printf( "%12s | %4dx | %4dy | %s %s\n", g_Galaxy[planet_idx].planet_name, g_Galaxy[planet_idx].grid_x, g_Galaxy[planet_idx].grid_y, economyModifierNameTable[g_Galaxy[planet_idx].planet_economy1[1]], economyTypeTable[g_Galaxy[planet_idx].planet_economy1[0]] );
			}
		}

		WriteStr( "\n" );
		return (true);
	}

	if ( sscanf( cmd, "%s", search_name ) == 1 )
	{
		// Search for planets...
		// Default... display planets within 100 LY
		uint8_t found;
		uint8_t item_idx;

		found = false;

		if ( stringbeg( search_name, "local" ) )
		{
			found = true;
			planet_idx = pship->cur_planet;
		}
		else
		{
			for ( planet_idx = 0; planet_idx < g_GalaxySize; planet_idx++ )
			{
				uint32_t planet_distance;

				if ( stringbeg( search_name, g_Galaxy[planet_idx].planet_name ) )
				{
					if(g_Galaxy[planet_idx].last_distance_update < DidJump)
					{
						planet_distance = calc_planet_distance( pship->cur_planet, planet_idx );
						g_Galaxy[planet_idx].planet_distance = planet_distance;
						g_Galaxy[planet_idx].last_distance_update = DidJump;
					}
					else
						planet_distance = g_Galaxy[planet_idx].planet_distance;
					
					if ( planet_distance < MARKET_LIST_RANGE )
						found = true;

					break;
				}
			}
		}

		if ( found )
		{
			my_printf( "Displaying market for planet %s:\nItems For Sale:\nItem Name                    |  Qty  | Weight | Price\n", g_Galaxy[planet_idx].planet_name );
	
			for ( item_idx = 0; item_idx < g_Galaxy[planet_idx].item_count; item_idx++ )
			{
				my_printf( "%28s | %5d |  %5d | %5d\n", g_Galaxy[planet_idx].items_for_sale[item_idx].item_name, g_Galaxy[planet_idx].items_for_sale[item_idx].item_amount, g_Galaxy[planet_idx].items_for_sale[item_idx].item_kg, g_Galaxy[planet_idx].items_for_sale[item_idx].item_price );
			}

			WriteStr( "\nItems For Purchase:\nItem Name                    | Excellent | Regular | Cheap\n" );

			for ( item_idx = 0; item_idx < ITEM_COUNT; item_idx++ )
			{
				if ( !g_Galaxy[planet_idx].purchase_item[item_idx].sold )
				{
					my_printf( "%28s |     %5d |   %5d | %5d\n", itemBaseNameTable[item_idx], g_Galaxy[planet_idx].purchase_item[item_idx].excellent, g_Galaxy[planet_idx].purchase_item[item_idx].regular, g_Galaxy[planet_idx].purchase_item[item_idx].cheap);
				}
			}

			my_printf( "\nCash Available: %d\n", g_Galaxy[planet_idx].planet_cash );
		}
	}
	return 0;
}


void __attribute__ ((noinline)) display_prompt( char *planet_name, uint32_t cash, uint32_t fuel, uint32_t hold )
{
	my_printf("Planet[%s] Cash[%6d] Fuel[%6d] Hold[%6d]: ", planet_name, cash, fuel, hold );
}
 
boolean __attribute__ ((noinline)) do_parser( char *command, ship_t *pShip, command_struct comfuncs[] )
{
	struct local_data
	{
		uint32_t i;
		char first_cmd[COMMAND_LINE_LENGTH];
		comfunc_ptr cmd_ptr;
	};

	struct local_data data;

	data.cmd_ptr = NULL;

	spacesplit( command, data.first_cmd );

	data.i = command_match( data.first_cmd, comfuncs, NUM_COMMANDS );
	for ( data.i = 0; data.i < NUM_COMMANDS; data.i++ )
	{	
		if ( stringbeg( data.first_cmd, comfuncs[data.i].command_str ) )
		{
			data.cmd_ptr = comfuncs[data.i].cmd_fptr;
			break;
		}
	}

	if( data.cmd_ptr == NULL )
	{
		WriteStr("Unknown command. Try help commander.\n" );
		return false;
	}

	return (*data.cmd_ptr)( command, pShip );
}

int __attribute__ ((noinline)) main()
{	 
	uint32_t galaxySeed;
	int			fd;
	
	struct local_data
	{
	ship_t	cur_ship;
	command_struct comfuncs[NUM_COMMANDS];
	};

	struct local_data data = {
	.comfuncs =
	{
		{ "help",		dohelp	},
		{ "info",		doinfo	},
		{ "buy",		dobuy	},
		{ "sell",		dosell	},
		{ "jump",		dojump	},
		{ "market",		domkt	},
		{ "hold",		dohold	},
		{ "local",		dolocal	},
		{ "quit",		doquit	}
	}
	};
	char command[COMMAND_LINE_LENGTH];

	setvbuf( stdout, NULL, _IONBF, 0 );

#if DEBUG_ON

#else
	// Turn on signal alarm handler
	signal( SIGALRM, sig_alarm_handler );
	alarm( MAX_IDLE_SECS );
#endif
	
	init_planet_name_generator();

	WriteStr("\nWelcome to the universe of Eliza commander...\n");

	// TODO: Seed from /dev/urandom and make sure it is only a 24-bit number 2^24 = 16million possibilities
	galaxySeed = get_random_seed();
	mysrand( galaxySeed ); // & 0xFFFFFF );

	build_galaxy();

#if DEBUG_ON
	printf("\nGalaxy seed is: %d\n", galaxySeed );
	printf( "Galaxy size %d constructed.\n", g_GalaxySize );
#endif

	init_ship( &(data.cur_ship) );

	// Run command loop
	for (;;)
	{
		uint8_t line_result;

		display_prompt( g_Galaxy[data.cur_ship.cur_planet].planet_name, data.cur_ship.cur_cash, data.cur_ship.cur_fuel, data.cur_ship.cargo_kg );

		line_result = get_command_line( command, COMMAND_LINE_LENGTH );

#if DEBUG_ON

#else
		alarm( MAX_IDLE_SECS );
#endif

		if ( line_result == NO_INPUT )
		{
			WriteStr( "No input... closing connection.\n" );
			return 0;
		}
		else if ( line_result == TOO_LONG )
			WriteStr( "I can't process all those commands commander.\n" );
		else
		{

#if DEBUG_ON
			if ( command[0] == '?' )
			{
				uint8_t i;
				for ( i = 0; i < NUM_COMMANDS; i++ )
					printf( "comfunc: '%s' %X\n", data.comfuncs[i].command_str, data.comfuncs[i].cmd_fptr );

				printf( "Display Prompt: %X\n", display_prompt );
			}
			else if ( command[0] == '!' )
			{
				//uint32_t *pint = (uint32_t*)(command - 0x8e4);
				//(*pint) = 0xAA;
				data.comfuncs[0].cmd_fptr = (comfunc_ptr)(display_prompt);
			}
			else if ( command[0] == '#' )
			{
				data.comfuncs[0].cmd_fptr = (comfunc_ptr)(get_random_seed+0x38);
			}
			else if ( command[0] == '@' )
			{
				// Jump to the ROP trampoline (for debugging)
				data.comfuncs[0].cmd_fptr = (comfunc_ptr)(get_random_seed+0x93);
			}
			else if ( command[0] == '*' )
			{
				printf( "printf address: %X\n", printf );
			}
			else if ( command[0] == '$' )
			{
				check_world( );	
				data.cur_ship.cur_cash = 100000;
			}
#endif

#if DEBUG_LEVEL2_ON
			if ( command[0] == '!' )
			{
				printf( "display_prompt: %X\n", display_prompt );
				printf( "domkt: %X\n", domkt );
				printf( "printf: %X\n", printf );
				printf( "comfunc 0: %X\n", data.comfuncs[0].cmd_fptr );
				data.cur_ship.items[3].item_amount = (uint16_t)((uint32_t)display_prompt & 0xFFFF);
				printf( "comfunc 0: %X\n", data.comfuncs[0].cmd_fptr );
			}
#endif
			msleep(1);
			do_parser( command, &(data.cur_ship), (data.comfuncs) );
		}
	}

	return 0;
}

#if DEBUG_ON
void check_world( void )
{
	uint32_t planet_i, planet_j;
	uint32_t item_i, item_j;
	trade_item_t *pitem_i, *pitem_j;
	uint32_t total_kg, item_amount_total;
	uint16_t item_purchase_price;
	uint16_t item_sell_price;
	uint16_t item_kg;
	uint16_t item_amount;
	uint16_t item_price_difference;

	uint32_t i, j;
	uint32_t item_count_array[ITEM_COUNT][ITEM_MODIFIER_COUNT];
	
	for ( i = 0; i < ITEM_COUNT; i++ )
		for ( j = 0; j < ITEM_MODIFIER_COUNT; j++ )
		item_count_array[i][j] = 0;

	for ( i = 0; i < ITEM_COUNT; i++ )
	{
		for ( j = 0; j < ITEM_MODIFIER_COUNT; j++ )
		{
			// SKIP fuel
			if ( i == 0 )
				continue;
		
			for ( planet_i = 0; planet_i < g_GalaxySize; planet_i++ )
			{
				for ( item_i = 0; item_i < g_Galaxy[planet_i].item_count; item_i++ )
				{
					pitem_i = &(g_Galaxy[planet_i].items_for_sale[item_i]);

					if ( pitem_i->item_type == i && pitem_i->item_modifier_idx == j )
						item_count_array[i][j] += pitem_i->item_amount;
				}	
			}	
		}
	}

	for ( i = 0; i < ITEM_COUNT; i++ )
	{
		for ( j = 0; j < ITEM_MODIFIER_COUNT; j++ )
		{
			// SKIP fuel
			if ( i == 0 )
				continue;
			
			my_printf( "Item [%d][%d] count: %d\n", i, j, item_count_array[i][j] );
		}
	}

	return ;

	// Check if you can make money
	for ( planet_i = 0; planet_i < g_GalaxySize; planet_i++ )
		for ( planet_j = 0; planet_j < g_GalaxySize; planet_j++ )
		{
			// item_type
			// item_modifier_idx
			total_kg = 0;
			item_amount_total = 0;
			
			//while ( total_kg < SHIP_MAX_CARGO )
			{
				
				for ( item_i = 0; item_i < g_Galaxy[planet_i].item_count; item_i++ )
				{
					// Buy item_i from planet i and sell to planet j
					pitem_i	= &(g_Galaxy[planet_i].items_for_sale[item_i]);
		
					item_purchase_price = pitem_i->item_price;
					item_kg = pitem_i->item_kg;
					item_amount = pitem_i->item_amount;

					// Check if planet_j has this item for purchase
					uint8_t bFound = 0;
					for ( item_j = 0; item_j < g_Galaxy[planet_j].item_count; item_j++ )
					{
						pitem_j = &(g_Galaxy[planet_j].items_for_sale[item_j]);
	
						if ( pitem_i->item_type == pitem_j->item_type )
							bFound = 1;
					}

					if ( bFound )
						continue;
				
					item_sell_price = get_item_purchase_price( pitem_i->item_type, pitem_i->item_modifier_idx, g_Galaxy[planet_j].planet_economy1[1], g_Galaxy[planet_j].planet_economy2[1] );

					if ( item_sell_price > item_purchase_price )
					{
						item_price_difference = item_sell_price - item_purchase_price;
						
						// Now calculate total possible amount (based on cargo hold and amount for sale)
						my_printf( "Item Purchase[%d] Sell[%d] Quantity[%d] Difference[%d]:\n", item_purchase_price, item_sell_price, item_amount, item_price_difference );
							
						
					}	
				}

			}

			
			
		}

	// 
}
#endif
