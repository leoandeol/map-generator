#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

// COLOR STUFF

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

const int SIZE = 513;

const int COEFF_SCALE = 5;

const double WATER_RATIO = 0.7;

const int COL_RANGE = 256;

int T_DYN_WATER = 128;

const int RADIUS_RATIO = 0.7;
const int RAND_DECR = 3;
const int POLISH_CYCLES = 3;

//const unsigned int ZERO_DEG_IN_KELVIN = 273;
const int DEFAULT_LOWEST_TEMPERATURE_DEGREE = -30;
const int DEFAULT_HIGHEST_TEMPERATURE_DEGREE = 30;
const int DEGREE_CHANGE_BY_LAYER = 3;

//PROTOS

typedef enum {
	TUNDRA,
	GRASSLAND,
	DESERT,
	SAVANNA,
	TROPICAL_THORN_SCRUB_AND_WOODLAND,
	BOREAL_FOREST,
	TEMPERATE_FOREST,
	TROPICAL_SEASONAL_FOREST,
	TEMPERATE_RAINFOREST,
	TROPICAL_RAINFOREST
} Biome;

typedef struct {
	// temperatures in celsius
	short ground_temperature; // distance to equator
	short surface_temperature; // taking in account altitute
	short elevation; // according to the noise map
	short precipitations;
	//short humidity; // between 0 and 100
	Biome biome;
} Tile;

//const Biome[][] WHITTAKER_CHART;

int _rand(int,int);
Tile** diamondsquare(int);
void convert(Tile**,int);
void calculateWaterLevel(Tile**, int);
Tile** generateLayers(int,int);
void shape(Tile**);//outddated
void calculateGroundTemperature(Tile**);
void calculatePrecipitations(Tile**);
void calculateSurfaceTemperature(Tile**);
void calculateBiome(Tile**);



int _rand(int min, int max){
	if(min == 0 && max == 0) { return 0; }
	int i = rand()%(max-min+1);
	i+=min;
	return i;
}

Tile** diamondsquare(int tot_size)
{
	int size = tot_size-1;
	int extent = size;
	int half = (tot_size) / 2;
    Tile** points = malloc(sizeof(Tile*)*(tot_size));
	for(int i = 0; i < tot_size; i++) { points[i] = malloc(sizeof(Tile)*(tot_size)); }
	int scale = half * COEFF_SCALE;

	points[0][0].elevation=0;
	points[size][0].elevation=0;
	points[0][size].elevation=0;
	points[size][size].elevation=0;

	while(size > 1)
    {
		// SQUARE
		for(int x = 0; x < extent; x+= size)
		{
			for(int y = 0; y < extent; y+=size)
			{
				//printf("%d %d %d %d\n",x,y,extent,size);
				int sq_avg = (points[x][y].elevation + points[x+size][y].elevation + points[x][y+size].elevation + points[x+size][y+size].elevation)/4;
				points[x+half][y+half].elevation = sq_avg + _rand(-scale,scale*2);
			}
		}
		//DIAMOND
		for(int x = 0; x < extent; x+= size)
		{
			for(int y = 0; y < extent; y+=size)
			{
				int avg;
				// the top point of the diamond
				if(y > 0){
					avg = (points[x][y].elevation + points[x+size][y].elevation + points[x+half][y+half].elevation + points[x+half][y-half].elevation) / 4;
				}else{
					avg = (points[x][y].elevation + points[x+size][y].elevation + points[x+half][y+half].elevation) / 3;
				}
				points[x+half][y].elevation = avg + _rand(-scale, scale*2);
 
				// the bottom point of the diamond
				if(y < extent - size){
					avg = (points[x+size][y+size].elevation + points[x+half][y+half].elevation + points[x+size][y+size].elevation + points[x+half][y+size+half].elevation) / 4;
				}else{
					avg = (points[x+size][y+size].elevation + points[x+half][y+half].elevation + points[x+size][y+size].elevation) / 3;
				}
				points[x+half][y+size].elevation = avg + _rand(-scale, scale*2);
 
				// the left point of the diamond
				if(x > 0){
					avg = (points[x][y].elevation + points[x+half][y+half].elevation + points[x][y+size].elevation + points[x-half][y+half].elevation) / 4;
				}else{
					avg = (points[x][y].elevation + points[x+half][y+half].elevation + points[x][y+size].elevation) / 3;
				}
				points[x][y+half].elevation = avg + _rand(-scale, scale*2);
 
				//the right point of the diamond
				if(x < extent - size){
					avg = (points[x+size][y+size].elevation + points[x+half][y+half].elevation + points[x+size][y+size].elevation + points[x+size+half][y+half].elevation) / 4;
				}else{
					avg = (points[x+size][y+size].elevation + points[x+half][y+half].elevation + points[x+size][y+size].elevation) / 3;
				}
				points[x+size][y+half].elevation = avg + _rand(-scale, scale*2);
			}
		}
		// update values
		size /= 2;
		half /= 2;
		scale = half * COEFF_SCALE;
    }
	return points;
}

