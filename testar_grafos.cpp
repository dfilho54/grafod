#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "timer.h"

typedef struct {
	int *cidades;
	int cost;
	int num_cidades;
} tour_t;

typedef struct {
  tour_t *contents;
  int size;
  int top;
} pilha_t;

// Fun��es da pilha
void pilha_init(pilha_t *stack);
void pilha_destroy(pilha_t *stack);
int pilha_vazia(pilha_t *stack);
int pilha_cheia(pilha_t *stack);
void pilha_push(pilha_t *stack, tour_t element);
tour_t pilha_pop(pilha_t *stack);
void print_pilha(pilha_t stack);

// Fun��es do tour
void init_tour(tour_t *t, int cidade_inicial, int num_cidades);
tour_t copy_tour(tour_t *t, int num_cidades);
void update_tour(tour_t *best_tour, tour_t *t, int num_cidades);
int check_tour(tour_t *t, int cidade, int cidade_inicial, int num_cidades);
int check_best_tour(tour_t *best_tour, tour_t *t);
void add_cidade(tour_t *t, int cidade, int num_cidades, int **rotas);
void remove_cidade(tour_t *t, int cidade, int num_cidades, int **rotas);
int cidade_no_tour(tour_t *t, int cidade);
void print_tour(tour_t *t);
void calcular_custo_minimo(tour_t *tour_inicial, tour_t *best_tour, int num_cidades, int cidade_inicial, int **rotas);


int main(int argc, char *argv[]){
	int i, j, thread_count;
	int num_cidades;
	int cidade_inicial;
	int **matriz;
	double start, end;
	tour_t t, best_tour;
	tour_t *tours_iniciais;

	best_tour.cost = 99999;
	best_tour.cidades = NULL;

	thread_count = strtol(argv[1], NULL, 10);

	scanf("%d", &num_cidades);
	scanf("%d", &cidade_inicial);

	tours_iniciais = (tour_t*) malloc((num_cidades-1)*(sizeof(tour_t)));
	matriz = (int**) malloc(num_cidades*sizeof(int*));

	for (i = 0; i < num_cidades; i++) {
		matriz[i] = (int*) malloc(num_cidades*sizeof(int));
		for (j = 0; j < num_cidades; j++) {
			scanf("%d", &matriz[i][j]);
		}
	}

	init_tour(&t, cidade_inicial, num_cidades);

	GET_TIME(start);


#	pragma omp parallel for num_threads(thread_count)
	for (i = 0; i < num_cidades; i++) {
		if (i != cidade_inicial) {
			tour_t tour_inicial;
			init_tour(&tour_inicial, cidade_inicial, num_cidades);
			add_cidade(&tour_inicial, i, num_cidades, matriz);

			calcular_custo_minimo(&tour_inicial, &best_tour, num_cidades, cidade_inicial, matriz);
		}
	}


	GET_TIME(end);

	print_tour(&best_tour);

	printf("Tempo de execu��o: %f\n", end - start);

	// Liberar recursos
	free(best_tour.cidades);
	free(tours_iniciais);
	free(matriz);

	return 0;
}


// Fun��es da pilha

void pilha_init(pilha_t *stack) {
  stack->contents = (tour_t *) malloc(sizeof(tour_t) * 1);

  stack->size = 1;
  stack->top = -1;
}

void pilha_destroy(pilha_t *stack) {
  free(stack->contents);

  stack->contents = NULL;
  stack->size = 0;
  stack->top = -1;
}

int pilha_vazia(pilha_t *stack) {
  return (stack->top < 0);
}

int pilha_cheia(pilha_t *stack) {
  return (stack->top >= stack->size - 1);
}

void pilha_push(pilha_t *stack, tour_t element) {
  if (pilha_cheia(stack)) {
  	stack->size++;
    stack->contents = (tour_t *) realloc(stack->contents, sizeof(tour_t) * stack->size);
  }

  stack->contents[++stack->top] = element;
}

