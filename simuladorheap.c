#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define max 200

#define firstfit 1
#define bestfit 2
#define worstfit 3
#define nextfit 4

typedef struct variavel{
    char nome[20];
    int tamanho;
    int inicio;
    struct variavel *prox;
	} variavel;

typedef struct bloco {
    int inicio;
    int tamanho;
    int livre;
    char id;
    struct bloco *prox;
    variavel *vars;
	} bloco;

char *heap = NULL;
int tamheap = 0;
bloco *inicioheap = NULL;
bloco *ultimobloco = NULL;
char proximo_id = 'A';
int algoritmo = firstfit;

void limpar_tela(){
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void configurar() {
    do{
        printf("Informe o tamanho do heap: ");
        scanf("%d", &tamheap);
        if (tamheap <= 0 || tamheap > max) {
            printf("\nTamanho invalido! Tente novamente.\n\n");
        	}
    	}while (tamheap <= 0 || tamheap > max);

    heap = (char *) malloc(tamheap * sizeof(char));
    if (heap == NULL){
        printf("Falha ao alocar memoria para o heap!\n");
        exit(1);
    	}
	}

void inicializar(){
    for (int i = 0; i < tamheap; i++){
        heap[i] = '.';
		}
    inicioheap = (bloco *) malloc(sizeof(bloco));
    inicioheap->inicio = 0;
    inicioheap->tamanho = tamheap;
    inicioheap->livre = 1;
    inicioheap->id = '-';
    inicioheap->prox = NULL;
    inicioheap->vars = NULL;
    ultimobloco = inicioheap;
	}

void mostrar_heap(){
    printf("Indices: ");
    for (int i = 0; i < tamheap; i++){
        printf("%02d ", i + 1);
    	}
    printf("\nHEAP:    ");
    for (int i = 0; i < tamheap; i++){
        printf(" %c ", heap[i]);
    	}

    printf("\n--------------------------------------------------------\n");
    printf("ID | Inicio | Tamanho | Livre\n");
    printf("---+---------+----------+------\n");

    for (bloco *b = inicioheap; b != NULL; b = b->prox){
        printf(" %c | %6d | %8d | %s\n", b->id, b->inicio + 1, b->tamanho, b->livre ? "Sim" : "Não");
    	}
    printf("========================================================\n");
    for (bloco *b = inicioheap; b != NULL; b = b->prox){
        if (!b->livre && b->vars){
            printf("\nVariaveis no bloco %c:\n", b->id);
            printf(" Nome               | Inicio | Tamanho\n");
            printf("--------------------+---------+---------\n");
            for (variavel *v = b->vars; v != NULL; v = v->prox){
                printf(" %-18s | %6d | %7d\n", v->nome, b->inicio + v->inicio + 1, v->tamanho);
            	}
        	}
    	}
	}

void juntar_blocos(){
    bloco *b = inicioheap;
    while (b != NULL && b->prox != NULL) {
        if (b->livre && b->prox->livre) {
            bloco *temp = b->prox;
            b->tamanho += temp->tamanho;
            b->prox = temp->prox;
            free(temp);
        	} 
		else {
            b = b->prox;
        	}
    	}
	}

void liberar(char id){
    bloco *b = inicioheap;
	while (b != NULL){
        if (!b->livre && b->id == id) {
            b->livre = 1;
            b->id = '-';
            variavel *v = b->vars;
            while (v){
                variavel *tmp = v;
                v = v->prox;
                free(tmp);
            	}
            b->vars = NULL;
            for (int i = b->inicio; i < b->inicio + b->tamanho; i++)
                heap[i] = '.';

            printf("\nBloco liberado.\n");
            juntar_blocos();
            return;
        	}
        b = b->prox;
    	}
	printf("\nBloco não encontrado!\n", id);
	}

void* alocar(int n){
    bloco *b = inicioheap;
    bloco *melhor = NULL;
    bloco *pior = NULL;
	switch (algoritmo){
        case 1:
            while (b != NULL) {
                if (b->livre && b->tamanho >= n) {
                    melhor = b;
                    break;
                	}
                b = b->prox;
            	}
            break;

        case 2:
            while (b != NULL) {
                if (b->livre && b->tamanho >= n) {
                    if (melhor == NULL || b->tamanho < melhor->tamanho)
                        melhor = b;
                	}
                b = b->prox;
            	}
            break;

        case 3:
            while (b != NULL) {
                if (b->livre && b->tamanho >= n) {
                    if (pior == NULL || b->tamanho > pior->tamanho)
                        pior = b;
                		}
                b = b->prox;
            	}
            melhor = pior;
            break;

        case 4: {
            bloco *inicio_busca = ultimobloco;
            b = inicio_busca;

            do {
                if (b->livre && b->tamanho >= n) {
                    melhor = b;
                    break;
                	}
                b = (b->prox != NULL) ? b->prox : inicioheap;
            } while (b != inicio_busca);

            if (melhor) ultimobloco = melhor;
            break;
        	}
    	}

    if (melhor == NULL) {
        printf("\nSem espaço suficiente para alocar %d unidades!\n", n);
        return NULL;
    	}

    if (melhor->tamanho > n) {
        bloco *novo = (bloco *) malloc(sizeof(bloco));
        novo->inicio = melhor->inicio + n;
        novo->tamanho = melhor->tamanho - n;
        novo->livre = 1;
        novo->id = '-';
        novo->vars = NULL;
        novo->prox = melhor->prox;
        melhor->prox = novo;
    	}

    melhor->tamanho = n;
    melhor->livre = 0;
    melhor->id = proximo_id++;
    melhor->vars = NULL;

    for (int i = melhor->inicio; i < melhor->inicio + n; i++)
        heap[i] = '#';

    printf("\nBloco %c alocado.\n", melhor->id);
    return (void*) melhor;
	}

void alocar_variavel(char bloco_id, char var_nome[], int tamanho){
    bloco *b = inicioheap;
	while (b != NULL) {
        if (b->id == bloco_id && !b->livre){
            int espaco_ocupado = 0;
            for (variavel *v = b->vars; v != NULL; v = v->prox)
                espaco_ocupado += v->tamanho;

            if (espaco_ocupado + tamanho > b->tamanho) {
                printf("\nEspaço insuficiente dentro do bloco %c!\n", bloco_id);
                return;
            	}

            variavel *nova = (variavel *) malloc(sizeof(variavel));
            strncpy(nova->nome, var_nome, 19);
            nova->nome[19] = '\0';
            nova->tamanho = tamanho;
            nova->inicio = espaco_ocupado;
            nova->prox = b->vars;
            b->vars = nova;

            printf("\nVariavel '%s' criada no bloco %c.\n", var_nome, bloco_id);
            return;
        	}
        b = b->prox;
    	}

    printf("\nBloco %c nao encontrado! Tente alocar um bloco antes de criar variaveis.\n", bloco_id);
	}


void escolher_algoritmo() {
    printf("Escolha o algoritmo:\n");
    printf("1 - First Fit\n");
    printf("2 - Best Fit\n");
    printf("3 - Worst Fit\n");
    printf("4 - Next Fit\n");
    scanf("%d", &algoritmo);
    printf("\nAlgoritmo selecionado: ");
    switch (algoritmo) {
        case 1: 
			printf("First Fit\n"); 
			break;
			
        case 2: 
			printf("Best Fit\n"); 
			break;
			
        case 3: 
			printf("Worst Fit\n"); 
			break;
			
        case 4: 
			printf("Next Fit\n"); 
			break;
			
        default:
            printf("Opcao invalida! Algoritmo padrao: First Fit\n");
            algoritmo = firstfit;
    	}
	}

void menu() {
    int opcao, tamanho;
    char id;
    char var[20];

    do {
        limpar_tela();
        printf("============== SIMULADOR DE HEAP ==============\n");
        printf("Algoritmo: ");
        switch (algoritmo) {
            case firstfit: 
				printf("First Fit\n"); 
				break;
			
            case bestfit:  
				printf("Best Fit\n"); 
				break;
			
            case worstfit: 
				printf("Worst Fit\n"); 
				break;
				
            case nextfit:  
				printf("Next Fit\n"); 
				break;
        	}
        printf("-----------------------------------------------\n");
        printf("1 - Mostrar heap\n");
        printf("2 - Alocar bloco\n");
        printf("3 - Liberar bloco\n");
        printf("4 - Alocar variavel dentro de bloco\n");
        printf("0 - Sair\n");
        printf("===============================================\n");
        printf("Escolha: ");
        scanf("%d", &opcao);

        limpar_tela();

        switch (opcao) {
            case 1:
                mostrar_heap();
                break;
            case 2:
                printf("Tamanho do bloco a alocar: ");
                scanf("%d", &tamanho);
                alocar(tamanho);
                break;
            case 3:
                printf("Informe o ID do bloco a liberar: ");
                scanf(" %c", &id);
                liberar(id);
                break;
            case 4:
                printf("Informe o ID do bloco: ");
                scanf(" %c", &id);
                printf("Nome da variavel: ");
                scanf("%s", var);
                printf("Tamanho da variavel: ");
                scanf("%d", &tamanho);
                alocar_variavel(id, var, tamanho);
                break;
            case 0:
                printf("\nEncerrando simulador...\n");
                break;
            default:
                printf("\nOpção inválida!\n");
        	}

        if (opcao != 0) {
            printf("\nPressione ENTER para voltar ao menu...");
            getchar(); getchar();
        	}
    	} while (opcao != 0);
	}

int main() {
    configurar();
    inicializar();
    escolher_algoritmo();
    menu();

    free(heap);
    return 0;
}