void convert(Tile** tab, int length)
{
	int min = 0, max = 0;
	printf("Converting array to [ 0 ; %d [ ...\n",length);
	for(int i = 0; i < SIZE; i++)
    {
		for(int j = 0; j < SIZE; j++)
		{
			if(tab[i][j].elevation>max){ max = tab[i][j].elevation; }
			if(tab[i][j].elevation<min){ min = tab[i][j].elevation; }
		}
    }
	printf("BEFORE : Minimum : %d | Maximum : %d\n",min, max);
	for(int i = 0; i < SIZE; i++)
    {
		for(int j = 0; j < SIZE; j++)
		{
			double k = tab[i][j].elevation;
			k = k - min;
			k = (k)/(max-min);
			k = k * (length-1);
			tab[i][j].elevation = (int)k;
		}
    }
	min = 0;
	max = 0;
	for(int i = 0; i < SIZE; i++)
    {
		for(int j = 0; j < SIZE; j++)
		{
			if(tab[i][j].elevation>max){ max = tab[i][j].elevation; }
			if(tab[i][j].elevation<min){ min = tab[i][j].elevation; }
		}
    }
	printf("AFTER : Minimum : %d | Maximum : %d\n\n",min, max);
}

void calculateWaterLevel(Tile** t, int length)
{
	printf("Calculating the water level ...\n");

	// let's find the value for which 2/3 of the values are lower
	int histo[length];

	// init the histogram
	for(int i = 0; i < length; i++)
    {
		histo[i]=0;
    }

	// fill the histogram
	for(int i = 0; i < SIZE; i++)
    {
		for(int j = 0; j < SIZE; j++)
		{
			histo[t[i][j].elevation]++;
		}
    }
	
	int sum = 0;
	int cap = (SIZE*SIZE)*WATER_RATIO;
	for(int i = 0; i < length; i++)
    {
		sum+=histo[i];
		if(sum>cap)
		{
			T_DYN_WATER=i;
			break;
		}
    }
	
	printf("Water height : %d\n\n",T_DYN_WATER);
}

Tile**  generateLayers(int k, int seed)
{
	Tile** layers = malloc(sizeof(Tile*)*SIZE);
	for(int i = 0; i < SIZE; i++)
	{
		layers[i] = malloc(sizeof(Tile)*SIZE);
		for(int j = 0; j < SIZE; j++)
		{
			layers[i][j].elevation = 0;
		}
	}
	srand(seed);
	printf("Generating and combining map layers ...\n\n");
	
	for(int a = 0; a < k; a++)
    {
		printf("Generating layer %d ...\n",a+1);
		//srand((1+a)*seed);
		Tile** t = diamondsquare(SIZE);
		convert(t,COL_RANGE/(a+1.0));
		for(int i = 0; i < SIZE; i++)
		{
			for(int j = 0; j < SIZE; j++)
			{
				layers[i][j].elevation+=t[i][j].elevation;
			}
		}
		for(int i = 0; i < SIZE; i++)
		{
			free(t[i]);
		}
		free(t);
	}
	printf("Normalizing map ...\n");
	convert(layers,COL_RANGE);
	calculateWaterLevel(layers,COL_RANGE);
	shape(layers);
	
	return layers;
}
 
