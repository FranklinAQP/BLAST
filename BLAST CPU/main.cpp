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

//#define N (1048576) //1024 * 1024*5) //longitud cadena A para procesamiento en GPU
//#define M 300 //longitud cadena B para procesamiento en GPU

#define BLOCK_SIZE 16
#define SIZE_KEY 15 //tamaño de la key
#define SUCCESS 13 //minimo valor de coincidencias a considerar en la key
#define PENALTY -1

long long int M,N;

//probando que un elemento output es el resultado de la suma de su input con 14 posteriores
void semillero(char *A, char *B, bool *out){
	int result;
	cout<<"tamano cadena A: "<<N<<endl;
	cout<<"tamano cadena B: "<<M<<endl;

    for(long long int j=0; j<N; j=j+1 ){
        result  = 0;
        if(j%100000000==0)
            cout<<"j= "<<j<<endl;

        for (int i = 0 ; i < SIZE_KEY  ; ++i){
            //cout<<"B+i = pos "<<i<<" = "<<*(B+i)<<"  y  "<< "(*(A+j+i)) = pos " << j+i << "  " << (*(A+j+i)) << endl;
            if( (*(B+i)) == (*(A+j+i)) ){
                result = result + 1;
                //cout << (*(B+i)) << " == " << (*(A+j+i))<<endl;
            }

        }
        // Almacena los resultados
        //cout<<"almacenando datos"<<endl;
        if( result>=SUCCESS ){
            //cout<<"result = "<<result<<endl;
            //cout<<"*(out+j) = ";
            //cout<<*(out+j)<<endl;
            *(out+j)  = true;

            //cout<<"coincidencia en posicion: "<<j<<endl;
        }else{
            *(out+j)  = false;
        }


	}
}

int similarityScore(char *a, char *b){
    int result;
    if((*a)==(*b)){
        result=2;
    }else{
        result=PENALTY;
    }
    return result;
}

int findMax(int *traceback, int length, int  &index){
    int max = *traceback;
    index = 0;
    for(int i=1; i<length; ++i){
        if(*(traceback+i) > max){
            max = *(traceback+i);
            index=i;
        }
    }
    return max;
}

