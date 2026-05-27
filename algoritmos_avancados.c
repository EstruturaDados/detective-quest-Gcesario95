#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 *          Detective Quest - Nivel Novato
 *
 * Representa o mapa da mansão como uma árvore binária.
 * O jogador explora os cômodos escolhendo ir à esquerda (e)
 * ou à direita (d) a partir do Hall de Entrada.
 * ============================================================ */

/* Estrutura que representa um cômodo da mansão */
typedef struct Sala {
    char nome[50];         /* nome identificador do cômodo */
    struct Sala* esquerda; /* ponteiro para o cômodo à esquerda */
    struct Sala* direita;  /* ponteiro para o cômodo à direita */
} Sala;

/* ------------------------------------------------------------
 * criarSala()
 * Aloca dinamicamente uma nova sala com o nome fornecido e
 * inicializa os ponteiros filhos como NULL.
 * Parâmetro: nome - string com o nome do cômodo.
 * Retorno:   ponteiro para a sala recém-criada.
 * ------------------------------------------------------------ */
Sala* criarSala(const char* nome) {
    Sala* nova = (Sala*)malloc(sizeof(Sala));
    if (nova == NULL) {
        printf("Erro: falha ao alocar memória para a sala.\n");
        exit(1);
    }
    strcpy(nova->nome, nome);
    nova->esquerda = NULL;
    nova->direita  = NULL;
    return nova;
}

/* ------------------------------------------------------------
 * liberarMapa()
 * Percorre a árvore em pós-ordem e libera cada nó alocado,
 * evitando vazamento de memória ao encerrar o programa.
 * ------------------------------------------------------------ */
void liberarMapa(Sala* sala) {
    if (sala != NULL) {
        liberarMapa(sala->esquerda);
        liberarMapa(sala->direita);
        free(sala);
    }
}

/* ------------------------------------------------------------
 * explorarSalas()
 * Permite a navegação interativa do jogador pela árvore.
 * A cada sala visitada, exibe o nome do cômodo e oferece
 * as opções disponíveis (e, d, s). A exploração continua
 * até o jogador atingir um nó-folha (sem filhos) ou digitar s.
 * ------------------------------------------------------------ */
void explorarSalas(Sala* atual) {
    char opcao;

    while (atual != NULL) {
        /* Exibe o cômodo atual */
        printf("\n========================================\n");
        printf("  Você está em: %s\n", atual->nome);
        printf("========================================\n");

        /* Verifica se é um nó-folha (sem saídas) */
        if (atual->esquerda == NULL && atual->direita == NULL) {
            printf("  [!] Este cômodo não tem mais saídas.\n");
            printf("      Exploração encerrada aqui.\n");
            return;
        }

        /* Exibe as saídas disponíveis */
        printf("  Saídas disponíveis:\n");
        if (atual->esquerda != NULL)
            printf("    [e] Esquerda -> %s\n", atual->esquerda->nome);
        if (atual->direita != NULL)
            printf("    [d] Direita  -> %s\n", atual->direita->nome);
        printf("    [s] Sair da exploração\n");
        printf("\n  Sua escolha: ");

        scanf(" %c", &opcao);

        /* Processa a escolha do jogador */
        if (opcao == 'e' || opcao == 'E') {
            if (atual->esquerda != NULL) {
                atual = atual->esquerda;
            } else {
                printf("  [!] Não há caminho à esquerda daqui.\n");
            }
        } else if (opcao == 'd' || opcao == 'D') {
            if (atual->direita != NULL) {
                atual = atual->direita;
            } else {
                printf("  [!] Não há caminho à direita daqui.\n");
            }
        } else if (opcao == 's' || opcao == 'S') {
            printf("\n  Você decidiu parar a exploração.\n");
            return;
        } else {
            printf("  [!] Opção inválida. Digite e, d ou s.\n");
        }
    }
}

/* ------------------------------------------------------------
 * main()
 * Ponto de entrada do programa. Monta o mapa da mansão
 * de forma manual, criando cada sala e ligando os ponteiros
 * para formar a árvore binária. Em seguida, inicia a
 * exploração interativa a partir do Hall de Entrada.
 *
 * Estrutura da mansão:
 *
 *               Hall de Entrada
 *              /               \
 *       Sala de Estar        Biblioteca
 *        /       \            /       \
 *    Cozinha   Jardim    Escritório  Porão
 *                            \
 *                          Quarto Secreto
 * ------------------------------------------------------------ */
int main() {
    /* --- Montagem do mapa (árvore) --- */

    /* Nível 0 - raiz */
    Sala* hall = criarSala("Hall de Entrada");

    /* Nível 1 */
    Sala* salaEstar   = criarSala("Sala de Estar");
    Sala* biblioteca  = criarSala("Biblioteca");

    /* Nível 2 */
    Sala* cozinha     = criarSala("Cozinha");
    Sala* jardim      = criarSala("Jardim");
    Sala* escritorio  = criarSala("Escritório");
    Sala* porao       = criarSala("Porão");

    /* Nível 3 */
    Sala* quartoSecreto = criarSala("Quarto Secreto");

    /* Ligação dos nós */
    hall->esquerda        = salaEstar;
    hall->direita         = biblioteca;

    salaEstar->esquerda   = cozinha;
    salaEstar->direita    = jardim;

    biblioteca->esquerda  = escritorio;
    biblioteca->direita   = porao;

    escritorio->direita   = quartoSecreto;

    /* --- Apresentação do jogo --- */
    printf("============================================\n");
    printf("          DETECTIVE QUEST                  \n");
    printf("       Mapa da Mansão - Enigma Studios     \n");
    printf("============================================\n");
    printf("\nBem-vindo, Detetive!\n");
    printf("Explore a mansão em busca de pistas.\n");
    printf("Use [e] para ir à esquerda, [d] para ir\n");
    printf("à direita e [s] para encerrar.\n");

    /* --- Início da exploração --- */
    explorarSalas(hall);

    printf("\n============================================\n");
    printf("  Fim da exploração. Até a próxima!\n");
    printf("============================================\n");

    /* --- Libera a memória alocada --- */
    liberarMapa(hall);

    return 0;
}