#include <iostream>
#include <stdio.h> //printf
#include <time.h> //time, time_t, struct tm, difftime, mktime, clock_t, clock, CLOCKS_PER_SEC
#include <stdlib.h> //srand, rand
#include <algorithm> //fill_n(iterator,size,val)
#include "string.h"
#include <fstream>
#include <math.h>
#include <vector>

using namespace std;

//#define N (1024 * 1024) //longitud cadena A para procesamiento en GPU
//#define M 50 //longitud cadena B para procesamiento en GPU

#define BLOCK_SIZE 16
#define SIZE_KEY 15 //tamaño de la key
#define SUCCESS 7 //minimo valor de coincidencias a considerar en la key

long long int N, M;

//probando que un elemento output es el resultado de la suma de su input con 14 posteriores
__global__ void stencil_1d(char *A, char *B, bool *out){
	__shared__ char temp[BLOCK_SIZE + SIZE_KEY -1];
	int gindex = threadIdx.x + blockIdx.x *blockDim.x;
	int lindex = threadIdx.x;
	
	// Enviar elementos de entrada a la memoria compartida
	temp[lindex]  = A[gindex];
	if(threadIdx.x < SIZE_KEY-1) {
		temp[lindex + BLOCK_SIZE]  = A[gindex + BLOCK_SIZE];
	}

	// sincronizar  (Garantizar que todos los datos estén disponibles)
	__syncthreads();

	// Aplicar la plantilla
	int result  = 0;
	for (int i = 0 ; i < SIZE_KEY  ; ++i)
		if(*(B+i)==temp[lindex + i])		
			++result;

	// Almacena los resultados
	if(result>=SUCCESS)
		out[gindex]  = true;
}

__global__ void smith_waterman(char *A, char *B, bool *out){
	__shared__ char temp[M];
	int gindex = threadIdx.x + blockIdx.x *blockDim.x;
	int lindex = threadIdx.x;
	
	// Enviar elementos de entrada a la memoria compartida
	temp[lindex]  = A[gindex];
	if(*(out + gindex) ) {
		temp[lindex + M]  = A[gindex + M];
	}else{
		return;
	}

	// sincronizar  (Garantizar que todos los datos estén disponibles)
	__syncthreads();

	// Aplicar la plantilla

	int **matrix = (int**) malloc( (M+1) * sizeof(int *) );

    for(long long int i = 0; i < M+1; i++)
    {
	matrix[i] = (int*) malloc( (M+1) * sizeof(int));
	if(matrix[i] == NULL)
	{
	    fprintf(stderr, "No hay memoria suficiente\n");
	    return;
	}
    }

    cout<<"Iniciando matriz a 0s con \n";
    for(long long int i=0;i<=M;i++){

        for(long long int j=0;j<=M;j++){
            matrix[i][j]=0;
        }
    }

    int traceback[4];
    int I_i[M+1][M+1];
    int I_j[M+1][M+1];

    ///Comparación de cadenas con key SUCCESS
    printf ("Iniciando comparación\n");
    for (long long int k=0;k<N + SIZE_KEY -2 -M; ++k){
        if(*(out+k)==true){
            printf ("Iniciando comparación en posicion %d \n",k);
            for (long long int i=1;i<=n;i++){
                for(long long int j=0;j<=n;j++){
                    traceback[0] = matrix[i-1][j-1]+similarityScore(A+k+i-1,B+j-1);
                    traceback[1] = matrix[i-1][j]+PENALTY;
                    traceback[2] = matrix[i][j-1]+PENALTY;
                    traceback[3] = 0;
                    matrix[i][j] = findMax(&(traceback[0]),4, type_index);
                    switch(type_index){
                        case 0:
                            I_i[i][j] = i-1;
                            I_j[i][j] = j-1;
                            break;
                        case 1:
                            I_i[i][j] = i-1;
                            I_j[i][j] = j;
                            break;
                        case 2:
                            I_i[i][j] = i;
                            I_j[i][j] = j-1;
                            break;
                        case 3:
                            I_i[i][j] = i;
                            I_j[i][j] = j;
                            break;
                    }
                }
            }

            matrix_max = 0;
            int i_max=0, j_max=0;
            for(long long int i=1;i<n;i++){
                for(long long int j=1;j<n;j++){
                    if(matrix[i][j]>matrix_max){
                        matrix_max = matrix[i][j];
                        i_max=i;
                        j_max=j;
                    }
                }
            }
            //*(ind+k) = matrix_max;
            printf("Maximo escore es: %d \n",matrix_max);

        }
    }

    for(long long int i = 0; i < n+1; i++)
    {
        free (matrix[i]);
    }
    free (matrix);

	int result  = 0;
	for (int i = 0 ; i < SIZE_KEY  ; ++i)
		if(*(B+i)==temp[lindex + i])		
			++result;

	// Almacena los resultados
	if(result>=SUCCESS)
		out[gindex]  = true;
}


