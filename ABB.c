// abb.c
// Compilar: gcc -O2 -std=c11 abb.c -o abb
// Uso: ./abb           -> executa testes de corretude
//      ./abb perf N    -> executa teste de desempenho com N elementos (N inteiro)

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

typedef struct No{
    int chave;
    struct No *esquerda, *direita;
}No;

/* ---------- Criação / utilitários ---------- */

No *novo_no(int chave){
    No *n = (No *)malloc(sizeof(No));
    if (!n){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    n->chave = chave;
    n->esquerda = n->direita = NULL;
    return n;
}

void libera_abb(No *raiz){
    if (!raiz)
        return;
    libera_abb(raiz->esquerda);
    libera_abb(raiz->direita);
    free(raiz);
}


// insere: insere chave se não existir. Retorna a nova raiz (permite chamada recursiva com retorno).
No *insere(No *raiz, int chave){
    if(raiz == NULL){
        return novo_no(chave);
    }
    if(chave < raiz->chave){
        raiz->esquerda = insere(raiz->esquerda, chave);
    }else if(chave > raiz->chave){
        raiz->direita = insere(raiz->direita, chave);
    }
    // se igual, não insere (mantém chaves únicas)
    return raiz;
}

// contem: procura chave, retorna 1 se encontrou, 0 caso contrário.
int contem(No *raiz, int chave){
    No *cur = raiz;
    while(cur){
        if(chave == cur->chave){
            return 1;
        }else if(chave < cur->chave){
            cur = cur->esquerda;
        }else
            cur = cur->direita;
    }
    return 0;
}

// encontra o mínimo (ponteiro) em uma árvore não-vazia
No *encontra_min(No *raiz){
    if(!raiz){
        return NULL;
    }
    No *cur = raiz;
    while(cur->esquerda){
        cur = cur->esquerda;
    }
    return cur;
}

// remove: remove chave se existir. Retorna a nova raiz.
No *remove_no(No *raiz, int chave){
    if(!raiz){
        return NULL;
    }
    if(chave < raiz->chave){
        raiz->esquerda = remove_no(raiz->esquerda, chave);
    }else if(chave > raiz->chave){
        raiz->direita = remove_no(raiz->direita, chave);
    }else{
        if(!raiz->esquerda && !raiz->direita){
            // folha
            free(raiz);
            return NULL;
        }
        else if(!raiz->esquerda){
            No *r = raiz->direita;
            free(raiz);
            return r;
        }else if (!raiz->direita){
            No *l = raiz->esquerda;
            free(raiz);
            return l;
        }else{
            // dois filhos: substitui pela chave mínima da subárvore direita
            No *succ = encontra_min(raiz->direita);
            raiz->chave = succ->chave;
            raiz->direita = remove_no(raiz->direita, succ->chave);
        }
    }
    return raiz;
}

// altura: número de nós no caminho mais longo? Aqui definimos altura como
// número de níveis (raiz sozinha -> altura 1). Alternativamente poderia ser 0-based.
// Vou usar: altura(NULL)=0, altura(leaf)=1.
int altura(No *raiz){
    if (!raiz){
        return 0;
    }
    int hl = altura(raiz->esquerda);
    int hr = altura(raiz->direita);
    return (hl > hr ? hl : hr) + 1;
}

// menor: retorna o menor valor (assume árvore não vazia). Se árvore vazia, imprime erro e aborta.
int menor(No *raiz){
    No *m = encontra_min(raiz);
    if (!m){
        fprintf(stderr, "menor(): arvore vazia\n");
        exit(EXIT_FAILURE);
    }
    return m->chave;
}

// imprime em ordem (in-order)
void imprimeOrdem(No *raiz){
    if (!raiz){
        return;
    }
    imprimeOrdem(raiz->esquerda);
    printf("%d ", raiz->chave);
    imprimeOrdem(raiz->direita);
}

/* ---------- Funções de teste de corretude ---------- */

void teste_corretude(){
    printf("== Teste de corretude ==\n");
    No *r = NULL;
    // inserir e verificar contem
    int arr[] = {50, 30, 70, 20, 40, 60, 80};
    int n = sizeof(arr) / sizeof(arr[0]);
    for(int i = 0; i < n; ++i){
        r = insere(r, arr[i]);
    }

    // contem deve encontrar todos
    for(int i = 0; i < n; ++i){
        assert(contem(r, arr[i]) == 1);
    }

    // contem deve não encontrar um valor ausente
    assert(contem(r, 999) == 0);

    // imprime ordem deve produzir ordenado: 20 30 40 50 60 70 80
    printf("Ordem esperada: 20 30 40 50 60 70 80\nOrdem obtida:   ");
    imprimeOrdem(r);
    printf("\n");

    // menor deve ser 20
    assert(menor(r) == 20);

    // altura: com essa árvore balanceada manualmente, altura = 3
    assert(altura(r) == 3);

    // remoção: remover folha
    r = remove_no(r, 20);
    assert(contem(r, 20) == 0);
    // remover nó com um filho
    r = remove_no(r, 30); // 30 tinha apenas 40 como filho esquerdo/ direito? dependendo da montagem; ainda, árvore permanece válida
    assert(contem(r, 30) == 0);
    // remover nó com dois filhos
    r = remove_no(r, 50);
    assert(contem(r, 50) == 0);

    printf("Remocoes OK. Arvore em ordem agora: ");
    imprimeOrdem(r);
    printf("\n");

    libera_abb(r);
    printf("Teste de corretude concluido com sucesso.\n\n");
}

/* ---------- Teste de desempenho simples ---------- */

void teste_desempenho(int N){
    printf("== Teste de desempenho (N=%d) ==\n", N);
    No *r = NULL;
    int *arr = malloc(sizeof(int) * N);
    if(!arr){
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    srand((unsigned)time(NULL));
    for(int i = 0; i < N; ++i){
        arr[i] = rand();
    }
    clock_t t0, t1;
    double dt_insert, dt_search, dt_remove;

    // medir inserção
    t0 = clock();
    for(int i = 0; i < N; ++i){
        r = insere(r, arr[i]);
    }
    t1 = clock();
    dt_insert = (double)(t1 - t0) / CLOCKS_PER_SEC;

    // medir buscas (procurar todos)
    t0 = clock();
    int found = 0;
    for(int i = 0; i < N; ++i){
        if(contem(r, arr[i])){
            ++found;
        }
    }
    t1 = clock();
    dt_search = (double)(t1 - t0) / CLOCKS_PER_SEC;

    // medir remoções (remover todos)
    t0 = clock();
    for(int i = 0; i < N; ++i)
        r = remove_no(r, arr[i]);
    t1 = clock();
    dt_remove = (double)(t1 - t0) / CLOCKS_PER_SEC;

    printf("Insercao de %d elementos: %.3f s\n", N, dt_insert);
    printf("Buscas de %d elementos (achei %d): %.3f s\n", N, found, dt_search);
    printf("Remocao de %d elementos: %.3f s\n", N, dt_remove);

    free(arr);
    libera_abb(r);
    printf("Teste de desempenho concluido.\n\n");
}

/* ---------- Função main: executa testes ---------- */

int main(int argc, char **argv){
    if(argc >= 2 && strcmp(argv[1], "perf") == 0){
        int N = 200000;
        if(argc >= 3){
            N = atoi(argv[2]);
        }if(N <= 0){
            fprintf(stderr, "N inválido\n");
            return 1;
        }
        teste_desempenho(N);
        return 0;
    }
    teste_corretude();
    printf("Todos os testes foram executados com sucesso.\n");
    return 0;
}
