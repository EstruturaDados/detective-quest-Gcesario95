/* ============================================================
 *           Detective Quest — Nivel Mestre
 * =============================================================
 * 
 * Sistema completo de exploracao, coleta de pistas, tabela
 * hash pista->suspeito e julgamento final.
 *
 * Estrutura da mansao:
 *
 *              Hall de Entrada
 *             /               \
 *      Sala de Estar        Biblioteca
 *       /       \            /       \
 *   Cozinha   Jardim    Escritorio  Porao
 *                /           \
 *          Garagem        Quarto Secreto
 * ============================================================ */


/* ============================================================
 * CONSTANTES
 * ============================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_TAM     13   /* tamanho da tabela hash (numero primo reduz colisoes) */
#define MAX_PISTA   100   /* tamanho maximo de uma pista                          */
#define MAX_NOME     50   /* tamanho maximo de um nome de sala ou suspeito        */
#define MIN_PISTAS    2   /* minimo de pistas necessarias para uma acusacao       */


/* ============================================================
 * ESTRUTURAS DE DADOS
 * ============================================================ */

/*
 * Sala — no da arvore binaria que representa o mapa da mansao.
 * Cada comodo possui um nome e pode conter uma pista.
 */
typedef struct Sala {
    char nome[MAX_NOME];   /* nome identificador do comodo          */
    char pista[MAX_PISTA]; /* pista encontrada (vazia se nao houver) */
    struct Sala* esquerda; /* caminho a esquerda                    */
    struct Sala* direita;  /* caminho a direita                     */
} Sala;

/*
 * PistaNode — no da arvore BST de pistas coletadas.
 * Armazena o texto da pista e registra quantas vezes foi coletada.
 */
typedef struct PistaNode {
    char conteudo[MAX_PISTA];   /* texto da pista                  */
    struct PistaNode* esquerda; /* pista alfabeticamente menor     */
    struct PistaNode* direita;  /* pista alfabeticamente maior     */
} PistaNode;

/*
 * EntradaHash — no de lista encadeada dentro de cada bucket
 * da tabela hash. Associa uma pista a um suspeito.
 * O encadeamento (chaining) resolve colisoes de hash.
 */
typedef struct EntradaHash {
    char pista[MAX_PISTA];       /* chave: texto da pista        */
    char suspeito[MAX_NOME];     /* valor: nome do suspeito      */
    struct EntradaHash* proximo; /* proximo no da lista (colisao) */
} EntradaHash;

/*
 * TabelaHash — array de ponteiros para listas de EntradaHash.
 * Cada posicao (bucket) pode conter zero ou mais entradas.
 */
typedef struct {
    EntradaHash* buckets[HASH_TAM];
} TabelaHash;


/* ============================================================
 * FUNCOES DA TABELA HASH
 * ============================================================ */

/*
 * funcaoHash()
 * Calcula o indice do bucket para uma chave (pista) usando
 * o metodo polinomial (soma de char * fator).
 * Retorna um valor entre 0 e HASH_TAM-1.
 */
int funcaoHash(const char* chave) {
    unsigned int hash = 0;
    while (*chave) {
        hash = (hash * 31 + (unsigned char)*chave) % HASH_TAM;
        chave++;
    }
    return (int)hash;
}

/*
 * inicializarHash()
 * Prepara a tabela hash zerand todos os ponteiros de bucket.
 * Deve ser chamada antes de qualquer insercao.
 */
void inicializarHash(TabelaHash* tabela) {
    for (int i = 0; i < HASH_TAM; i++) {
        tabela->buckets[i] = NULL;
    }
}

/*
 * inserirNaHash()
 * Insere a associacao pista->suspeito na tabela hash.
 * Caso a pista ja exista na tabela, a insercao e ignorada
 * para evitar duplicatas. Colisoes sao tratadas por chaining
 * (novo no e inserido no inicio da lista do bucket).
 *
 * Parametros:
 *   tabela   — ponteiro para a tabela hash.
 *   pista    — string usada como chave de busca.
 *   suspeito — nome do suspeito associado a pista.
 */
