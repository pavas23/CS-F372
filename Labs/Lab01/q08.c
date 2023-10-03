#include<stdio.h>
#include<stdlib.h>

// argc is ARGument Count, which counts the number of arguments passed including the name of the program, so if we pass one argument, argc will be 2
// argv is ARGument Vector which is array of character pointers, listing all the arguments
// argv[0] is name of the program

int main(int argc,char* argv[]){
    int n = atoi(argv[1]);
    printf("Number of arguments passed are: %d\n",argc);
    for(int i=0;i<n;i++){
        printf("Hello World\n");
    }
    return 0;
}

