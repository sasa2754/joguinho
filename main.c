// gcc  -I. -o joguinho main.c -lpthread
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <windows.h>
#include <conio.h>

#include "scores.h"
#include "scenario.h"
#include "structs.h"
#include "terminal.c"

#define ALTURA_TERMINAL 20
#define LARGURA_TERMINAL 40
#define BONECO "^~^"
int map[HEIGHT][WIDTH];

pthread_mutex_t mutex;
pthread_cond_t cond_var;
int should_cenario = 0;
int pontos = 0;
bool jogo_ativo = true;
bool colisao_detectada = false;

typedef struct {
    int x, y;
    int pulando;
} boneco_t;

boneco_t *boneco;


void* morceguinho(void *arg) {
    boneco->x = 5;
    boneco->y = ALTURA_TERMINAL / HEIGHT;
    boneco->pulando = 0;

    int pulo_atual = 0;

    while (jogo_ativo) {
        pthread_mutex_lock(&mutex);
        while (should_cenario == 1) {
            pthread_cond_wait(&cond_var, &mutex);
        }

        if (_kbhit()) {
            char tecla = _getch();
            
            if (tecla == ' ') {
                if (!boneco->pulando) {
                    boneco->pulando = 1;
                    pulo_atual = 0;
                }
            }
        }

        if (boneco->pulando) {
            if (pulo_atual < 4) {
                boneco->y--;
                pulo_atual++;
            } else {
                boneco->pulando = 0;
            }
            // Sleep(50);
        } else {
            boneco->y++;
        }

        if (boneco->y >= HEIGHT - 1) {
            boneco->y = HEIGHT - 1;
        }

        // Verifica colisão com algum obstáculo
        if (map[boneco->y][boneco->x] != 0 && map[boneco->y][boneco->x] != 2 && map[boneco->y][boneco->x] != 9) {
            colisao_detectada = true;  // Marca a colisão detectada
            jogo_ativo = false;  // Marca o jogo como não ativo
            // Sem break aqui para permitir o término do loop principal no main
        }

        Sleep(25);
        should_cenario = 1;
        pthread_cond_signal(&cond_var);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void *cenarioLindo(void *arg) {
    printf("Thread cenarioLindo started.\n");
    pthread_mutex_lock(&mutex);
    while (!should_cenario) {
        pthread_cond_wait(&cond_var, &mutex);
    }

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            map[i][j] = 0;
        }
    }

    Pipe first_pipe = construct_pipe();
    Pipe second_pipe = construct_pipe();
    Pipe third_pipe = construct_pipe();

    int y1  = 5;
    int y2  = 13;
    int y3  = 9;

    first_pipe.x = WIDTH - 4;
    first_pipe.y = y1;
    second_pipe.x = -1;
    third_pipe.x = -1;

    for (int i = (HEIGHT - y1); i < (HEIGHT); i++) {
        map[i][first_pipe.x] = 1;
        map[i][first_pipe.x + 1] = 1;
        map[i][first_pipe.x + 2] = 1;
    }

    for (int i = (HEIGHT - y1) - SPACE_BTW; i >= 0; i--) {
        map[i][first_pipe.x] = 1;
        map[i][first_pipe.x + 1] = 1;
        map[i][first_pipe.x + 2] = 1;
    }

    while (1) {
        Sleep(150);
        ERASE_ALL();
        MOVE_HOME();

        if (first_pipe.x == 0) {
            restart_pipe(map, &first_pipe);
        }

        if (second_pipe.x == 0) {
            restart_pipe(map, &second_pipe);
        }

        if (third_pipe.x == 0) {
            restart_pipe(map, &third_pipe);
        }

        if (first_pipe.x == WIDTH - 16) {
            second_pipe.x = WIDTH - 4;
            second_pipe.y = y2;
        }

        if (second_pipe.x == WIDTH - 16) {
            third_pipe.x = WIDTH - 4;
            third_pipe.y = y3;
        }

        if (first_pipe.x > 0) {
            pipe_movement(map, &first_pipe);
            first_pipe.x--; 
        }

        if (second_pipe.x > 0) {
            pipe_movement(map, &second_pipe);
            second_pipe.x--;
        }

        if (third_pipe.x > 0) {
            pipe_movement(map, &third_pipe);
            third_pipe.x--;
        }

        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                if (map[i][j] == 9) map[i][j] = 0;
            }
        }

        // Colisão
        if (map[boneco->y][boneco->x] != 0 && map[boneco->y][boneco->x] != 2 && map[boneco->y][boneco->x] != 9) {
            jogo_ativo = false;  // Marca o jogo como não ativo
            ERASE_ALL();
            printf("Voce perdeu!");
            break;  // Sai do loop principal
        }

        // Coloca as coordenadas do boneco na matriz
        map[boneco->y][boneco->x] = 9;

        // Contagem de pontos caso o boneco passe do cano
        if (boneco->x == first_pipe.x || boneco->x == second_pipe.x || boneco->x == third_pipe.x) {
            pontos++;
        }

        printf("%d - %d - %d | Pontos: %d\n", first_pipe.x, second_pipe.x, third_pipe.x, pontos);

        rendering(map);
        fflush(stdout);

        should_cenario = 0;
        pthread_cond_signal(&cond_var);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    Player player;
    printf("Digite seu nome: ");
    scanf("%s", player.name);
    configureTerminal();
    Queue queue = construct_fila();
    pthread_t threadA, threadB;

    FILE *ranking;
    ranking = fopen("ranking.txt", "a");

    MOVE_HOME();


    boneco = (boneco_t*)malloc(sizeof(boneco_t));

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_var, NULL);

    pthread_create(&threadA, NULL, cenarioLindo, NULL);
    pthread_create(&threadB, NULL, morceguinho, NULL);

    // Espera pela finalização das threads
    pthread_join(threadB, NULL);
    pthread_join(threadA, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_var);

    player.score = pontos;

    // Se uma colisão foi detectada, ainda assim, permite salvar no arquivo de ranking
    if (colisao_detectada) {
        printf("Voce perdeu!");
        // Exibe a pontuação final
        fprintf(ranking, "Nome: %s | Pontos: %d\n", player.name, player.score);

    }

    fclose(ranking);
    free(boneco);
    return 0;
}
















































































































//        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//       < :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: >
//       < ::::::::::::            ::::::::::::            :::::::::::: >
//       < :::::::::                   ::::                   ::::::::: >
//       < :::::::         ********     ::     ********         ::::::: >
//       < :::::      ****          ****  ****         ****       ::::: >
//       < ::::     ****               ****               ****     :::: >
//       < :::     ****                 **                 ****     ::: >
//       < :::     ****                                    ****     ::: >
//       < ::::     ****                EU                *****    :::: >
//       < :::::     ****               TE               ****     ::::: >
//       < :::::::     ****             AMO            ****     ::::::: >
//       < :::::::::     ****                        ****     ::::::::: >
//       < :::::::::::     ****                    ****     ::::::::::: >
//       < ::::::::::::::      ****            ****      :::::::::::::: >
//       < :::::::::::::::::       ****    ****       ::::::::::::::::: >
//       < ::::::::::::::::::::        ****        :::::::::::::::::::: >
//       < :::::::::::::::::::::::      **      ::::::::::::::::::::::: >
//       < :::::::::::::::::::::::::          ::::::::::::::::::::::::: >
//       < :::::::::::::::::::::::::::      ::::::::::::::::::::::::::: >
//       < :::::::::::::::::::::::::::::  ::::::::::::::::::::::::::::: >
//       < :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: >
//         ()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()