void inserirNaHash(TabelaHash* tabela, const char* pista,
                   const char* suspeito) {
    int idx = funcaoHash(pista);

    /* Verifica se a pista ja esta no bucket (evita duplicata) */
    EntradaHash* atual = tabela->buckets[idx];
    while (atual != NULL) {
        if (strcmp(atual->pista, pista) == 0) return; /* ja existe */
        atual = atual->proximo;
    }

    /* Cria novo no e insere no inicio da lista do bucket */
    EntradaHash* novo = (EntradaHash*)malloc(sizeof(EntradaHash));
    if (novo == NULL) {
        printf("Erro: falha ao alocar entrada na hash.\n");
        exit(1);
    }
    strcpy(novo->pista,    pista);
    strcpy(novo->suspeito, suspeito);
    novo->proximo          = tabela->buckets[idx]; /* encadeia */
    tabela->buckets[idx]   = novo;
}

/*
 * encontrarSuspeito()
 * Consulta a tabela hash e retorna o nome do suspeito
 * associado a uma pista. Retorna NULL se nao encontrar.
 *
 * Parametros:
 *   tabela — ponteiro para a tabela hash.
 *   pista  — chave de busca.
 * Retorno: string com o nome do suspeito, ou NULL.
 */
const char* encontrarSuspeito(TabelaHash* tabela, const char* pista) {
    int idx = funcaoHash(pista);
    EntradaHash* atual = tabela->buckets[idx];
    while (atual != NULL) {
        if (strcmp(atual->pista, pista) == 0) {
            return atual->suspeito;
        }
        atual = atual->proximo;
    }
    return NULL; /* pista nao encontrada */
}

/*
 * liberarHash()
 * Percorre todos os buckets e libera cada no da lista,
 * evitando vazamento de memoria.
 */
void liberarHash(TabelaHash* tabela) {
    for (int i = 0; i < HASH_TAM; i++) {
        EntradaHash* atual = tabela->buckets[i];
        while (atual != NULL) {
            EntradaHash* prox = atual->proximo;
            free(atual);
            atual = prox;
        }
        tabela->buckets[i] = NULL;
    }
}


/* ============================================================
 * FUNCOES DA MANSAO (arvore binaria)
 * ============================================================ */

/*
 * criarSala()
 * Aloca dinamicamente um comodo com nome e pista.
 * Passe "" como pista para cômodos sem indicio.
 *
 * Parametros:
 *   nome  — string com o nome do comodo.
 *   pista — string com a pista (ou "" se nao houver).
 * Retorno: ponteiro para a sala criada.
 */