void fill_ints (int *x,int n) {
	fill_n(x,  n, 1);
}

void fill_ADN (int *x, int n){
	srand(time(NULL)); //Inicia random	
	for(int i=0; i<n; ++i){
		*(x+i)=rand()%4;
	}
}   

int main(void)  {
	clock_t t;	
	t = clock();
	//time_t antes, despues;	
	//double seconds;
	//time(&antes);

	string  a,b;
        char namefile_a[256];
        char namefile_b[256];
        char *A,*B; bool *out;
        char base;
        char index_a[256], index_b[256];
	
	/*Codigo para cargar archivo*/
	cout<<"\nArchivo A leido:  " << namefile_a<<"\n";
    	ifstream filea("a.txt", std::ifstream::binary);//namefile_a
    	filea.seekg(0,filea.end);///Calcula el tamano del archivo
    	long long int size_a = filea.tellg();
    	cout<<"tamaño: "<< filea.tellg()<<endl;///Imprime al tamano
    	filea.seekg(0,filea.beg);///Retorna el puntero al inicio del documento
    	A = (char*) malloc( size_a * sizeof(char) );
    	filea.read( A, size_a );
    	cout<<"Cadena: "<<endl;
    	filea.close();

	cout<<"\n\nArchivo B leido:  " << namefile_b<<"\n";
    	ifstream fileb("b.txt", std::ifstream::binary);//namefile_b
    	fileb.seekg(0,fileb.end);///Calcula el tamano del archivo
    	long long int size_b = fileb.tellg();
    	cout<<"tamano: "<<fileb.tellg()<<endl;///Imprime al tamano
    	fileb.seekg(0,fileb.beg);
	B = (char*) malloc( size_b * sizeof(char) );
    	fileb.read( B, size_b);//-z
    	fileb.close();
    	/*Codigo para cargar archivo*/
	
	cout<<"actualizando variables"<<endl;
    	N = size_a -1;
    	M = size_b -1;
	cout<<"creando variables  de almacenamiento de resultados"<<endl;	

    	long long int sizeout = (size_a -1+ SIZE_KEY) *sizeof(bool);
    	out = (bool*)malloc(sizeout); //fill_ints(out,  N + SIZE_KEY);


	//int *A, *B; bool *out; // host  copies  of a, b, c
	char *d_A, *d_B; bool *d_out; // device  copies  of a, b, c
	
	// Alloc space  for device  copies
	cudaMalloc ((void **)&d_A,  N);
	cudaMalloc ((void **)&d_B,  M);
	cudaMalloc ((void **)&d_out, N);

	// Copy  to device
	cudaMemcpy (d_A, A, N, cudaMemcpyHostToDevice);
	cudaMemcpy (d_B, B, M, cudaMemcpyHostToDevice);
	cudaMemcpy (d_out, out, N, cudaMemcpyHostToDevice);

	// Launch  stencil_1d()  kernel  on  GPU
	cout<< "iniciando semillero"<<endl;
	stencil_1d<<<N/BLOCK_SIZE,BLOCK_SIZE>>>(d_A, d_B, d_out);

	// Copy  result  back  to  host
	cudaMemcpy(out, d_out, N, cudaMemcpyDeviceToHost);
	cout<< "terminando semillero"<<endl;
	//time(&despues);
	t = clock() - t;
	//seconds = double(despues-antes);
	//Imprimiendo
/*
	for(int i=0; i<N + SIZE_KEY -1; ++i){
		printf("valor out en pos %d es %d \n",i,*(out+i));
	}
*/
	printf ("Tiempo t: %f seconds.\n", double(t)/double(CLOCKS_PER_SEC));
	//printf ("Tiempo usado: %f seconds.\n", seconds);
	
	//Cleanup
	free(A);  free(out);
	cudaFree(d_A);cudaFree(d_out);
	return 0;
}
