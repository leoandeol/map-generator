#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

//const int WIDTH = 2000;
//const int HEIGHT = 2000;

// POUR GEN1 : 2pow11 +1

const int SIZE = 2048;
const int WIDTH = 2049;
const int HEIGHT = 2049;
const int H = 2;

const char* WATER = "0 20 255";
const char* GROUND = "0 100 0";
const char* HIGH = "128 128 128";

int _rand(int min, int max){
	if(min == 0 && max == 0) { return 0; }
	int i = rand()%(max-min+1);
	i+=min;
	return i;
}

void gen1rec(int** t,int x, int y, int mx, int my, int dev)
{
	//printf("x=%d\ny=%d\nmx=%d\nmy=%d\n\n",x,y,mx,my);

	/*      e
	      a 3 b
	    g 2 1 4 f
          c 5 d
		    h
	

			 e f g h being theoretically out of scope
	*/
	int a = t[x][y];
	int b = t[mx][y];
	int c = t[x][my];
	int d = t[mx][my];

	// halves

	int hx = (mx+x)/2;
	int hy = (my+y)/2;

	// Gotta test them, 0 by default : will make the edges have a lower altitude;

	int r = _rand(-dev,dev);
	
	int tmp = (y - ((my-y)/2));

	int e = tmp>=0 ? t[hx][tmp] : r;

	tmp = (x + ((mx-x)/2));

	int f = tmp<WIDTH ? t[tmp][hy] : r;

	tmp = (x - ((mx-x)/2));

	int g = tmp>=0 ? t[tmp][hy] : r;

	tmp = (y + ((my-y)/2));

	int h = tmp<HEIGHT ? t[hx][tmp] : r;

	// Update values
	
	int ce = (a + b + c + d)/4 + _rand(-dev,dev);

	t[hx][hy] = ce;
	
	t[x][hy] = (ce + a + g + c)/4 + _rand(-dev,dev);

	t[hx][y] = (a + e + b + ce)/4 + _rand(-dev,dev);

	t[mx][hy] = (b + f + d + ce)/4 + _rand(-dev,dev);

	t[hx][my] = (ce + c + d + h)/4 + _rand(-dev,dev);

	dev *= pow(2,-H);
	
	if(mx-x>1)
	{	   
		gen1rec(t,x,y,(x+mx)/2,(y+my)/2,dev);
		gen1rec(t,(x+mx)/2,y,mx,(y+my)/2,dev);
		gen1rec(t,x,(y+my)/2,(x+mx)/2,my,dev);
		gen1rec(t,(x+mx)/2,(y+my)/2,mx,my,dev); 
	}
}

int** gen1(){
	int** tab = malloc(sizeof(int*)*WIDTH);
	for(int i = 0; i < WIDTH; i++)
	{
		tab[i] = malloc(sizeof(int)*HEIGHT);
		/*for(int j; j < HEIGHT; j++)
		{
			tab[i][j]=0;
		}*/
	}
	int half = WIDTH/2;
	int DEVIATION = half*3;
	
	tab[0][0]=half+_rand(-DEVIATION,DEVIATION);
	tab[WIDTH-1][0]=half+_rand(-DEVIATION,DEVIATION);
	tab[0][HEIGHT-1]=half+_rand(-DEVIATION,DEVIATION);
	tab[WIDTH-1][HEIGHT-1]=half+_rand(-DEVIATION,DEVIATION);
	// never touch corners to avoid crashes
	gen1rec(tab,0,0,WIDTH-1,HEIGHT-1,DEVIATION);
    for(int i = 0; i < WIDTH; i++)
	{
		for(int j; j < HEIGHT; j++)
		{
			tab[i][j]+=150;;
		}
	}  
	return tab;
}

int** gen2()
{
	int size = SIZE;
	int extent = size;
	int half = (size) / 2;
	int ** points = malloc(sizeof(int*)*(size+1));
	for(int i = 0; i < size+1; i++) { points[i] = malloc(sizeof(int)*(size+1)); }
	int scale = half * 2;
	
	points[0][0]=half+_rand(-scale,scale);
	points[size][0]=half+_rand(-scale,scale);
	points[0][size]=half+_rand(-scale,scale);
	points[size][size]=half+_rand(-scale,scale);

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
		scale = half * 2;
	}
	return points;
}

int** convert(int** tab)
{
	int min = 0, max = 0;
	int** t = malloc(sizeof(int*)*WIDTH);
	printf("Converting...\n");
	for(int i = 0; i < WIDTH; i++)
	{
		t[i] = malloc(sizeof(int)*HEIGHT);
		for(int j = 0; j < HEIGHT; j++)
		{
			if(i<WIDTH-1&&j<HEIGHT-1)
			{
				t[i][j] = (tab[i][j] + tab[i+1][j] + tab[i][j+1] + tab[i+1][j+1])/4;
			}
			if(t[i][j]>max){ max = t[i][j]; }
			if(t[i][j]<min){ min = t[i][j]; }
		}
	}
	printf("Minimum : %d | Maximum : %d\n",min, max);
	for(int i = 0; i < WIDTH; i++)
	{
		for(int j = 0; j < HEIGHT; j++)
		{
			double k = t[i][j];
			k = k - min;
			k = (k)/(max-min);
			k = k * 255;
			t[i][j] = (int)k;
		}
	}
	return t;
}

int main(int argc, char** argv){
	int seed = atoi(argv[2]);
	srand(seed);
	FILE* out = fopen("output.pgm","w");
	if(argc!=3)
	{
		fprintf(stderr,"Usage : ./main MODE SEED\n");
		return -1;
	}
	int** (*fct)();
	int nb_fct = atoi(argv[1]);
	switch(nb_fct){
	case 1:
		fct=&gen1;
		break;
	case 2:
		fct=&gen2;
		break;
	}
	// FUNCTION CALL
	int** output = (*fct)();
	int** conv = convert(output);
	// WRITE HEADER
	
	fprintf(out,"P2\n");
	fprintf(out,"%d",WIDTH);
	fprintf(out," ");
	fprintf(out,"%d",HEIGHT);
	fprintf(out,"\n");
	fprintf(out,"255\n");
	// WRITE IMAGE
	for(int y = 0; y < HEIGHT; y++){
		for(int x = 0; x < WIDTH; x++){
			/*switch((int)output[x][y]){
			case 0:
				fprintf(out,WATER);
				break;
			case 1:
				fprintf(out,GROUND);
				break;
			default:
				fprintf(out,HIGH);
				break;
			}*/
			fprintf(out,"%d",conv[x][y]);
			/*if(i<20){
				fprintf(out,);
			} else if(i < 45){
				fprintf(out,GROUND);
			} else {
				fprintf(out,HIGH);
			}*/
			if(x<WIDTH-1)
			{
				fprintf(out," ");
			}
		}
		fprintf(out,"\n");
	}
	fclose(out);
	return 0;
}