void shape(Tile** new)
{
	// fix the seed^
	
	printf("Shaping the map ...\n\n");

	
	/*int** new = malloc(sizeof(Tile*)*SIZE);
	for(int i = 0; i < SIZE; i++)
	{
		new[i] = malloc(sizeof(Tile)*SIZE);
		for(int j = 0; j < SIZE; j++)
		{
			new[i][j]=t[i][j];
		}
	}*/
	
	//polish is overrated we want small islands
	for(int a = 0; a < POLISH_CYCLES; a++){
		for(int i = 1; i < SIZE-1; i++)
		{
			for(int j = 1; j < SIZE-1; j++)
			{
				if(new[i][j].elevation>=T_DYN_WATER)
				{
					int count = 0;
					if(new[i-1][j].elevation<T_DYN_WATER){count++;}
					if(new[i+1][j].elevation<T_DYN_WATER){count++;}
					if(new[i][j-1].elevation<T_DYN_WATER){count++;}
					if(new[i][j+1].elevation<T_DYN_WATER){count++;}
					
					if(count>=3)
					{
						new[i][j].elevation=(new[i-1][j].elevation+new[i][j-1].elevation+new[i+1][j].elevation+new[i][j+1].elevation)/4;
					}
				}
				else if(new[i][j].elevation<T_DYN_WATER)
				{
					int count = 0;
					if(new[i-1][j].elevation>=T_DYN_WATER){count++;}
					if(new[i+1][j].elevation>=T_DYN_WATER){count++;}
					if(new[i][j-1].elevation>=T_DYN_WATER){count++;}
					if(new[i][j+1].elevation>=T_DYN_WATER){count++;}
					
					if(count>=3)
					{
						new[i][j].elevation=(new[i-1][j].elevation+new[i][j-1].elevation+new[i+1][j].elevation+new[i][j+1].elevation)/4;
					}
				}
			}
		}
	}	
	//return new;
 }

void calculateGroundTemperature(Tile** t)
{
	int center = SIZE / 2;
	for(int i = 0; i < SIZE; i++)
	{
		for(int j = 0; j < SIZE; j++)
		{
			short tmp = ((double)center/abs(j-center))*60 - 30;
			t[i][j].ground_temperature=tmp;
		}
	}
}


void calculatePrecipitations(Tile** t)
{
	
}

void calculateSurfaceTemperature(Tile** t)
{
	for(int i = 0; i < SIZE; i++)
	{
		for(int j = 0; j < SIZE; j++)
		{
			short rel_level = t[i][j].elevation - T_DYN_WATER;
			if(rel_level>=0)
			{
				t[i][j].surface_temperature = t[i][j].ground_temperature - (rel_level * DEGREE_CHANGE_BY_LAYER);
			}
			else
			{
				t[i][j].surface_temperature = t[i][j].ground_temperature + (rel_level * DEGREE_CHANGE_BY_LAYER);
			}
		}
	}
}

void calculateBiome(Tile** t)
{
	
}

int main(int argc, char** argv){
	FILE* out = fopen("output.ppm","w");
	if(argc>2)
    {
	fprintf(stderr,"%sUsage : ./main SEED\n",KRED);
		return -1;
    }

	printf("***************************************************************\n*** Welcome to Léo Andéol's world generation algorithm v0.1 ***\n*** Using the diamond square algorithm                      ***\n*** Website : www.altruth.com                               ***\n*** Source : www.github.com/leoandeol/map-generator         ***\n***************************************************************\n\n");

	int seed;
	
	if(argc==2){
		seed = atoi(argv[1]);
		printf("Generating world according to chosen seed %d\n\n",seed);
	} else {
		seed = time(NULL);
		printf("Generating world according to random seed %d\n\n",seed);
	}

	// FUNCTION CALL
	Tile** output = generateLayers(3,seed);

	// WRITE HEADER	
	fprintf(out,"P3\n");
	fprintf(out,"%d",SIZE);
	fprintf(out," ");
	fprintf(out,"%d",SIZE);
	fprintf(out,"\n");
	fprintf(out,"255\n");

	// WRITE IMAGE
	for(int y = 0; y < SIZE; y++){
		for(int x = 0; x < SIZE; x++){
			short i = output[x][y].elevation;
			if(i<T_DYN_WATER){
				i=50+(i/2);
				fprintf(out,"0 0 %d",i);
			}
			else {
				i=180-(i/2);
				fprintf(out,"%d %d %d",i/2,i,i/3);
			}
			if(x<SIZE-1)
			{
				fprintf(out," ");
			}
		}
		fprintf(out,"\n");
	}

	printf("%sGeneration complete as output.ppm\n",KGRN);
  
	fclose(out);
	return 0;
}
