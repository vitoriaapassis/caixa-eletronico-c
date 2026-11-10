#include <stdio.h>

int main() {
    int notas[] = {100, 50, 20, 10, 5};
    int saque;
    float saldo = 1000;

    printf("Saldo: R$ %.2f\n", saldo);
    printf("Digite o valor do saque: ");
    scanf("%d", &saque);

    while (saque > saldo || saque <=0 ) {
        printf("Saldo insuficiente\n");
        return 0;
    }

    if (saque % 5 != 0) {
        printf("Valor invalido. O saque deve ser multiplo de 5\n");
        return 0;
    }

    printf("\nNotas entregues: \n");
    for (int i = 0; i < 5; i++) {
        if (saque / notas [i] > 0){
            printf("%d nota(s) de R$ %d\n", saque / notas[i], notas[i]);
            saque %= notas[i];
        }
    }

    return 0;
}
