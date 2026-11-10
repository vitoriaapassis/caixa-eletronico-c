/*
 * CAIXA ELETRONICO v2.0
 * Evolucao da atividade do caixa eletronico com o conteudo das aulas 10 a 15.
 *
 * Mapa dos conceitos aplicados:
 *  Aula 10 (vetores, matrizes, strings): matriz de cedulas [2][5] (valor e
 *          estoque), vetores de contas e transacoes, strcmp/strcpy/strlen/fgets.
 *  Aula 11 (arquivos): persistencia em contas.txt, extrato.txt e cedulas.txt
 *          (fopen, fprintf, fscanf, fgets, fclose, modos "r", "w" e "a").
 *  Aula 12 (funcoes, modularizacao, structs): struct Conta e struct Transacao,
 *          programa dividido em funcoes de responsabilidade unica, prototipos.
 *  Aula 13 (ponteiros): passagem por referencia (struct Conta *), retorno de
 *          multiplos valores por ponteiro, operador ->, funcao trocar.
 *  Aula 14 (Bubble Sort): extrato ordenado por valor, com flag de otimizacao
 *          e contagem de comparacoes e trocas.
 *  Aula 15 (recursividade e Quick Sort): distribuicao de cedulas recursiva e
 *          relatorio de contas ordenado por saldo com Quick Sort.
 *
 * Compilar: gcc -Wall -o caixa caixa_eletronico.c
 * Contas de teste (criadas na primeira execucao):
 *   conta 1001 senha 1234 | conta 1002 senha 4321 | conta 1003 senha 1111
 * Senha da area administrativa: 9999
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_CONTAS      50
#define MAX_TRANS       500
#define NUM_NOTAS       5
#define MAX_TENTATIVAS  3
#define SENHA_ADMIN     9999
#define ESTOQUE_INICIAL 10

#define ARQ_CONTAS  "contas.txt"
#define ARQ_EXTRATO "extrato.txt"
#define ARQ_CEDULAS "cedulas.txt"

/* ---------- Aula 12: structs ---------- */
struct Conta {
    int numero;
    int senha;
    float saldo;
    char titular[50];
};

struct Transacao {
    int conta;
    char tipo[15];   /* SAQUE, DEPOSITO, TRANSF_ENV, TRANSF_REC */
    float valor;
};

/* ---------- Prototipos (Aula 12) ---------- */
/* entrada de dados */
int  ler_linha(char *buf, int n);
int  ler_inteiro(const char *msg, int *destino);
int  ler_valor(const char *msg, float *destino);

/* arquivos (Aula 11) */
void carregar_contas(struct Conta contas[], int *total);
void salvar_contas(const struct Conta contas[], int total);
void carregar_cedulas(int cedulas[2][NUM_NOTAS]);
void salvar_cedulas(int cedulas[2][NUM_NOTAS]);
void registrar_transacao(int conta, const char *tipo, float valor);
int  carregar_transacoes(int conta, struct Transacao trans[]);

/* operacoes bancarias (Aula 13: ponteiros) */
int  buscar_conta(const struct Conta contas[], int total, int numero);
int  autenticar(struct Conta contas[], int total);
int  distribuir_notas(int valor, int cedulas[2][NUM_NOTAS], int usadas[], int i);
void sacar(struct Conta *c, int cedulas[2][NUM_NOTAS]);
void depositar(struct Conta *c);
void transferir(struct Conta *origem, struct Conta contas[], int total);

/* ordenacao (Aulas 13, 14 e 15) */
void trocar_conta(struct Conta *a, struct Conta *b);
void trocar_transacao(struct Transacao *a, struct Transacao *b);
void bubble_sort_transacoes(struct Transacao arr[], int n, int *comparacoes, int *trocas);
int  particionar(struct Conta arr[], int baixo, int alto);
void quick_sort_contas(struct Conta arr[], int baixo, int alto);

/* telas e menus */
void exibir_extrato(const struct Conta *c);
void menu_cliente(struct Conta *c, struct Conta contas[], int total, int cedulas[2][NUM_NOTAS]);
void menu_admin(struct Conta contas[], int *total, int cedulas[2][NUM_NOTAS]);
void relatorio_contas(const struct Conta contas[], int total);
void cadastrar_conta(struct Conta contas[], int *total);
void exibir_cedulas(int cedulas[2][NUM_NOTAS]);

/* =====================================================================
 * ENTRADA DE DADOS (Aula 10: strings com fgets, sem buffer overflow)
 * ===================================================================== */
int ler_linha(char *buf, int n) {
    if (fgets(buf, n, stdin) == NULL) return 0;
    buf[strcspn(buf, "\n")] = '\0';
    return 1;
}

