

/**
 * @authors Amanda Gomes e Ana Clara Morais
 */

#include <stdio.h>
#include <stdlib.h> 
#include <stdbool.h> // Implementa valores booleanos
#include <time.h>    
#include <assert.h> // Para testes aleatorios 


#define INTERVALO_TESTE 100 // Testar com numeros de 0 a 99( pode ser modificado para efetuar maiores testes)
#define NUM_OPERACOES 5000  // Fazer 5 mil operações aleatorias

typedef struct {
    int min;
    int max;
} Intervalo;

Intervalo cria_intervalo(int min, int max) {
    if (min > max) {
        int temp = min;
        min = max;
        max = temp;
    }
    Intervalo inv = {min, max};
    return inv;
}

// Verifica se um valor esta dentro do intervalo
bool intervalo_contem(Intervalo* inv, int valor) {
    return (valor >= inv->min && valor <= inv->max);
}
// Ajusta os limites do intervalo
void intervalo_set(Intervalo* inv, int min, int max) {
    if (min > max) {
        int temp = min;
        min = max;
        max = temp;
    }
    inv->min = min;
    inv->max = max;
}

typedef struct No {
    Intervalo dados;
    struct No* prox;
} No;

typedef struct {
    No* cabeca;
} ListaIntervalar;


// Cria lista com no cabeça (sentinela)
ListaIntervalar* cria_lista() {
    ListaIntervalar* lista = (ListaIntervalar*)malloc(sizeof(ListaIntervalar));
    if (lista == NULL) {
        perror("Falha ao alocar lista");
        exit(1);
    }
    
    lista->cabeca = (No*)malloc(sizeof(No));
    if (lista->cabeca == NULL) {
        perror("Falha ao alocar nó cabeça");
        free(lista);
        exit(1);
    }
    
    lista->cabeca->prox = NULL;

    return lista;
}

// Libera toda a lista
void libera_lista(ListaIntervalar* lista) {
    if (lista == NULL) return;
    
    No* atual = lista->cabeca;
    while (atual != NULL) {
        No* temp = atual;
        atual = atual->prox;
        free(temp);
    }
    free(lista);
}

// Imprime todos os intervalos da lista
void imprime_lista(ListaIntervalar* lista) {
    No* atual = lista->cabeca->prox; 
    while (atual != NULL) {
        printf("[%d, %d] -> ", atual->dados.min, atual->dados.max);
        atual = atual->prox;
    }
    printf("NULL\n");
}

// Consulta se um inteiro pertence a algum intervalo
bool lista_contem_inteiro(ListaIntervalar* lista, int valor) {
    No* atual = lista->cabeca->prox; 
    
    while (atual != NULL) {
        if (valor < atual->dados.min) {
            return false;
        }
        if (intervalo_contem(&atual->dados, valor)) {
            return true;
        }
        atual = atual->prox;
    }
    return false;
}

// Mantem intervalos ordenados e sem sobreposicao
void lista_insere_inteiro(ListaIntervalar* lista, int valor) {
    No* ant = lista->cabeca;
    No* atual = lista->cabeca->prox;

    // Encontra posicao correta para inserir
    while (atual != NULL && valor > atual->dados.max) { 
        ant = atual;
        atual = atual->prox;
    }

    // Caso já esteja dentro de um intervalo existente
    if (atual != NULL && valor >= atual->dados.min) {
        if (valor <= atual->dados.max) {
            return; 
        }
    }

    bool funde_com_ant = (ant != lista->cabeca) && (valor == ant->dados.max + 1);
    bool funde_com_atual = (atual != NULL) && (valor == atual->dados.min - 1);

    if (funde_com_ant && funde_com_atual) {
        // Junta tres partes: [ant] valor [atual]
        ant->dados.max = atual->dados.max; 
        ant->prox = atual->prox;       
        free(atual);                
        
    } else if (funde_com_ant) {
        ant->dados.max = valor;
        
    } else if (funde_com_atual) {
        atual->dados.min = valor;
        
    } else {
        // Cria novo intervalo isolado
        No* novo = (No*)malloc(sizeof(No));
        if (novo == NULL) {
            perror("Falha ao alocar novo nó");
            return;
        }
        novo->dados = cria_intervalo(valor, valor);

        novo->prox = atual; 
        ant->prox = novo;
    }
}

// Pode dividir um intervalo em dois
void lista_remove_inteiro(ListaIntervalar* lista, int valor) {
    No* ant = lista->cabeca;
    No* atual = lista->cabeca->prox;

    while (atual != NULL) {
        if (valor < atual->dados.min) {
            return;
        }
        if (intervalo_contem(&atual->dados, valor)) {
            break;
        }
        ant = atual;
        atual = atual->prox;
    }

    if (atual == NULL) {
        return;
    }
    
    // Intervalo tem apenas um número
    if (atual->dados.min == atual->dados.max) {
        ant->prox = atual->prox;
        free(atual);
        
    } 

    // Remover extremidade esquerda
    else if (valor == atual->dados.min) {
        atual->dados.min = valor + 1;
        
    }

    // Remover extremidade direita
    else if (valor == atual->dados.max) {
        atual->dados.max = valor - 1;
        
    }

    // Remoção no meio --> divide em dois intervalos
    else {
        No* novo = (No*)malloc(sizeof(No));
        if (novo == NULL) {
            perror("Falha ao alocar nó");
            return;
        }
        novo->dados = cria_intervalo(valor + 1, atual->dados.max);
        atual->dados.max = valor - 1;
        novo->prox = atual->prox;
        atual->prox = novo;
    }
}

// Teste
int main() {
    printf(" Iniciando Teste  da Lista Intervalar \n");

    srand(time(NULL));

    ListaIntervalar* lista = cria_lista();
    
    bool sombra[INTERVALO_TESTE] = {false};

    for (int i = 0; i < NUM_OPERACOES; i++) {
        int op = rand() % 2; 
        int val = rand() % INTERVALO_TESTE; 

        if (op == 0) {
            lista_insere_inteiro(lista, val);
            sombra[val] = true;
        } else {
            lista_remove_inteiro(lista, val);
            sombra[val] = false;
        }
    }

    printf("... %d operações concluídas.\n", NUM_OPERACOES);
    printf("Estado final:\n");
    imprime_lista(lista);

    printf("Verificando corretude...\n");
    
    bool ok = true;
    for (int i = 0; i < INTERVALO_TESTE; i++) {
        bool lista_tem = lista_contem_inteiro(lista, i);
        bool sombra_tem = sombra[i];
   
        if (lista_tem != sombra_tem) {
            printf("ERRO! Valor: %d\n", i);
            printf("  Esperado: %s\n", sombra_tem ? "true" : "false");
            printf("  Obtido : %s\n", lista_tem ? "true" : "false");
            ok = false;
        }
    }

    if (ok) {
        printf("\nTudo certo: Todos os testes passaram!\n");
    } else {
        printf("\nErros encontrados.\n");
    }

    libera_lista(lista);

    return 0;
}
