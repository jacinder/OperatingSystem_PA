#include<stdio.h>
#include<stdlib.h>
int** prefix;
int count = 0;

void swap(int *a, int *b ){
	int tmp;
	tmp = *a;
	*a = *b;
	*b = tmp;
}

void print_arr(int size, int* arr){
	for(int i=0; i < size; i++)
		prefix[count][i]=arr[i];
    count++;
}

void permutation(int n, int r, int depth,int* arr){

	if(r == depth){
		print_arr(depth, arr);
		return;
	}
	for(int i=depth; i<n; i++){
		swap(&arr[i], &arr[depth]);
		permutation(n, r, depth+1, arr);
		swap(&arr[i], &arr[depth]);
	}

}

int main() {
    prefix = (int**)malloc(sizeof(int*)*24);
    for(int i=0;i<24;i++){
        prefix[i]=(int*)malloc(sizeof(int)*3);
    }
    int arr[] = {1,2,3,4};
	permutation(4, 3, 0, arr);
    printf("count : %d\n",count);
    for(int j=0;j<24;j++){
        for(int i=0;i<3;i++){
            printf("%d ", prefix[j][i]);
        }
        printf("\n");
    }
	return 0;
}