/* Retorna 1 se leu um inteiro valido; o valor sai por ponteiro (Aula 13) */
int ler_inteiro(const char *msg, int *destino) {
    char linha[50];
    char *fim;
    printf("%s", msg);
    if (!ler_linha(linha, sizeof(linha))) return 0;
    long v = strtol(linha, &fim, 10);
    if (fim == linha || *fim != '\0') {
        printf("Entrada invalida.\n");
        return 0;
    }
    *destino = (int)v;
    return 1;
}

int ler_valor(const char *msg, float *destino) {
    char linha[50];
    char lixo;
    printf("%s", msg);
    if (!ler_linha(linha, sizeof(linha))) return 0;
    if (sscanf(linha, "%f %c", destino, &lixo) == 1 && *destino > 0) return 1;
    printf("Valor invalido.\n");
    return 0;
}

/* =====================================================================
 * ARQUIVOS (Aula 11)
 * ===================================================================== */
void carregar_contas(struct Conta contas[], int *total) {
    FILE *arq = fopen(ARQ_CONTAS, "r");
    *total = 0;

    if (arq == NULL) {
        /* Primeira execucao: cria contas de exemplo */
        struct Conta padrao[3] = {
            {1001, 1234, 1000.00f, "Ana Paula Souza"},
            {1002, 4321, 2500.50f, "Bruno Lima"},
            {1003, 1111,  300.00f, "Carla Mendes"}
        };
        for (int i = 0; i < 3; i++) contas[i] = padrao[i];
        *total = 3;
        salvar_contas(contas, *total);
        return;
    }

    while (*total < MAX_CONTAS &&
           fscanf(arq, "%d %d %f %49[^\n]",
                  &contas[*total].numero, &contas[*total].senha,
                  &contas[*total].saldo, contas[*total].titular) == 4) {
        (*total)++;
    }
    fclose(arq);
}

void salvar_contas(const struct Conta contas[], int total) {
    FILE *arq = fopen(ARQ_CONTAS, "w");
    if (arq == NULL) {
        printf("ERRO: nao foi possivel salvar as contas!\n");
        return;
    }
    for (int i = 0; i < total; i++) {
        fprintf(arq, "%d %d %.2f %s\n",
                contas[i].numero, contas[i].senha, contas[i].saldo, contas[i].titular);
    }
    fclose(arq);
}

/* Matriz de cedulas: linha 0 = valor da nota, linha 1 = estoque (Aula 10) */
void carregar_cedulas(int cedulas[2][NUM_NOTAS]) {
    int valores[NUM_NOTAS] = {100, 50, 20, 10, 5};
    for (int j = 0; j < NUM_NOTAS; j++) {
        cedulas[0][j] = valores[j];
        cedulas[1][j] = ESTOQUE_INICIAL;
    }

    FILE *arq = fopen(ARQ_CEDULAS, "r");
    if (arq == NULL) return;   /* sem arquivo: usa estoque inicial */

    for (int j = 0; j < NUM_NOTAS; j++) {
        int valor, qtd;
        if (fscanf(arq, "%d %d", &valor, &qtd) == 2 && valor == cedulas[0][j]) {
            cedulas[1][j] = qtd;
        }
    }
    fclose(arq);
}

void salvar_cedulas(int cedulas[2][NUM_NOTAS]) {
    FILE *arq = fopen(ARQ_CEDULAS, "w");
    if (arq == NULL) {
        printf("ERRO: nao foi possivel salvar o estoque de cedulas!\n");
        return;
    }
    for (int j = 0; j < NUM_NOTAS; j++) {
        fprintf(arq, "%d %d\n", cedulas[0][j], cedulas[1][j]);
    }
    fclose(arq);
}

/* Modo "a": acrescenta ao final sem apagar o historico */
void registrar_transacao(int conta, const char *tipo, float valor) {
    FILE *arq = fopen(ARQ_EXTRATO, "a");
    if (arq == NULL) {
        printf("ERRO: nao foi possivel registrar a transacao!\n");
        return;
    }
    fprintf(arq, "%d %s %.2f\n", conta, tipo, valor);
    fclose(arq);
}

/* Devolve quantas transacoes da conta foram lidas */
int carregar_transacoes(int conta, struct Transacao trans[]) {
    FILE *arq = fopen(ARQ_EXTRATO, "r");
    int n = 0;
    struct Transacao t;
    if (arq == NULL) return 0;

    while (n < MAX_TRANS &&
           fscanf(arq, "%d %14s %f", &t.conta, t.tipo, &t.valor) == 3) {
        if (t.conta == conta) trans[n++] = t;
    }
    fclose(arq);
    return n;
}