Sala* criarSala(const char* nome, const char* pista) {
    Sala* nova = (Sala*)malloc(sizeof(Sala));
    if (nova == NULL) {
        printf("Erro: falha ao alocar sala.\n");
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
 * Libera todos os nos da arvore da mansao em pos-ordem.
 */
void liberarMapa(Sala* sala) {
    if (sala != NULL) {
        liberarMapa(sala->esquerda);
        liberarMapa(sala->direita);
        free(sala);
    }
}


/* ============================================================
 * FUNCOES DA BST DE PISTAS
 * ============================================================ */

/*
 * inserirPista() / adicionarPista()
 * Insere uma pista na BST em ordem alfabetica (strcmp).
 * Pistas identicas sao ignoradas (sem duplicatas na BST).
 *
 * Parametros:
 *   raiz     — raiz atual da BST.
 *   conteudo — texto da pista a inserir.
 * Retorno: nova raiz da BST apos a insercao.
 */
PistaNode* inserirPista(PistaNode* raiz, const char* conteudo) {
    if (raiz == NULL) {
        PistaNode* novo = (PistaNode*)malloc(sizeof(PistaNode));
        if (novo == NULL) {
            printf("Erro: falha ao alocar pista.\n");
            exit(1);
        }
        strcpy(novo->conteudo, conteudo);
        novo->esquerda = NULL;
        novo->direita  = NULL;
        return novo;
    }
    int cmp = strcmp(conteudo, raiz->conteudo);
    if      (cmp < 0) raiz->esquerda = inserirPista(raiz->esquerda, conteudo);
    else if (cmp > 0) raiz->direita  = inserirPista(raiz->direita,  conteudo);
    /* cmp == 0: duplicata, ignora */
    return raiz;
}

/* Alias descritivo exigido pelo enunciado */
PistaNode* adicionarPista(PistaNode* raiz, const char* conteudo) {
    return inserirPista(raiz, conteudo);
}

/*
 * contarPistasPorSuspeito()
 * Percorre a BST em ordem e conta quantas pistas apontam
 * para o suspeito informado, consultando a tabela hash.
 *
 * Parametros:
 *   raiz     — raiz da BST de pistas.
 *   suspeito — nome do suspeito a verificar.
 *   tabela   — tabela hash para consultar a associacao.
 * Retorno: numero de pistas que apontam para o suspeito.
 */
int contarPistasPorSuspeito(PistaNode* raiz, const char* suspeito,
                             TabelaHash* tabela) {
    if (raiz == NULL) return 0;

    int contagem = 0;
    const char* s = encontrarSuspeito(tabela, raiz->conteudo);
    if (s != NULL && strcmp(s, suspeito) == 0) {
        contagem = 1;
    }
    return contagem
           + contarPistasPorSuspeito(raiz->esquerda, suspeito, tabela)
           + contarPistasPorSuspeito(raiz->direita,  suspeito, tabela);
}

/*
 * exibirPistasComSuspeito()
 * Percorre a BST em ordem e exibe cada pista junto ao
 * suspeito associado (consultado na tabela hash).
 */
void exibirPistasComSuspeito(PistaNode* raiz, TabelaHash* tabela) {
    if (raiz != NULL) {
        exibirPistasComSuspeito(raiz->esquerda, tabela);

        const char* s = encontrarSuspeito(tabela, raiz->conteudo);
        if (s != NULL)
            printf("  [+] \"%s\"\n      -> Suspeito: %s\n",
                   raiz->conteudo, s);
        else
            printf("  [+] \"%s\"\n      -> Suspeito: desconhecido\n",
                   raiz->conteudo);

        exibirPistasComSuspeito(raiz->direita, tabela);
    }
}

/*
 * liberarPistas()
 * Libera todos os nos da BST de pistas em pos-ordem.
 */
void liberarPistas(PistaNode* raiz) {
    if (raiz != NULL) {
        liberarPistas(raiz->esquerda);
        liberarPistas(raiz->direita);
        free(raiz);
    }
}


/* ============================================================
 * EXPLORACAO
 * ============================================================ */

/*
 * explorarSalas()
 * Navega pela arvore da mansao de modo interativo.
 * A cada visita:
 *   1. Exibe o nome do comodo.
 *   2. Detecta e exibe a pista do local (se houver).
 *   3. Insere a pista na BST via adicionarPista().
 *   4. Exibe as saidas disponiveis e aguarda a escolha.
 * A exploracao termina ao digitar 's' ou chegar a no-folha.
 *
 * Parametros:
 *   inicio    — sala inicial (raiz do mapa).
 *   pistasPtr — ponteiro para o ponteiro da raiz da BST.
 */
void explorarSalas(Sala* inicio, PistaNode** pistasPtr) {
    Sala* atual = inicio;
    char opcao;

    while (atual != NULL) {

        /* Exibe o comodo atual */
        printf("\n+---------------------------------------+\n");
        printf("  Voce esta em: %s\n", atual->nome);
        printf("+---------------------------------------+\n");

        /* Coleta a pista do comodo (se existir) */
        if (strlen(atual->pista) > 0) {
            printf("\n  [!] PISTA ENCONTRADA:\n");
            printf("      \"%s\"\n", atual->pista);
            *pistasPtr = adicionarPista(*pistasPtr, atual->pista);
        } else {
            printf("  Nenhuma pista neste comodo.\n");
        }

        /* No-folha: sem saidas */
        if (atual->esquerda == NULL && atual->direita == NULL) {
            printf("\n  [X] Beco sem saida. Retorne ou encerre.\n");
            return;
        }

        /* Exibe as saidas disponiveis */
        printf("\n  Saidas:\n");
        if (atual->esquerda != NULL)
            printf("    [e] Esquerda -> %s\n", atual->esquerda->nome);
        if (atual->direita != NULL)
            printf("    [d] Direita  -> %s\n", atual->direita->nome);
        printf("    [s] Encerrar exploracao\n");
        printf("\n  Escolha: ");
        scanf(" %c", &opcao);

        /* Processa a escolha */
        if (opcao == 'e' || opcao == 'E') {
            if (atual->esquerda != NULL) atual = atual->esquerda;
            else printf("  [!] Sem caminho a esquerda.\n");
        } else if (opcao == 'd' || opcao == 'D') {
            if (atual->direita != NULL) atual = atual->direita;
            else printf("  [!] Sem caminho a direita.\n");
        } else if (opcao == 's' || opcao == 'S') {
            printf("\n  Voce encerrou a exploracao.\n");
            return;
        } else {
            printf("  [!] Opcao invalida. Use e, d ou s.\n");
        }
    }
}


/* ============================================================
 * JULGAMENTO FINAL
 * ============================================================ */

/*
 * verificarSuspeitoFinal()
 * Conduz a fase de julgamento:
 *   1. Exibe todas as pistas coletadas com seus suspeitos.
 *   2. Solicita ao jogador que acuse um suspeito.
 *   3. Conta quantas pistas apontam para o acusado.
 *   4. Exibe o veredicto: condenado (>= MIN_PISTAS) ou
 *      inocentado (< MIN_PISTAS).
 *
 * Parametros:
 *   bst    — raiz da BST de pistas coletadas.
 *   tabela — tabela hash pista->suspeito.
 */
void verificarSuspeitoFinal(PistaNode* bst, TabelaHash* tabela) {
    char acusado[MAX_NOME];

    printf("\n================================================\n");
    printf("           RELATORIO DE INVESTIGACAO           \n");
    printf("================================================\n");

    if (bst == NULL) {
        printf("  Nenhuma pista coletada. Sem evidencias.\n");
        printf("  Impossivel realizar uma acusacao.\n");
        return;
    }

    /* Exibe pistas coletadas com seus suspeitos */
    printf("  Pistas coletadas (ordem alfabetica):\n\n");
    exibirPistasComSuspeito(bst, tabela);

    /* Solicita a acusacao */
    printf("\n------------------------------------------------\n");
    printf("  Suspeitos conhecidos:\n");
    printf("    - Carlos\n");
    printf("    - Ana\n");
    printf("    - Joao\n");
    printf("------------------------------------------------\n");
    /* Limpa todo o buffer de entrada antes de ler o nome */
    {
        int ch;
        do { ch = getchar(); } while (ch != '\n' && ch != EOF);
    }
    printf("  Quem voce acusa? ");
    if (fgets(acusado, MAX_NOME, stdin) == NULL) acusado[0] = '\0';
    acusado[strcspn(acusado, "\n")] = '\0'; /* remove \n do fgets */

    /* Conta pistas que apontam para o acusado */
    int total = contarPistasPorSuspeito(bst, acusado, tabela);

    printf("\n================================================\n");
    printf("               VEREDICTO FINAL                 \n");
    printf("================================================\n");
    printf("  Acusado : %s\n", acusado);
    printf("  Pistas  : %d encontrada(s) contra ele/ela\n", total);
    printf("------------------------------------------------\n");

    if (total >= MIN_PISTAS) {
        printf("  CULPADO! Evidencias suficientes.\n");
        printf("  %s sera levado(a) a julgamento!\n", acusado);
    } else {
        printf("  INOCENTADO. Evidencias insuficientes.\n");
        printf("  %s nao pode ser condenado(a) com apenas\n", acusado);
        printf("  %d pista(s). O caso continua em aberto.\n", total);
    }
    printf("================================================\n");
}


/* ============================================================
 * MAIN
 * ============================================================ */

/*
 * main()
 * Ponto de entrada:
 *   1. Monta o mapa da mansao (arvore binaria).
 *   2. Inicializa a tabela hash e insere as associacoes
 *      pista->suspeito.
 *   3. Inicia a exploracao interativa.
 *   4. Conduz o julgamento final.
 *   5. Libera toda a memoria alocada.
 */
int main() {

    /* --------------------------------------------------------
     * 1. Mapa da mansao
     * --------------------------------------------------------
     *              Hall de Entrada
     *             /               \
     *      Sala de Estar        Biblioteca
     *       /       \            /       \
     *   Cozinha   Jardim    Escritorio  Porao
     *                /           \
     *          Garagem        Quarto Secreto
     * -------------------------------------------------------- */

    Sala* hall     = criarSala("Hall de Entrada",
                               "Tapete deslocado perto da porta principal");
    Sala* salaEstar  = criarSala("Sala de Estar",
                                 "Copo com batom vermelho sobre a mesa");
    Sala* biblioteca = criarSala("Biblioteca", "");
    Sala* cozinha    = criarSala("Cozinha",
                                 "Faca com cabo de madeira fora do lugar");
    Sala* jardim     = criarSala("Jardim",
                                 "Pegadas na lama proximo ao muro");
    Sala* escritorio = criarSala("Escritorio",
                                 "Gaveta arrombada com documentos espalhados");
    Sala* porao      = criarSala("Porao",
                                 "Corrente enferrujada jogada no chao");
    Sala* garagem    = criarSala("Garagem",
                                 "Luva de couro escondida atras de uma caixa");
    Sala* quartoSec  = criarSala("Quarto Secreto",
                                 "Agenda com pagina rasgada e nome ilegivel");

    /* Ligacao dos nos */
    hall->esquerda       = salaEstar;
    hall->direita        = biblioteca;
    salaEstar->esquerda  = cozinha;
    salaEstar->direita   = jardim;
    biblioteca->esquerda = escritorio;
    biblioteca->direita  = porao;
    jardim->esquerda     = garagem;
    escritorio->direita  = quartoSec;

    /* --------------------------------------------------------
     * 2. Tabela hash: pista -> suspeito
     * --------------------------------------------------------
     * Cada pista foi plantada por um suspeito especifico.
     * O detetive so podera condenar com >= 2 pistas.
     * -------------------------------------------------------- */
    TabelaHash tabela;
    inicializarHash(&tabela);

    inserirNaHash(&tabela,
        "Tapete deslocado perto da porta principal", "Carlos");
    inserirNaHash(&tabela,
        "Copo com batom vermelho sobre a mesa",       "Ana");
    inserirNaHash(&tabela,
        "Faca com cabo de madeira fora do lugar",     "Joao");
    inserirNaHash(&tabela,
        "Pegadas na lama proximo ao muro",            "Carlos");
    inserirNaHash(&tabela,
        "Gaveta arrombada com documentos espalhados", "Joao");
    inserirNaHash(&tabela,
        "Corrente enferrujada jogada no chao",        "Joao");
    inserirNaHash(&tabela,
        "Luva de couro escondida atras de uma caixa", "Carlos");
    inserirNaHash(&tabela,
        "Agenda com pagina rasgada e nome ilegivel",  "Ana");

    /* --------------------------------------------------------
     * 3. Inicializa BST de pistas e inicia o jogo
     * -------------------------------------------------------- */
    PistaNode* bstPistas = NULL;

    printf("================================================\n");
    printf("     DETECTIVE QUEST — Nivel Mestre            \n");
    printf("     Enigma Studios                            \n");
    printf("================================================\n");
    printf(" Explore a mansao, colete pistas e acuse\n");
    printf(" o verdadeiro culpado.\n");
    printf(" Comandos: [e] esquerda  [d] direita  [s] sair\n");
    printf("================================================\n");

    explorarSalas(hall, &bstPistas);

    /* --------------------------------------------------------
     * 4. Julgamento final
     * -------------------------------------------------------- */
    verificarSuspeitoFinal(bstPistas, &tabela);

    /* --------------------------------------------------------
     * 5. Liberacao de memoria
     * -------------------------------------------------------- */
    liberarMapa(hall);
    liberarPistas(bstPistas);
    liberarHash(&tabela);

    return 0;
}