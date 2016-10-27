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

// Take care, increases wildly the duration if it gets too high O(2^n) probably
const int COEFF_FREQ_MOV_NOISE = 1;
const int RANDOM_NOISE_ADD = 5;
const int MOV = 50;

const double WATER_RATIO = 0.666666;

const int COL_RANGE = 256;

int T_DYN_WATER = 128;

//PROTOS

int _rand(int,int);
int** diamondsquare();
int** convert(int**,int);
void calculateWaterLevel(int**, int);
int** generateLayers(int,int);
int circularMask(int,int,double,int**,int**);
int** shape(int**,int);



int _rand(int min, int max){
	if(min == 0 && max == 0) { return 0; }
	int i = rand()%(max-min+1);
	i+=min;
	return i;
}

// TODO add free

int** diamondsquare(int tot_size)
{
	int size = tot_size-1;
	int extent = size;
	int half = (tot_size) / 2;
	int ** points = malloc(sizeof(int*)*(tot_size));
	for(int i = 0; i < tot_size; i++) { points[i] = malloc(sizeof(int)*(tot_size)); }
	int scale = half * COEFF_SCALE;

	points[0][0]=0;
	points[size][0]=0;
	points[0][size]=0;
	points[size][size]=0;

	while(size > 1)
    {
		// SQUARE
		for(int x = 0; x < extent; x+= size)
		{
			for(int y = 0; y < extent; y+=size)
			{
				//printf("%d %d %d %d\n",x,y,extent,size);
				int sq_avg = (points[x][y] + points[x+size][y] + points[x][y+size] + points[x+size][y+size])/4;
				points[x+half][y+half] = sq_avg + _rand(-scale,scale);
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
					avg = (points[x][y] + points[x+size][y] + points[x+half][y+half] + points[x+half][y-half]) / 4;
				}else{
					avg = (points[x][y] + points[x+size][y] + points[x+half][y+half]) / 3;
				}
				points[x+half][y] = avg + _rand(-scale, scale);
 
				// the bottom point of the diamond
				if(y < extent - size){
					avg = (points[x+size][y+size] + points[x+half][y+half] + points[x+size][y+size] + points[x+half][y+size+half]) / 4;
				}else{
					avg = (points[x+size][y+size] + points[x+half][y+half] + points[x+size][y+size]) / 3;
				}
				points[x+half][y+size] = avg + _rand(-scale, scale);
 
				// the left point of the diamond
				if(x > 0){
					avg = (points[x][y] + points[x+half][y+half] + points[x][y+size] + points[x-half][y+half]) / 4;
				}else{
					avg = (points[x][y] + points[x+half][y+half] + points[x][y+size]) / 3;
				}
				points[x][y+half] = avg + _rand(-scale, scale);
 
				//the right point of the diamond
				if(x < extent - size){
					avg = (points[x+size][y+size] + points[x+half][y+half] + points[x+size][y+size] + points[x+size+half][y+half]) / 4;
				}else{
					avg = (points[x+size][y+size] + points[x+half][y+half] + points[x+size][y+size]) / 3;
				}
				points[x+size][y+half] = avg + _rand(-scale, scale);
			}
		}
		// update values
		size /= 2;
		half /= 2;
		scale = half * COEFF_SCALE;
    }
	return points;
}

int** convert(int** tab, int length)
{
	int min = 0, max = 0;
	int** t = malloc(sizeof(int*)*SIZE);
	printf("Converting array to [ 0 ; %d [ ...\n",length);
	for(int i = 0; i < SIZE; i++)
    {
		t[i] = malloc(sizeof(int)*SIZE);
		for(int j = 0; j < SIZE; j++)
		{
			t[i][j] = tab[i][j];
			if(t[i][j]>max){ max = t[i][j]; }
			if(t[i][j]<min){ min = t[i][j]; }
		}
    }
	printf("BEFORE : Minimum : %d | Maximum : %d\n",min, max);
	for(int i = 0; i < SIZE; i++)
    {
		for(int j = 0; j < SIZE; j++)
		{
			double k = t[i][j];
			k = k - min;
			k = (k)/(max-min);
			k = k * (length-1);
			t[i][j] = (int)k;
		}
    }
	min = 0;
	max = 0;
	for(int i = 0; i < SIZE; i++)
    {
		for(int j = 0; j < SIZE; j++)
		{
			if(t[i][j]>max){ max = t[i][j]; }
			if(t[i][j]<min){ min = t[i][j]; }
		}
    }
	printf("AFTER : Minimum : %d | Maximum : %d\n\n",min, max);
	return t;
}

void calculateWaterLevel(int** t, int length)
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
			histo[t[i][j]]++;
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

int**  generateLayers(int k, int seed)
{
	int** layers = malloc(sizeof(int*)*SIZE);
	for(int i = 0; i < SIZE; i++)
	{
		layers[i] = malloc(sizeof(int)*SIZE);
		for(int j = 0; j < SIZE; j++)
		{
			layers[i][j] = 0;
		}
	}
	srand(seed);
	printf("Generating and combining map layers ...\n\n");
	
	for(int a = 0; a < k; a++)
    {
		printf("Generating layer %d ...\n",a+1);
		//srand((1+a)*seed);
		int** t = diamondsquare(SIZE);
		int** t2 = convert(t,COL_RANGE/(a+1.0));
		for(int i = 0; i < SIZE; i++)
		{
			for(int j = 0; j < SIZE; j++)
			{
				layers[i][j]+=t2[i][j];
			}
		}
	}
	printf("Normalizing map ...\n");
	int** result = convert(layers,COL_RANGE);
	calculateWaterLevel(result,COL_RANGE);
	result = shape(result,seed);
	return result;
}

