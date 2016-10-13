#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

//const int WIDTH = 2000;
//const int HEIGHT = 2000;

// POUR GEN1 : 2pow11 +1

const int WIDTH = 257;
const int HEIGHT = 257;
const int DEVIATION = 40;

const char* WATER = "0 20 255";
const char* GROUND = "0 100 0";
const char* HIGH = "128 128 128";

int _rand(int min, int max){
	if(min == 0 && max == 0) { return 0; }
	int i = rand()%(max-min+1);
	i+=min;
	return i;
}

void gen1rec(float** t,int x, int y, int mx, int my,int dev)
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

	int tmp = (y - ((my-y)/2));

	int e = tmp>=0 ? t[hx][tmp] : 0;

	tmp = (x + ((mx-x)/2));

	int f = tmp<WIDTH ? t[tmp][hy] : 0;

	tmp = (x - ((mx-x)/2));

	int g = tmp>=0 ? t[tmp][hy] : 0;

	tmp = (y + ((my-y)/2));

	int h = tmp<HEIGHT ? t[hx][tmp] : 0;

	// Update values
	
	int ce = (a + b + c + d)/4 + _rand(-dev,dev);

	t[hx][hy] = ce;
	
	t[x][hy] = (ce + a + g + c)/4 + _rand(-dev,dev);

	t[hx][y] = (a + e + b + ce)/4 + _rand(-dev,dev);

	t[mx][hy] = (b + f + d + ce)/4 + _rand(-dev,dev);

	t[hx][my] = (ce + c + d + h)/4 + _rand(-dev,dev);
    
	if(mx-x>2)
	{	   
		gen1rec(t,x,y,(x+mx)/2,(y+my)/2,dev/2);
		gen1rec(t,(x+mx)/2,y,mx,(y+my)/2,dev/2);
		gen1rec(t,x,(y+my)/2,(x+mx)/2,my,dev/2);
		gen1rec(t,(x+mx)/2,(y+my)/2,mx,my,dev/2); 
	}
}

float** gen1(){
	float** tab = malloc(sizeof(float*)*WIDTH);
	for(int i = 0; i < WIDTH; i++)
	{
		tab[i] = malloc(sizeof(float)*HEIGHT);
		/*for(int j; j < HEIGHT; j++)
		{
			tab[i][j]=0;
		}*/
	} 
	tab[0][0]=_rand(-DEVIATION,DEVIATION);
	tab[WIDTH-1][0]=_rand(-DEVIATION,DEVIATION);
	tab[0][HEIGHT-1]=_rand(-DEVIATION,DEVIATION);
	tab[WIDTH-1][HEIGHT-1]=_rand(-DEVIATION,DEVIATION);
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

int main(int argc, char** argv){
	int seed = atoi(argv[2]);
	srand(seed);
	FILE* out = fopen("output.pgm","w");
	if(argc!=3)
	{
		fprintf(stderr,"Usage : ./main MODE SEED\n");
		return -1;
	}
	float** (*fct)();
	int nb_fct = atoi(argv[1]);
	switch(nb_fct){
	case 1:
		fct=&gen1;
		break;
	}
	// FUNCTION CALL
	float** output = (*fct)();
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
			int i = (int)output[x][y];
			fprintf(out,"%d",i>=0?i*40:0);
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
