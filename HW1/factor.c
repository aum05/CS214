#include <stdlib.h>
#include <stdio.h>
#include <math.h>

void factorization (int n) {
    // dividing n by the smallest prime number
    // this loop will only be executed if n is even
    while (n % 2 == 0)
    {
        printf("%d ", 2);
        n /= 2;
    }
    
    // when the program gets to this loop,
    // n has become, or already was, odd.
    // Since, dividing it by 1 has no point, we start the loop from i = 3.
    for (int i = 3; i < sqrt(n); i = i+2)
    {
        while (n % i == 0)
        {
            printf("%d ", i);
            n /= i;
        }
        
    }

    // this last condition handles the case where n is prime number greater than 2
    if (n > 2){
        printf("%d ", n);
    }
}

int main(int argc, char* argv[argc + 1]){
    int n = argv[1];
    factorization(n);
    return EXIT_SUCCESS;
}