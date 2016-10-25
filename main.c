#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

const int SIZE = 2049;

const int COEFF_SCALE = 4;

const int T_DEEP_WATER = 30;
const int T_WATER = 170;
const int T_GROUND = 200;
const int T_HIGH = 220;
int T_DYN_WATER = 128;

const char* DEEP_WATER = "0 20 140";
const char* WATER = "0 20 255";
const char* GROUND = "0 100 0";
const char* HIGH = "128 128 128";
const char* HIGHER = "180 180 180";

int _rand(int min, int max){
  if(min == 0 && max == 0) { return 0; }
  int i = rand()%(max-min+1);
  i+=min;
  return i;
}

int** diamondsquare()
{
  int size = SIZE-1;
  int extent = size;
  int half = (SIZE) / 2;
  int ** points = malloc(sizeof(int*)*(SIZE));
  for(int i = 0; i < SIZE; i++) { points[i] = malloc(sizeof(int)*(SIZE)); }
  int scale = half * COEFF_SCALE;
  int firstloop=1;
  /* full random
     points[0][0]=half+_rand(-scale,scale);
     points[size][0]=half+_rand(-scale,scale);
     points[0][size]=half+_rand(-scale,scale);
     points[size][size]=half+_rand(-scale,scale);
  */

  points[0][0]=0;//-(size);
  points[size][0]=0;//-(size);
  points[0][size]=0;//-(size);
  points[size][size]=0;//-(size);

  while(size > 1)
    {
      // SQUARE
      if(!firstloop){
	for(int x = 0; x < extent; x+= size)
	  {
	    for(int y = 0; y < extent; y+=size)
	      {
		//printf("%d %d %d %d\n",x,y,extent,size);
		int sq_avg = (points[x][y] + points[x+size][y] + points[x][y+size] + points[x+size][y+size])/4;
		points[x+half][y+half] = sq_avg + _rand(-scale,scale);
	      }
	  }} else { firstloop=0; points[half][half] = size;}
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
  printf("Converting...\n");
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
  printf("Minimum : %d | Maximum : %d\n",min, max);
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
  int cap = (SIZE*SIZE*2)/3;
  for(int i = 0; i < length; i++)
    {
      sum+=histo[i];
      if(sum>cap)
	{
	  T_DYN_WATER=i;
	  break;
	}
    }
	
  printf("Water height : %d\n",T_DYN_WATER);
}

int**  generateLayers(int k, long seed)
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
  for(int a = 0; a < k; a++)
    {
      srand((1+a)*seed);
      int** t = diamondsquare();
      int** t2 = convert(t,256.0/(a+1.0));
      for(int i = 0; i < SIZE; i++)
	{
	  for(int j = 0; j < SIZE; j++)
	    {
	      layers[i][j]+=t2[i][j];
	    }
	}
}
  int** result = convert(layers,256);
  calculateWaterLevel(result,256);
  return result;
}

int main(int argc, char** argv){
  FILE* out = fopen("output.ppm","w");
  if(argc>2)
    {
      fprintf(stderr,"Usage : ./main SEED\n");
      return -1;
    }
  if(argc==2){
    printf("Generating according to seed %d\n",atoi(argv[1]));
    srand(atoi(argv[1]));
  } else {
    unsigned int a = time(NULL);
    printf("Generating according to random seed %d\n",a);
    srand(a);
  }

  // FUNCTION CALL
  int** output = generateLayers(3,time(NULL));

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
      /*if(i<T_DEEP_WATER){
	fprintf(out,DEEP_WATER);
	}
	else if(i<T_WATER){
	fprintf(out,WATER);
	} else if(i < T_GROUND){
	fprintf(out,GROUND);
	} else if(i < T_HIGH){
	fprintf(out,HIGH);
	} else {
	fprintf(out,HIGHER);
	}*/
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
  fclose(out);
  return 0;
}
