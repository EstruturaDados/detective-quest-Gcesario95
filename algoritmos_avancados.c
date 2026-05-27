#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 * Detective Quest — Nível Aventureiro
 * ============================================================
 * O detetive explora a mansão (árvore binária) coletando
 * pistas em cada cômodo. As pistas são armazenadas em uma
 * árvore BST e exibidas em ordem alfabética ao final.
 *
 * Estrutura da mansão:
 *
 *              Hall de Entrada
 *             /               \
 *      Sala de Estar        Biblioteca
 *       /       \            /       \
 *   Cozinha   Jardim    Escritório  Porão
 *                            \
 *                         Quarto Secreto
 * ============================================================ */

/* ============================================================
 * ESTRUTURAS DE DADOS
 * ============================================================ */

/*
 * Sala — nó da árvore binária do mapa da mansão.
 * Cada cômodo pode ter uma pista associada (ou string vazia
 * caso não haja indício naquele local).
 */
typedef struct Sala {
    char nome[50];         /* nome do cômodo                     */
    char pista[100];       /* pista encontrada (pode ser vazia)  */
    struct Sala* esquerda; /* cômodo à esquerda                  */
    struct Sala* direita;  /* cômodo à direita                   */
} Sala;

/*
 * PistaNode — nó da árvore BST de pistas coletadas.
 * Organiza as pistas em ordem alfabética pelo conteúdo.
 */
typedef struct PistaNode {
    char conteudo[100];        /* texto da pista              */
    struct PistaNode* esquerda; /* pista alfabeticamente menor */
    struct PistaNode* direita;  /* pista alfabeticamente maior */
} PistaNode;


/* ============================================================
 * FUNÇÕES DA MANSÃO (árvore binária)
 * ============================================================ */

/*
 * criarSala()
 * Aloca dinamicamente um novo cômodo com nome e pista.
 * Se não houver pista no local, passe "" como argumento.
 *
 * Parâmetros:
 *   nome  — string com o nome do cômodo.
 *   pista — string com a pista encontrada (ou "" se não houver).
 * Retorno: ponteiro para a sala recém-criada.
 */
Sala* criarSala(const char* nome, const char* pista) {
    Sala* nova = (Sala*)malloc(sizeof(Sala));
    if (nova == NULL) {
        printf("Erro: falha ao alocar memória para a sala.\n");
        exit(1);
    }
    strcpy(nova->nome,  nome);
    strcpy(nova->pista, pista);
    nova->esquerda = NULL;
    nova->direita  = NULL;
    return nova;
}

/*
 * liberarMapa()
 * Percorre a árvore da mansão em pós-ordem e libera
 * cada nó, evitando vazamento de memória.
 */
void liberarMapa(Sala* sala) {
    if (sala != NULL) {
        liberarMapa(sala->esquerda);
        liberarMapa(sala->direita);
        free(sala);
    }
}


/* ============================================================
 * FUNÇÕES DA BST DE PISTAS
 * ============================================================ */

/*
 * inserirPista()
 * Insere uma nova pista na árvore BST de forma recursiva.
 * A ordenação é feita alfabeticamente com strcmp():
 *   — se a pista nova < nó atual, vai para a esquerda.
 *   — se a pista nova >= nó atual, vai para a direita.
 * Pistas duplicadas são ignoradas (strcmp == 0).
 *
 * Parâmetros:
 *   raiz     — raiz atual da BST (pode ser NULL).
 *   conteudo — texto da pista a inserir.
 * Retorno: nova raiz da BST após a inserção.
 */