/* =====================================================================
 * BUSCA E AUTENTICACAO
 * ===================================================================== */
int buscar_conta(const struct Conta contas[], int total, int numero) {
    for (int i = 0; i < total; i++) {
        if (contas[i].numero == numero) return i;
    }
    return -1;
}

/* Retorna o indice da conta autenticada ou -1. Maximo de 3 tentativas. */
int autenticar(struct Conta contas[], int total) {
    int numero, senha;
    if (!ler_inteiro("Numero da conta: ", &numero)) return -1;

    int idx = buscar_conta(contas, total, numero);
    if (idx == -1) {
        printf("Conta nao encontrada.\n");
        return -1;
    }

    for (int t = 1; t <= MAX_TENTATIVAS; t++) {
        if (!ler_inteiro("Senha: ", &senha)) senha = -1;
        if (senha == contas[idx].senha) return idx;
        printf("Senha incorreta (%d/%d).\n", t, MAX_TENTATIVAS);
    }
    printf("Numero maximo de tentativas excedido. Cartao bloqueado nesta sessao.\n");
    return -1;
}

/* =====================================================================
 * RECURSIVIDADE (Aula 15): distribuicao de cedulas
 * Caso base: percorreu todas as notas. Retorna 1 se o valor zerou.
 * Caso recursivo: usa o maximo possivel da nota i (respeitando o estoque)
 * e resolve o restante com as notas menores.
 * ===================================================================== */
int distribuir_notas(int valor, int cedulas[2][NUM_NOTAS], int usadas[], int i) {
    if (i == NUM_NOTAS) {
        return valor == 0;
    }
    int qtd = valor / cedulas[0][i];
    if (qtd > cedulas[1][i]) qtd = cedulas[1][i];
    usadas[i] = qtd;
    return distribuir_notas(valor - qtd * cedulas[0][i], cedulas, usadas, i + 1);
}

/* =====================================================================
 * OPERACOES (Aula 13: o ponteiro permite alterar a conta original)
 * ===================================================================== */
void sacar(struct Conta *c, int cedulas[2][NUM_NOTAS]) {
    int valor;
    int usadas[NUM_NOTAS] = {0};

    printf("Saldo atual: R$ %.2f\n", c->saldo);
    if (!ler_inteiro("Valor do saque: R$ ", &valor)) return;

    if (valor <= 0) {
        printf("Valor invalido.\n");
        return;
    }
    if (valor > c->saldo) {
        printf("Saldo insuficiente.\n");
        return;
    }
    if (valor % 5 != 0) {
        printf("Valor invalido. O saque deve ser multiplo de 5.\n");
        return;
    }
    if (!distribuir_notas(valor, cedulas, usadas, 0)) {
        printf("Caixa sem cedulas suficientes para este valor. Tente outro valor.\n");
        return;
    }

    /* Saque aprovado: atualiza saldo, estoque e arquivos */
    c->saldo -= valor;
    printf("\nNotas entregues:\n");
    for (int j = 0; j < NUM_NOTAS; j++) {
        if (usadas[j] > 0) {
            printf("%d nota(s) de R$ %d\n", usadas[j], cedulas[0][j]);
            cedulas[1][j] -= usadas[j];
        }
    }
    printf("Novo saldo: R$ %.2f\n", c->saldo);
    registrar_transacao(c->numero, "SAQUE", (float)valor);
    salvar_cedulas(cedulas);
}

void depositar(struct Conta *c) {
    float valor;
    if (!ler_valor("Valor do deposito: R$ ", &valor)) return;
    c->saldo += valor;
    printf("Deposito realizado. Novo saldo: R$ %.2f\n", c->saldo);
    registrar_transacao(c->numero, "DEPOSITO", valor);
}

void transferir(struct Conta *origem, struct Conta contas[], int total) {
    int numero;
    float valor;

    if (!ler_inteiro("Conta de destino: ", &numero)) return;
    if (numero == origem->numero) {
        printf("Nao e possivel transferir para a propria conta.\n");
        return;
    }
    int idx = buscar_conta(contas, total, numero);
    if (idx == -1) {
        printf("Conta de destino nao encontrada.\n");
        return;
    }
    if (!ler_valor("Valor da transferencia: R$ ", &valor)) return;
    if (valor > origem->saldo) {
        printf("Saldo insuficiente.\n");
        return;
    }

    struct Conta *destino = &contas[idx];
    origem->saldo  -= valor;
    destino->saldo += valor;
    printf("Transferencia de R$ %.2f para %s concluida.\n", valor, destino->titular);
    registrar_transacao(origem->numero,  "TRANSF_ENV", valor);
    registrar_transacao(destino->numero, "TRANSF_REC", valor);
}

