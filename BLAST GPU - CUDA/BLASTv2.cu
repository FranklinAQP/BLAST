#include <iostream>
#include <stdio.h> //printf
#include <time.h> //time, time_t, struct tm, difftime, mktime, clock_t, clock, CLOCKS_PER_SEC
#include <stdlib.h> //srand, rand
#include <algorithm> //fill_n(iterator,size,val)

using namespace std;

#define N (1024 * 1024) //longitud cadena A para procesamiento en GPU
#define M 50 //longitud cadena B para procesamiento en GPU

#define BLOCK_SIZE 16
#define SIZE_KEY 15 //tamaño de la key
#define SUCCESS 7 //minimo valor de coincidencias a considerar en la key

//probando que un elemento output es el resultado de la suma de su input con 14 posteriores
__global__ void stencil_1d(int *A, int *B, int *out){
	__shared__ int temp[BLOCK_SIZE + SIZE_KEY -1];
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
		out[gindex]  = result;
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
	int *A, *B, *out; // host  copies  of a, b, c
	int *d_A, *d_B, *d_out; // device  copies  of a, b, c
	int sizeA  = (N + SIZE_KEY -1) *sizeof(int); 
	int sizeB  = (M) *sizeof(int); 
	

	// Alloc space  for host  copies  and  setup  values
	A  = (int *)malloc(sizeA); fill_ADN(A, N + SIZE_KEY -1);
	B  = (int *)malloc(sizeB); fill_ADN(B, M);
	out  = (int *)malloc(sizeA); fill_ints(out,  N + SIZE_KEY -1);

	// Alloc space  for device  copies
	cudaMalloc ((void **)&d_A,  sizeA);
	cudaMalloc ((void **)&d_B,  sizeB);
	cudaMalloc ((void **)&d_out,  sizeA);

	// Copy  to device
	cudaMemcpy (d_A, A, sizeA, cudaMemcpyHostToDevice);
	cudaMemcpy (d_B, B, sizeB, cudaMemcpyHostToDevice);
	cudaMemcpy (d_out, out, sizeA, cudaMemcpyHostToDevice);

	// Launch  stencil_1d()  kernel  on  GPU
	stencil_1d<<<N/BLOCK_SIZE,BLOCK_SIZE>>>(d_A, d_B, d_out);

	// Copy  result  back  to  host
	cudaMemcpy(out, d_out, sizeA, cudaMemcpyDeviceToHost);
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
