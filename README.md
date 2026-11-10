# Caixa Eletrônico em C


Simulador de caixa eletrônico em linguagem C, executado no terminal. O projeto nasceu como um
exercício simples (um saque com contagem de cédulas) e evoluiu para um sistema com várias contas,
persistência em arquivos, extrato ordenado e área administrativa.

## Funcionalidades

- Login por número da conta e senha (máximo de 3 tentativas)
- Saque com distribuição de cédulas (R$ 100, 50, 20, 10 e 5) respeitando o estoque do caixa
- Depósito e transferência entre contas
- Extrato em ordem cronológica ou ordenado por valor
- Área administrativa: relatório de contas por saldo, estoque de cédulas, cadastro de contas e reabastecimento
- Dados salvos em arquivos de texto, mantidos entre execuções

## Como executar

Requisitos: `gcc` e `make`.

```bash
make        # compila
make run    # compila e executa
make test   # roda os testes automáticos
make clean  # remove o executável e os arquivos de dados
```

Sem `make`: `gcc -Wall -Wextra -o caixa src/caixa_eletronico.c && ./caixa`

### Contas de teste (criadas na primeira execução)

| Conta | Senha | Titular         | Saldo inicial |
|-------|-------|-----------------|---------------|
| 1001  | 1234  | Ana Paula Souza | R$ 1000,00    |
| 1002  | 4321  | Bruno Lima      | R$ 2500,50    |
| 1003  | 1111  | Carla Mendes    | R$ 300,00     |

Senha da área administrativa: `9999`

## Evolução do projeto

| Versão | Arquivo | O que tem |
|--------|---------|-----------|
| 1.0 | [`legacy/caixa_basico.c`](legacy/caixa_basico.c) | Uma conta fixa, um saque, contagem de notas com laço e `%` |
| 2.0 | [`src/caixa_eletronico.c`](src/caixa_eletronico.c) | Multi-contas, arquivos, structs, ponteiros, ordenação e recursividade |

### Conceitos aplicados (aulas 10 a 15)

| Aula | Tema | Onde aparece |
|------|------|--------------|
| 10 | Vetores, matrizes e strings | Matriz de cédulas `[2][5]` (valor e estoque), `fgets`, `strcmp`, `strlen` |
| 11 | Arquivos | `contas.txt`, `extrato.txt`, `cedulas.txt` (`fopen`, `fprintf`, `fscanf`, modos `r`, `w`, `a`) |
| 12 | Funções e structs | `struct Conta`, `struct Transacao`, programa dividido em funções |
| 13 | Ponteiros | Passagem por referência (`struct Conta *`), operador `->`, funções de troca |
| 14 | Bubble Sort | Extrato ordenado por valor, com contagem de comparações e trocas |
| 15 | Recursividade e Quick Sort | `distribuir_notas` recursiva e relatório de contas ordenado por saldo |

## Estrutura do repositório

```
.
├── .github/workflows/ci.yml      # pipeline: test -> build -> release
├── src/caixa_eletronico.c        # versão atual (2.0)
├── legacy/caixa_basico.c         # versão original (1.0)
├── tests/run_tests.sh            # testes automáticos
├── Makefile
├── LICENSE
└── README.md
```

## Integração contínua

A cada push na `main` e a cada Pull Request, o GitHub Actions executa:

1. **test**: compila com `-Wall -Wextra -Werror` e roda `tests/run_tests.sh`
2. **build**: gera o executável e o guarda como artefato
3.