/* =====================================================================
 * ORDENACAO
 * ===================================================================== */
/* Aula 13: troca por referencia */
void trocar_conta(struct Conta *a, struct Conta *b) {
    struct Conta temp = *a;
    *a = *b;
    *b = temp;
}

void trocar_transacao(struct Transacao *a, struct Transacao *b) {
    struct Transacao temp = *a;
    *a = *b;
    *b = temp;
}

/* Aula 14: Bubble Sort otimizado, ordem decrescente de valor.
 * Comparacoes e trocas voltam por ponteiro (retorno de multiplos valores). */
void bubble_sort_transacoes(struct Transacao arr[], int n, int *comparacoes, int *trocas) {
    *comparacoes = 0;
    *trocas = 0;
    for (int i = 0; i < n - 1; i++) {
        int trocou = 0;
        for (int j = 0; j < n - i - 1; j++) {
            (*comparacoes)++;
            if (arr[j].valor < arr[j + 1].valor) {
                trocar_transacao(&arr[j], &arr[j + 1]);
                (*trocas)++;
                trocou = 1;
            }
        }
        if (!trocou) break;
    }
}

/* Aula 15: particionamento (pivo = primeiro elemento), ordem decrescente de saldo */
int particionar(struct Conta arr[], int baixo, int alto) {
    float pivot = arr[baixo].saldo;
    int i = baixo;
    for (int j = baixo + 1; j <= alto; j++) {
        if (arr[j].saldo > pivot) {
            i++;
            trocar_conta(&arr[i], &arr[j]);
        }
    }
    trocar_conta(&arr[baixo], &arr[i]);
    return i;
}

void quick_sort_contas(struct Conta arr[], int baixo, int alto) {
    if (baixo < alto) {                       /* caso base: tamanho 0 ou 1 */
        int p = particionar(arr, baixo, alto);
        quick_sort_contas(arr, baixo, p - 1);
        quick_sort_contas(arr, p + 1, alto);
    }
}

/* =====================================================================
 * TELAS
 * ===================================================================== */
void exibir_extrato(const struct Conta *c) {
    struct Transacao trans[MAX_TRANS];
    int n = carregar_transacoes(c->numero, trans);
    int opcao = 1;

    if (n == 0) {
        printf("Nenhuma movimentacao registrada.\n");
        return;
    }

    printf("1 - Ordem cronologica\n2 - Ordenado por valor (maior para menor)\n");
    if (!ler_inteiro("Opcao: ", &opcao)) return;

    if (opcao == 2) {
        int comp, trocas;
        bubble_sort_transacoes(trans, n, &comp, &trocas);
        printf("(Bubble Sort: %d comparacoes, %d trocas)\n", comp, trocas);
    }

    printf("\n--- EXTRATO CONTA %d ---\n", c->numero);
    printf("%-3s %-12s %12s\n", "N", "Tipo", "Valor (R$)");
    for (int i = 0; i < n; i++) {
        printf("%-3d %-12s %12.2f\n", i + 1, trans[i].tipo, trans[i].valor);
    }
    printf("Saldo atual: R$ %.2f\n", c->saldo);
}

void menu_cliente(struct Conta *c, struct Conta contas[], int total, int cedulas[2][NUM_NOTAS]) {
    int opcao;
    printf("\nBem-vindo(a), %s!\n", c->titular);
    do {
        printf("\n===== MENU =====\n");
        printf("1 - Consultar saldo\n2 - Sacar\n3 - Depositar\n");
        printf("4 - Transferir\n5 - Extrato\n0 - Sair\n");
        if (!ler_inteiro("Opcao: ", &opcao)) { opcao = -1; continue; }

        switch (opcao) {
            case 1: printf("Saldo: R$ %.2f\n", c->saldo); break;
            case 2: sacar(c, cedulas); break;
            case 3: depositar(c); break;
            case 4: transferir(c, contas, total); break;
            case 5: exibir_extrato(c); break;
            case 0: printf("Encerrando sessao...\n"); break;
            default: printf("Opcao invalida.\n");
        }
        salvar_contas(contas, total);   /* persiste apos cada operacao */
    } while (opcao != 0);
}

void exibir_cedulas(int cedulas[2][NUM_NOTAS]) {
    int total = 0;
    printf("\n--- ESTOQUE DE CEDULAS ---\n");
    for (int j = 0; j < NUM_NOTAS; j++) {
        printf("R$ %3d: %3d nota(s)\n", cedulas[0][j], cedulas[1][j]);
        total += cedulas[0][j] * cedulas[1][j];
    }
    printf("Total no caixa: R$ %d\n", total);
}

