#include <stdlib.h>
#include <stdio.h>

/*
fact10.c

Calcula el factorial de 10 e imprime el resultado
*/

int factorial(int x) {
  if(x<=1)
    return 1;
  else
    return x*factorial(x-1);
}

int main() {
    int x = factorial(10);
    printf("f(10)=%i\n",x);
}