tour_t pilha_pop(pilha_t *stack) {
  if (pilha_vazia(stack)) {
    fprintf(stderr, "Can't pop element from stack: stack is empty.\n");
    exit(1);
  }

  return stack->contents[stack->top--];
}

void print_pilha(pilha_t stack) {
	int i;

	while (!pilha_vazia(&stack)) {
		tour_t t = pilha_pop(&stack);

		for (i = 0; i < t.num_cidades; i++) {
			printf("%d ", t.cidades[i]);
		}

		printf("\n");
	}
}


// Fun��es do tour

void init_tour(tour_t *t, int cidade_inicial, int num_cidades) {
	t->cidades = (int*) malloc((num_cidades+1)*sizeof(int));
	t->cost = 0;
	t->num_cidades = 1;
	t->cidades[0] = cidade_inicial;
}

tour_t copy_tour(tour_t *t, int num_cidades) {
	tour_t k;
	int i;
	k.num_cidades = t->num_cidades;
	k.cost = t->cost;
	k.cidades = (int*) malloc((num_cidades+1)*sizeof(int));
	for (i = 0; i < k.num_cidades; ++i)
	{
		k.cidades[i] = t->cidades[i];
	}
	return k;
}

void update_tour(tour_t *best_tour, tour_t *t, int num_cidades) {
	if (check_best_tour(best_tour, t)) {
		*best_tour = copy_tour(t, num_cidades);
	}
}

int check_tour(tour_t *t, int cidade, int cidade_inicial, int num_cidades) {
	if (t->num_cidades == num_cidades && cidade == cidade_inicial) {
		return 1;
	} else if (!cidade_no_tour(t, cidade)) {
		return 1;
	}

	return 0;
}

int check_best_tour(tour_t *best_tour, tour_t *t) {
	if (t->cost < best_tour->cost  || best_tour->cidades == NULL) {
		return 1;
	}

	return 0;
}

void add_cidade(tour_t *t, int cidade, int num_cidades, int **rotas) {
	int cidadeOrigem = t->cidades[t->num_cidades - 1];
	t->cost += rotas[cidadeOrigem][cidade];
	t->cidades[t->num_cidades++] = cidade;
}

void remove_cidade(tour_t *t, int cidade, int num_cidades, int **rotas) {
	t->num_cidades--;
	int cidadeOrigem = t->cidades[t->num_cidades - 1];
	t->cost -= rotas[cidadeOrigem][cidade];
}

int cidade_no_tour(tour_t *t, int cidade) {
	int i;

	for (i = 0; i < t->num_cidades; i++) {
		if (t->cidades[i] == cidade) {
			return 1;
		}
	}

	return 0;
}

void print_tour(tour_t *t) {
	int i;
	printf("Rota ");

	for (i = 0; i < t->num_cidades; i++) {
		printf("%d ", t->cidades[i]);
	}

	printf("\n");
	printf("Custo = %d\n", t->cost);
}

void calcular_custo_minimo(tour_t *tour_inicial, tour_t *best_tour, int num_cidades, int cidade_inicial, int **rotas) {
	int i, stop;
	tour_t t;
	pilha_t pilha;
	pilha_init(&pilha);

	pilha_push(&pilha, *tour_inicial);
	while (!pilha_vazia(&pilha)) {
		t = pilha_pop(&pilha);

		if (t.num_cidades == num_cidades) {
			add_cidade(&t, cidade_inicial, num_cidades, rotas);
		#	pragma omp critical
			update_tour(best_tour, &t, num_cidades);
		} else {
			for (i = num_cidades - 1; i >= 0; i--) {
				if (check_tour(&t, i, cidade_inicial, num_cidades)) {
					add_cidade(&t, i, num_cidades, rotas);
				#	pragma omp critical
					stop = check_best_tour(best_tour, &t);

					if (stop) {
						tour_t k = copy_tour(&t, num_cidades);
						pilha_push(&pilha, k);
					}
					remove_cidade(&t, i, num_cidades, rotas);
				}
			}
		}

		free(t.cidades);
	}

	pilha_destroy(&pilha);
}