void Smith_Waterman_run(char *A, char *B, int n, bool *out, int *ind){
    if((*out)==0)
        return;
    int matrix[n][n];
    int matrix_max, i_max, j_max;
    int type_index; ///tipo de rastreo con mayor indice
    ///inicializar a 0s la matriz
    for(int i=0;i<=n;i++){
        for(int j=0;j<=n;j++){
            matrix[i][j]=0;
        }
    }
    int traceback[4]; ///Para rastrear el mayor posible valor que tomaria el indice local
    int I_i[n+1][n+1];///Guarda i de secuencia de generación de mayor indice
    int I_j[n+1][n+1];///Guarda j de secuencia de generación de mayor indice

    ///Comparación de cadenas con key SUCCESS
    for (int i=1;i<=n;i++){
        for(int j=0;j<=n;j++){
            traceback[0] = matrix[i-1][j-1]+similarityScore(A+i-1,B+j-1);
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
    /// imprime lo almacenado en consola
    for(int i=1;i<n;i++){
        for(int j=1;j<n;j++){
            printf("%d - ",matrix[i][j]);
        }
        printf("\n \n");
    }
    /// Encuentra el maximo score de la matriz
    matrix_max = 0;
    i_max=0; j_max=0;
    for(int i=1;i<n;i++){
        for(int j=1;j<n;j++){
            if(matrix[i][j]>matrix_max){
                matrix_max = matrix[i][j];
                i_max=i;
                j_max=j;
            }
        }
    }
    *(ind) = matrix_max;
    printf("Maximo escore es: %d \n \n",matrix_max);
}
/*
void Smith_Waterman(char *A, char *B, long long int n, int *out, int *ind){
    cout<<"Dentro de Smith waterman"<<endl;

    int matrix[n+1][n+1];
    int matrix_max, i_max, j_max;
    int type_index;
    cout<<"Se crearon variables de inicio"<<endl;
    ///inicializar a 0s la matriz
    printf ("Iniciando matriz a 0s\n");
    for(int i=0;i<=n;i++){
        for(int j=0;j<=n;j++){
            matrix[i][j]=0;
        }
    }

    int traceback[4];
    int I_i[n+1][n+1];
    int I_j[n+1][n+1];

    ///Comparación de cadenas con key SUCCESS
    //printf ("cadena grande = %d\n",N);
    //printf ("cadena corta = %d\n",M);
    printf ("Iniciando comparación\n");
    for (long long int k=0;k<N + SIZE_KEY -2 -M; ++k){
        if(*(out+k)>=SUCCESS){
            printf ("Iniciando comparación en posicion %d \n",k);
            for (long long int i=1;i<=n;i++){
                for(long long int j=0;j<=n;j++){
                    //cout <<"i: " << i << " -- " << "j: "<< j << seqA[i-1] << seqA[j-1] << endl;
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

            /// imprime lo almacenado en consola

            /// Encuentra el maximo score de la matriz
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
            *(ind+k) = matrix_max;
            printf("Maximo escore es: %d \n",matrix_max);

        }
    }


}
*/
void Smith_Waterman(char *A, char *B, long long int n, bool *out, int *ind){
    cout<<"Dentro de Smith waterman"<<endl;


    int **matrix = (int**) malloc( (n+1) * sizeof(int *) );

    for(long long int i = 0; i < n+1; i++)
    {
        matrix[i] = (int*) malloc( (n+1) * sizeof(int));
        if(matrix[i] == NULL)
        {
            fprintf(stderr, "No hay memoria suficiente\n");
            return;
        }
    }

    int matrix_max, i_max, j_max;
    int type_index;

    cout<<"Se crearon variables de inicio"<<endl;
    ///inicializar a 0s la matriz
    cout<<"Iniciando matriz a 0s con \n";
    for(int i=0;i<=n;i++){

        for(int j=0;j<=n;j++){
            matrix[i][j]=0;
        }
    }

    int traceback[4];
    int I_i[n+1][n+1];
    int I_j[n+1][n+1];

    ///Comparación de cadenas con key SUCCESS
    //printf ("cadena grande = %d\n",N);
    //printf ("cadena corta = %d\n",M);
    printf ("Iniciando comparación\n");
    for (long long int k=0;k<N + SIZE_KEY -2 -M; ++k){
        if(*(out+k)==true){
            printf ("Iniciando comparación en posicion %d \n",k);
            for (long long int i=1;i<=n;i++){
                for(long long int j=0;j<=n;j++){
                    //cout <<"i: " << i << " -- " << "j: "<< j << seqA[i-1] << seqA[j-1] << endl;
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

            /// imprime lo almacenado en consola
            /*
            cout<<"  ";
            for(int i=0;i<n;i++){
                cout<<*(B+i)<<" ";
            }
            cout<<endl;
            for(int i=0;i<n;i++){
                cout<<*(A+k+i)<<" ";
                for(int j=0;j<n;j++){
                    cout << matrix[i+1][j+1] << " ";
                }
                cout << endl;
            }
            */
            /// Encuentra el maximo score de la matriz
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
}

void fill_ints (int *x,int n) {
	fill_n(x, n, 0);
}

void fill_ADN (char *x, int n){

	char bases[4] = {'A','C','G','T'};
	int tmp;
	for(int i=0; i<n-1; ++i){
        tmp = rand()%4;
		*(x+i)=bases[tmp];
	}
}

void print_ADN(char *x, int n){
    printf("valor de cadena ADN: \n");
    for(int i=0; i<n; ++i){
		printf("%c",*(x+i));
	}
	printf(" \n \n");
}

int main(void)  {
	clock_t t;
	srand(time(NULL)); //Inicia random

	char *A, *B;
	bool *out;
	int *ind; // coincidencias e indice mayor de coincidencias Smith_waterman

	/*Codigo para probar cadenas de determinada longitufd*/
	/*long int sizeA  = (N + SIZE_KEY -1) *sizeof(char);
	long int sizeB  = (M) *sizeof(char);
	long int sizeout  = (N + SIZE_KEY -1) *sizeof(int);
	long int sizeind  = (N + SIZE_KEY -1) *sizeof(int);
	cout<<"size A = "<<sizeA<<"\n";
	cout<<"size B = "<<sizeB<<"\n";
	cout<<"size out = "<<sizeout<<"\n";
	cout<<"size ind = "<<sizeind<<"\n";


    cout<<"Generando punteros\n";
	A  = (char *)malloc(sizeA); fill_ADN(A, N + SIZE_KEY -1);
	cout<<"Cadena A generada\n";
	B  = (char *)malloc(sizeB); fill_ADN(B, M);
	cout<<"Cadena B generada\n";
	out = (int *)malloc(sizeout); fill_ints(out,  N + SIZE_KEY -1);
	cout<<"Cadena out generada\n";
	ind = (int *)malloc(sizeind); fill_ints(ind,  N + SIZE_KEY -1);
    cout<<"Cadena ind generada\n";


	cout<<"Iniciando semillero\n";
	semillero(A, B, out);
	*/
	/*Codigo para probar cadenas de determinada longitufd*/



	/*Codigo para cargar archivo*/
	string  a,b;
        char namefile_a[256];
        char namefile_b[256];
        char *fileA,*fileB;
        char base;
        char index_a[256], index_b[256];
/*
	cout<<"ingrese nombre del archivo A (a.txt): "<<endl;
    cin>>namefile_a;
    //cin.get (namefile_a,256);

    cout<<"ingrese nombre del archivo B (b.txt): "<<endl;
    //cin.get (namefile_b,256);
    cin>>namefile_b;
*/
    t = clock();

    cout<<"\nArchivo A leido:  " << namefile_a<<"\n";

    ifstream filea("a.txt", std::ifstream::binary);//namefile_a
    filea.seekg(0,filea.end);///Calcula el tamano del archivo
    long long int size_a = filea.tellg();
    cout<<"tamaño: "<< filea.tellg()<<endl;///Imprime al tamano
    filea.seekg(0,filea.beg);///Retorna el puntero al inicio del documento
    fileA = (char*) malloc( size_a * sizeof(char) );
    /*
    while (filea.get(base)){
        cout<<base;
        if(base=='\n')
            break;
    }*/

    filea.read( fileA, size_a );
    cout<<"Cadena: "<<endl;
    /*
    for(long long int h=0; h<size_a-1; ++h){
        if(h<100 || h>3151400000)
            cout << *(fileA+h);
    }*/
    filea.close();


    cout<<"\n\nArchivo B leido:  " << namefile_b<<"\n";

    ifstream fileb("b.txt", std::ifstream::binary);//namefile_b
    fileb.seekg(0,fileb.end);///Calcula el tamano del archivo


    long long int size_b = fileb.tellg();
    cout<<"tamano: "<<fileb.tellg()<<endl;///Imprime al tamano
    fileb.seekg(0,fileb.beg);
    /*
    long long int z =0;
    while (fileb.get(base)){
        cout<<base;
        ++z;
        if(base=='\n')
            break;

    }
    cout<<"\nz= "<<z<<endl;
    */

    //fileb.seekg(0,fileb.beg);///Retorna el puntero al inicio del documento

    fileB = (char*) malloc( size_b * sizeof(char) );

           // loop getting single characters
    fileb.read( fileB, size_b);//-z

    /*
    cout<<"Cadena: "<<endl;
    for(long long int h=0; h<size_b-z; ++h){
        cout << *(fileB+h);
    }
    cout<<"Cadena impresa: "<<endl;
    */

    fileb.close();
    /*Codigo para cargar archivo*/

    cout<<"pasando a semillar"<<endl;
    N = size_a -1;
    M = size_b -1;// -z;
    cout<<"creando variables  de almacenamiento de resultados"<<endl;
    long int sizeout  = (size_a -1+ SIZE_KEY) *sizeof(bool);
    cout<<"sizeout = "<< sizeout << endl;
    out = (bool*)malloc(sizeout); //fill_ints(out,  N + SIZE_KEY);
    //long long int sizeind  = (size_a -1 + SIZE_KEY) *sizeof(int);
    //ind = (int*)malloc(sizeind); //fill_ints(ind,  N + SIZE_KEY -1);
    if (out==NULL){
        free(fileA);  free(fileB); free(out); free(ind);
        cout<<"No se puede asignar memoria a la variable"<<endl;
        exit (1);
    }

    cout<<"Iniciando semillero\n";
	semillero(fileA, fileB, out);

	cout<<"Tiempo de semillero: "<<double(clock() - t)/double(CLOCKS_PER_SEC);
	printf ("\n Iniciando Smith Waterman\n");
    //Smith_Waterman(A, B, int (M), out, ind);

    Smith_Waterman(fileA, fileB, (long long int) (M), out, ind);
    printf ("\n Terminando Smith Waterman\n");
	t = clock() - t;

	//Imprimiendo
    //print_ADN(A,N + SIZE_KEY -1);
    //print_ADN(B,M);
	printf ("Tiempo t: %f seconds.\n", double(t)/double(CLOCKS_PER_SEC));

	//Cleanup
	free(fileA);  free(fileB); free(out); //free(ind);
    //free(A);  free(B); free(out); free(ind);

	return 0;
}