PistaNode* inserirPista(PistaNode* raiz, const char* conteudo) {
    /* Posição vazia encontrada: cria o nó aqui */
    if (raiz == NULL) {
        PistaNode* novo = (PistaNode*)malloc(sizeof(PistaNode));
        if (novo == NULL) {
            printf("Erro: falha ao alocar memória para a pista.\n");
            exit(1);
        }
        strcpy(novo->conteudo, conteudo);
        novo->esquerda = NULL;
        novo->direita  = NULL;
        return novo;
    }

    int cmp = strcmp(conteudo, raiz->conteudo);

    if (cmp < 0) {
        /* Pista menor: desce para a subárvore esquerda */
        raiz->esquerda = inserirPista(raiz->esquerda, conteudo);
    } else if (cmp > 0) {
        /* Pista maior: desce para a subárvore direita */
        raiz->direita = inserirPista(raiz->direita, conteudo);
    }
    /* cmp == 0: pista já existe, não insere duplicata */

    return raiz;
}

/*
 * exibirPistas()
 * Percorre a BST de pistas em ordem simétrica (esquerda,
 * raiz, direita), que produz saída em ordem alfabética.
 *
 * Parâmetro: raiz — raiz da BST de pistas.
 */
void exibirPistas(PistaNode* raiz) {
    if (raiz != NULL) {
        exibirPistas(raiz->esquerda);
        printf("  -> %s\n", raiz->conteudo);
        exibirPistas(raiz->direita);
    }
}

/*
 * liberarPistas()
 * Libera toda a memória da BST de pistas em pós-ordem.
 */
void liberarPistas(PistaNode* raiz) {
    if (raiz != NULL) {
        liberarPistas(raiz->esquerda);
        liberarPistas(raiz->direita);
        free(raiz);
    }
}

/*
 * explorarSalasComPistas()
 * Controla a navegação interativa pela mansão.
 * A cada cômodo visitado:
 *   1. Exibe o nome da sala.
 *   2. Verifica e coleta a pista do local (se houver).
 *   3. Insere a pista na BST.
 *   4. Apresenta as saídas disponíveis (e / d / s).
 * A exploração continua até o jogador digitar 's' ou
 * chegar a um nó-folha sem saídas.
 *
 * Parâmetros:
 *   inicio    — ponteiro para a sala inicial (raiz do mapa).
 *   pistasPtr — ponteiro para o ponteiro da raiz da BST,
 *               necessário para atualizar a raiz da BST
 *               quando a primeira pista é inserida.
 */
void explorarSalasComPistas(Sala* inicio, PistaNode** pistasPtr) {
    Sala* atual = inicio;
    char opcao;

    while (atual != NULL) {

        /* --- Exibe o cômodo atual --- */
        printf("\n========================================\n");
        printf("  Voce esta em: %s\n", atual->nome);
        printf("========================================\n");

        /* --- Verifica e coleta a pista do cômodo --- */
        if (strlen(atual->pista) > 0) {
            printf("\n  [PISTA ENCONTRADA!]\n");
            printf("  \"%s\"\n", atual->pista);
            *pistasPtr = inserirPista(*pistasPtr, atual->pista);
        } else {
            printf("  Nenhuma pista neste comodo.\n");
        }

        /* --- Nó-folha: sem saídas disponíveis --- */
        if (atual->esquerda == NULL && atual->direita == NULL) {
            printf("\n  [!] Beco sem saida. Nao ha mais caminhos aqui.\n");
            return;
        }

        /* --- Exibe as saídas disponíveis --- */
        printf("\n  Saidas disponiveis:\n");
        if (atual->esquerda != NULL)
            printf("    [e] Esquerda -> %s\n", atual->esquerda->nome);
        if (atual->direita != NULL)
            printf("    [d] Direita  -> %s\n", atual->direita->nome);
        printf("    [s] Encerrar exploracao\n");
        printf("\n  Sua escolha: ");

        scanf(" %c", &opcao);

        /* --- Processa a escolha do jogador --- */
        if (opcao == 'e' || opcao == 'E') {
            if (atual->esquerda != NULL) {
                atual = atual->esquerda;
            } else {
                printf("  [!] Nao ha caminho a esquerda daqui.\n");
            }
        } else if (opcao == 'd' || opcao == 'D') {
            if (atual->direita != NULL) {
                atual = atual->direita;
            } else {
                printf("  [!] Nao ha caminho a direita daqui.\n");
            }
        } else if (opcao == 's' || opcao == 'S') {
            printf("\n  Voce decidiu encerrar a exploracao.\n");
            return;
        } else {
            printf("  [!] Opcao invalida. Digite e, d ou s.\n");
        }
    }
}

