#!/usr/bin/env bash
# Testes automaticos do Caixa Eletronico.
# Cada teste roda o programa numa pasta temporaria (para nao sujar o repositorio),
# alimenta a entrada padrao e confere a saida e os arquivos gerados.
# Uso: bash tests/run_tests.sh

set -u
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$(mktemp -d)/caixa"
gcc -Wall -Wextra -o "$BIN" "$RAIZ/src/caixa_eletronico.c" || { echo "FALHA: erro de compilacao"; exit 1; }

PASSOU=0
FALHOU=0

# rodar "<entrada>"  -> executa em pasta limpa e deixa a saida em $SAIDA e a pasta em $PASTA
rodar() {
    PASTA="$(mktemp -d)"
    SAIDA="$(cd "$PASTA" && printf "$1" | timeout 5 "$BIN" 2>&1)"
}

# conferir "<nome>" "<texto esperado na saida>"
conferir() {
    if grep -qF -- "$2" <<< "$SAIDA"; then
        echo "  [OK]    $1"; PASSOU=$((PASSOU + 1))
    else
        echo "  [FALHA] $1  (esperado: '$2')"; FALHOU=$((FALHOU + 1))
    fi
}

# conferir_arquivo "<nome>" "<arquivo>" "<texto esperado>"
conferir_arquivo() {
    if grep -qF -- "$3" "$PASTA/$2" 2>/dev/null; then
        echo "  [OK]    $1"; PASSOU=$((PASSOU + 1))
    else
        echo "  [FALHA] $1  (esperado '$3' em $2)"; FALHOU=$((FALHOU + 1))
    fi
}

echo "== Login =="
rodar "1\n1001\n1234\n0\n0\n"
conferir "login valido mostra o titular" "Bem-vindo(a), Ana Paula Souza"
rodar "1\n9999\n0\n"
conferir "conta inexistente" "Conta nao encontrada"
rodar "1\n1001\n1\n2\n3\n0\n"
conferir "3 senhas erradas bloqueiam" "Numero maximo de tentativas excedido"

echo "== Saque =="
rodar "1\n1001\n1234\n2\n285\n0\n0\n"
conferir "285 = 2x100" "2 nota(s) de R\$ 100"
conferir "285 = 1x50"  "1 nota(s) de R\$ 50"
conferir "285 = 1x20"  "1 nota(s) de R\$ 20"
conferir "285 = 1x10"  "1 nota(s) de R\$ 10"
conferir "285 = 1x5"   "1 nota(s) de R\$ 5"
conferir "novo saldo" "Novo saldo: R\$ 715.00"
conferir_arquivo "saldo persistido em contas.txt" contas.txt "1001 1234 715.00"
conferir_arquivo "saque registrado no extrato" extrato.txt "1001 SAQUE 285.00"
conferir_arquivo "estoque de cedulas atualizado" cedulas.txt "100 8"

rodar "1\n1001\n1234\n2\n17\n0\n0\n"
conferir "valor nao multiplo de 5" "multiplo de 5"
rodar "1\n1003\n1111\n2\n500\n0\n0\n"
conferir "saldo insuficiente" "Saldo insuficiente"
rodar "1\n1001\n1234\n2\n-50\n0\n0\n"
conferir "valor negativo" "Valor invalido"
rodar "1\n1001\n1234\n2\nabc\n0\n0\n"
conferir "entrada nao numerica" "Entrada invalida"

echo "== Deposito e transferencia =="
rodar "1\n1001\n1234\n3\n250.50\n0\n0\n"
conferir "deposito atualiza saldo" "Novo saldo: R\$ 1250.50"
rodar "1\n1001\n1234\n4\n1002\n200\n0\n0\n"
conferir "transferencia concluida" "Transferencia de R\$ 200.00 para Bruno Lima concluida"
conferir_arquivo "origem debitada" contas.txt "1001 1234 800.00"
conferir_arquivo "destino creditado" contas.txt "1002 4321 2700.50"
rodar "1\n1001\n1234\n4\n1001\n0\n0\n"
conferir "nao transfere para si mesmo" "propria conta"

echo "== Extrato (Bubble Sort) =="
rodar "1\n1001\n1234\n3\n10\n3\n500\n3\n100\n5\n2\n0\n0\n"
conferir "bubble sort informa estatisticas" "Bubble Sort"
ORDEM="$(grep -E '^[0-9]+ +DEPOSITO' <<< "$SAIDA" | awk '{print $3}' | tr '\n' ' ')"
if [ "$ORDEM" = "500.00 100.00 10.00 " ]; then
    echo "  [OK]    extrato ordenado do maior para o menor"; PASSOU=$((PASSOU + 1))
else
    echo "  [FALHA] extrato fora de ordem: '$ORDEM'"; FALHOU=$((FALHOU + 1))
fi

echo "== Area administrativa =="
rodar "2\n1234\n0\n"
conferir "senha admin errada" "Acesso negado"
rodar "2\n9999\n1\n0\n0\n"
conferir "relatorio de contas (Quick Sort)" "Total em depositos: R\$ 3800.50"
rodar "2\n9999\n3\n2000\n5555\nMaria Teste\n1\n0\n0\n"
conferir "cadastro de conta" "Conta 2000 criada para Maria Teste"
rodar "2\n9999\n2\n0\n0\n"
conferir "estoque inicial de cedulas" "Total no caixa: R\$ 1850"

echo
echo "Resultado: $PASSOU passaram, $FALHOU falharam."
[ "$FALHOU" -eq 0 ]