/* Quick Sort sobre uma COPIA, para nao alterar a ordem original das contas */
void relatorio_contas(const struct Conta contas[], int total) {
    struct Conta copia[MAX_CONTAS];
    float soma = 0;

    for (int i = 0; i < total; i++) copia[i] = contas[i];
    quick_sort_contas(copia, 0, total - 1);

    printf("\n--- CONTAS POR SALDO (Quick Sort) ---\n");
    printf("%-6s %-25s %12s\n", "Conta", "Titular", "Saldo (R$)");
    for (int i = 0; i < total; i++) {
        printf("%-6d %-25s %12.2f\n", copia[i].numero, copia[i].titular, copia[i].saldo);
        soma += copia[i].saldo;
    }
    printf("Total em depositos: R$ %.2f\n", soma);
}

void cadastrar_conta(struct Conta contas[], int *total) {
    struct Conta nova;
    int numero, senha;

    if (*total >= MAX_CONTAS) {
        printf("Limite de contas atingido.\n");
        return;
    }
    if (!ler_inteiro("Numero da nova conta: ", &numero)) return;
    if (numero <= 0 || buscar_conta(contas, *total, numero) != -1) {
        printf("Numero invalido ou ja existente.\n");
        return;
    }
    if (!ler_inteiro("Senha (4 digitos): ", &senha)) return;
    if (senha < 1000 || senha > 9999) {
        printf("A senha deve ter 4 digitos.\n");
        return;
    }
    printf("Nome do titular: ");
    if (!ler_linha(nova.titular, sizeof(nova.titular)) || strlen(nova.titular) == 0) {
        printf("Nome invalido.\n");
        return;
    }

    nova.numero = numero;
    nova.senha = senha;
    nova.saldo = 0;
    contas[*total] = nova;
    (*total)++;
    salvar_contas(contas, *total);
    printf("Conta %d criada para %s.\n", nova.numero, nova.titular);
}

void menu_admin(struct Conta contas[], int *total, int cedulas[2][NUM_NOTAS]) {
    int opcao, senha;
    if (!ler_inteiro("Senha administrativa: ", &senha) || senha != SENHA_ADMIN) {
        printf("Acesso negado.\n");
        return;
    }
    do {
        printf("\n===== ADMINISTRACAO =====\n");
        printf("1 - Relatorio de contas por saldo\n2 - Estoque de cedulas\n");
        printf("3 - Cadastrar conta\n4 - Reabastecer cedulas\n0 - Voltar\n");
        if (!ler_inteiro("Opcao: ", &opcao)) { opcao = -1; continue; }

        switch (opcao) {
            case 1: relatorio_contas(contas, *total); break;
            case 2: exibir_cedulas(cedulas); break;
            case 3: cadastrar_conta(contas, total); break;
            case 4:
                for (int j = 0; j < NUM_NOTAS; j++) cedulas[1][j] = ESTOQUE_INICIAL;
                salvar_cedulas(cedulas);
                printf("Caixa reabastecido (%d notas de cada valor).\n", ESTOQUE_INICIAL);
                break;
            case 0: break;
            default: printf("Opcao invalida.\n");
        }
    } while (opcao != 0);
}

/* =====================================================================
 * MAIN: apenas coordena as funcoes (decomposicao, Aula 12)
 * ===================================================================== */
int main(void) {
    struct Conta contas[MAX_CONTAS];
    int total;
    int cedulas[2][NUM_NOTAS];
    int opcao;

    carregar_contas(contas, &total);
    carregar_cedulas(cedulas);

    printf("=================================\n");
    printf("      CAIXA ELETRONICO v2.0\n");
    printf("=================================\n");

    do {
        printf("\n1 - Acessar conta\n2 - Area administrativa\n0 - Sair\n");
        if (!ler_inteiro("Opcao: ", &opcao)) {
            if (feof(stdin)) break;
            opcao = -1;
            continue;
        }
        switch (opcao) {
            case 1: {
                int idx = autenticar(contas, total);
                if (idx != -1) menu_cliente(&contas[idx], contas, total, cedulas);
                break;
            }
            case 2: menu_admin(contas, &total, cedulas); break;
            case 0: printf("Obrigado por utilizar o caixa eletronico!\n"); break;
            default: printf("Opcao invalida.\n");
        }
    } while (opcao != 0);

    salvar_contas(contas, total);
    salvar_cedulas(cedulas);
    return 0;
}