/*
 * main()
 * Ponto de entrada do programa.
 *   1. Monta a árvore binária do mapa com criarSala().
 *   2. Inicializa a BST de pistas como NULL.
 *   3. Inicia a exploração interativa.
 *   4. Exibe as pistas coletadas em ordem alfabética.
 *   5. Libera toda a memória alocada.
 */
int main() {

    /* --------------------------------------------------------
     * Montagem do mapa da mansão (árvore binária fixa)
     * --------------------------------------------------------
     * Nivel 0 — raiz
     * Nivel 1 — Sala de Estar / Biblioteca
     * Nivel 2 — Cozinha, Jardim, Escritório, Porão
     * Nivel 3 — Quarto Secreto
     * -------------------------------------------------------- */

    /* Nível 0 */
    Sala* hall = criarSala("Hall de Entrada",
                           "Tapete deslocado perto da porta principal");

    /* Nível 1 */
    Sala* salaEstar  = criarSala("Sala de Estar",
                                 "Copo com batom vermelho sobre a mesa");
    Sala* biblioteca = criarSala("Biblioteca", "");

    /* Nível 2 */
    Sala* cozinha    = criarSala("Cozinha",
                                 "Faca com cabo de madeira fora do lugar");
    Sala* jardim     = criarSala("Jardim",
                                 "Pegadas na lama proximo ao muro");
    Sala* escritorio = criarSala("Escritorio",
                                 "Gaveta arrombada com documentos espalhados");
    Sala* porao      = criarSala("Porao", "");

    /* Nível 3 */
    Sala* quartoSecreto = criarSala("Quarto Secreto",
                                    "Agenda com pagina rasgada e nome ilegivel");

    /* Ligação dos nós */
    hall->esquerda       = salaEstar;
    hall->direita        = biblioteca;

    salaEstar->esquerda  = cozinha;
    salaEstar->direita   = jardim;

    biblioteca->esquerda = escritorio;
    biblioteca->direita  = porao;

    escritorio->direita  = quartoSecreto;

    /* Inicializa a BST de pistas vazia */
    PistaNode* bstPistas = NULL;

    /* --------------------------------------------------------
     * Apresentação do jogo
     * -------------------------------------------------------- */
    printf("============================================\n");
    printf("      DETECTIVE QUEST — Nivel Aventureiro  \n");
    printf("============================================\n");
    printf("\nBem-vindo, Detetive!\n");
    printf("Explore a mansao, colete pistas e\n");
    printf("descubra o culpado.\n");
    printf("Use [e] esquerda, [d] direita, [s] sair.\n");

    /* --------------------------------------------------------
     * Exploração da mansão
     * -------------------------------------------------------- */
    explorarSalasComPistas(hall, &bstPistas);

    /* --------------------------------------------------------
     * Relatório de pistas coletadas (BST em ordem alfabética)
     * -------------------------------------------------------- */
    printf("\n============================================\n");
    printf("  RELATORIO DE PISTAS COLETADAS            \n");
    printf("============================================\n");

    if (bstPistas == NULL) {
        printf("  Nenhuma pista foi coletada.\n");
    } else {
        printf("  Pistas em ordem alfabetica:\n\n");
        exibirPistas(bstPistas);
    }

    printf("\n============================================\n");
    printf("  Fim da exploracao. Boa sorte, detetive!  \n");
    printf("============================================\n");

    /* --------------------------------------------------------
     * Liberação de memória
     * -------------------------------------------------------- */
    liberarMapa(hall);
    liberarPistas(bstPistas);

    return 0;
}