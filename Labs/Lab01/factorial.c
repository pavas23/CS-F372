#include<stdio.h>
#include<stdlib.h>

int main(int argc,char* argv[]){
    int n = atoi(argv[1]);
    unsigned long long factorial = 1;
    for(int i=1;i<=n;i++){
        factorial *= i;
    }
    printf("%lld\n",factorial);
    return 0;
}