int circularMask(int x, int y, double radius,int** noise_x,int** noise_y)
{
	int center = SIZE/2;

	int coord_noise_x = x*COEFF_FREQ_MOV_NOISE;
	int coord_noise_y = y*COEFF_FREQ_MOV_NOISE;

	coord_noise_x %= 2*SIZE;
	coord_noise_x = coord_noise_x >= SIZE ? (SIZE*2) - coord_noise_x : coord_noise_x;

	
	coord_noise_y %= 2*SIZE;
	coord_noise_y = coord_noise_y >= SIZE ? (SIZE*2) - coord_noise_y : coord_noise_y;
	
	int i = x + noise_x[coord_noise_x][coord_noise_y] + _rand(-RANDOM_NOISE_ADD,RANDOM_NOISE_ADD);
	int j = y + noise_y[coord_noise_x][coord_noise_y] + _rand(-RANDOM_NOISE_ADD,RANDOM_NOISE_ADD);
	
	i = i<0?0:i;
	i = i>=SIZE?SIZE-1:i;
	j = j<0?0:j;
	j = j>=SIZE?SIZE-1:j;
	
	double distance = sqrt(pow(i-center,2)+pow(j-center,2));//+_rand(-radius/10,radius/10);
	return distance<=radius?1:0;
}
 
 int** shape(int** t, int seed)
 {
	// fix the seed^
	
	printf("Shaping the map ...\n\n");
	int pow_size = log(SIZE-1)/log(2);

	int size = pow(2,pow_size+COEFF_FREQ_MOV_NOISE)+1;

	int radius = SIZE/2.0 * 0.7;

	
	int** new = malloc(sizeof(int*)*SIZE);
	srand(seed+1);
	printf("Generating x noise\n");
	int** noise_x = convert(diamondsquare(size),MOV*2);
	srand(seed+2);
	printf("Generating y noise\n");
	int** noise_y = convert(diamondsquare(size),MOV*2);
	for(int i = 0; i < SIZE; i++)
	{
		new[i] = malloc(sizeof(int)*SIZE);
		for(int j = 0; j < SIZE; j++)
		{
			new[i][j]=t[i][j];
			noise_x[i][j]-=MOV;
			noise_y[i][j]-=MOV;
		}
	}
	int center = SIZE/2;
	double max_dist = sqrt(pow(center,2)+pow(center,2));
	
	for(int i = 0; i < SIZE; i++)
	{
		for(int j = 0; j < SIZE; j++)
		{
			/*if(circularMask(i,j,radius,noise_x,noise_y)==1)
			{
				//new[x][y]=new[i][j];
			}
			else
			{
				int k = new[i][j];
				//k = k>=T_DYN_WATER ? (2*T_DYN_WATER)-k-1 : k-1;
				new[i][j] = k;
			}*/
			double distance = sqrt(pow(i-center,2)+pow(j-center,2));//+_rand(-radius/10,radius/10);
			if(distance > radius)
			{
				double d = (max_dist-distance)/(max_dist-radius);
				new[i][j]*=d;
			}
		}
	}
	
	//polish is overrated we want small islands
	// int polish_cycles = 20;
	/*for(int a = 0; a < polish_cycles; a++){
		for(int i = 2; i < SIZE-2; i++)
		{
			for(int j = 2; j < SIZE-2; j++)
			{
				if(new[i][j]>=T_DYN_WATER)
				{
					int count = 0;
					if(new[i-1][j]<T_DYN_WATER){count++;}
					if(new[i+1][j]<T_DYN_WATER){count++;}
					if(new[i][j-1]<T_DYN_WATER){count++;}
					if(new[i][j+1]<T_DYN_WATER){count++;}
					
					if(new[i-1][j-1]<T_DYN_WATER){count++;}
					if(new[i+1][j-1]<T_DYN_WATER){count++;}
					if(new[i-1][j+1]<T_DYN_WATER){count++;}
					if(new[i+1][j+1]<T_DYN_WATER){count++;}
					
					if(new[i-2][j]<T_DYN_WATER){count++;}
					if(new[i+2][j]<T_DYN_WATER){count++;}
					if(new[i][j-2]<T_DYN_WATER){count++;}
					if(new[i][j+2]<T_DYN_WATER){count++;}
					
					if(count>=7)
					{
						new[i][j]=T_DYN_WATER-1;
					}
				}
			}
		}
	}*/
	
	return new;
 }

int main(int argc, char** argv){
	FILE* out = fopen("output.ppm","w");
	if(argc>2)
    {
	fprintf(stderr,"%sUsage : ./main SEED\n",KRED);
		return -1;
    }

	printf("***********************************************************\n*** Welcome to Léo Andéol's world generation algorithm  ***\n*** Using the diamond square algorithm                  ***\n*** Website : www.altruth.com                           ***\n*** Source : www.github.com/leoandeol/TODO              ***\n***********************************************************\n\n");

	int seed;
	
	if(argc==2){
		seed = atoi(argv[1]);
		printf("Generating world according to chosen seed %d\n\n",seed);
	} else {
		seed = time(NULL);
		printf("Generating world according to random seed %d\n\n",seed);
	}

	// FUNCTION CALL
	int** output = generateLayers(5,seed);

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
			int i = output[x][y];
			if(i<T_DYN_WATER){
				i=50+(i/2);
				fprintf(out,"0 0 %d",i);
			}
			else {
				i=170-(i/2);
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
