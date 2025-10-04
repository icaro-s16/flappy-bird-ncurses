#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>

// força do pulo do personagem
#define J -22.0f
// velocidade de movimento dos pilares
#define v_p -20.0f
// força da gravidade
#define G 80.0f
// constante de ricochete do teto
#define R -0.33f
// tamanho da grossura dos canos
#define S_c 9
// define o valor de limite de spawn de um espaçamento no cano
#define M_c 10
// define o delay de spawn do segundo pilar
#define d_2p 2.5f
// define o espaço livre do cano
#define L_c 10
// bloqueador de spawn de colunas 
#define bloqueio_spawn -1
// definição do conjunto de variaveis que fazem parte do personagem
struct bird
{
    double y;
    double x;
    double v_y;
};
// define o conjunto de variaveis que fazem os pilares
struct pilar{
    double y;
    double x;
};
// Definições de tela
struct win_m
{
    int y;
    int x;
    int max_y;
    int min_y;
    int max_x;
    int min_x;
};

double d_framtime()
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return (double)time.tv_sec + (double)time.tv_usec / 1000000.0;
}

void mov(struct bird *bird, bool *jump, bool *gameover, const struct win_m game, const double frametime)
{
    // habilita o jump e altera a variavel velocidade para a força do pulo
    if (*(jump) == true)
    {
        bird->v_y = J;
        *jump = false;
    }
    // ele sobe se estiver dentro da área de movimentação
    if (bird->y <= game.max_y && bird->y >= game.min_y)
    {
        bird->y += bird->v_y * frametime;
    }
    if (bird->y >= game.max_y)
    {
        bird->y = game.max_y;
        *gameover = true;
    }
    if (bird->y <= game.min_y)
    {
        //pequeno ricochete se tiver batido no teto
        bird->v_y *= R;
        bird->y = game.min_y;
    }
}

void gravidade(struct bird *bird, const struct win_m game, const double framtime)
{
    if (bird->y < game.max_y)
    {
        // aciona a gravidade sobre toda e qualquer valor de velocidade se estiver com o pulo ela ira diminuir aos poucos
        // se tiver zerado aumentara a velocidade de queda aos poucos
        bird->v_y += G * framtime;
    }
}

// gera as cordenadas chaves do pilar e o movimenta no eixo x
void pilares(bool *gameover, bool *s_pilar, struct pilar *pilar,const struct bird bird, const struct win_m game, const double frametime){
    if(*s_pilar==true){
        pilar->y = rand() % ((game.max_y-M_c)-game.min_y+1)+game.min_y;
        pilar->x = game.x;
        *s_pilar = false;
    }
    else{
        pilar->x += v_p * frametime;
        if(pilar->x<game.min_x){
            *s_pilar = true;
            pilar->y = bloqueio_spawn;
        }
    }
}

// desenha o pilar e checa a colisão do personagem
void d_pilares(WINDOW *w_game, bool *gameover, struct pilar *pilar, int *ponto,const struct bird bird,const double frametime,const struct win_m game){
    if(pilar->y!=bloqueio_spawn){
        for(int i = game.min_y ; i<=game.max_y; i++){
            for(int j = 0; j < S_c; j++){
                if((i <= pilar->y || i >= pilar->y+L_c)&&pilar->x>game.min_x){
                    mvwaddch(w_game,  i, (int) (pilar->x-j) , '#');
                    if((int) bird.x == (int) pilar->x-j && (int) bird.y == i){
                        *gameover = true;
                    }
                }
            }
        }
    }
}

int main()
{
    srand(time(NULL));
    char pontuacao[15];
    int entrada; int pontos = 0;
    double randy = 0; double delay_segundo_pilar = 0; double delay_pontuacao = 0;
    bool game_over = false;
    bool jump = false;
    bool s_pilar[] = {true, true};
    WINDOW *game, *gameover, *stats;
    struct bird bird;
    struct win_m w_game;
    struct win_m w_gameover;
    struct win_m w_stats;
    // definição das estruturas da tela de pontos
    w_stats.x = 23;
    w_stats.y = 6;
    // definição das estruturas da tela de game over
    w_gameover.x = 20;
    w_gameover.y = 10;
    // definição das estruturas da tela de principal
    w_game.y = 55;
    w_game.x = 110;
    w_game.max_y = w_game.y - 2;
    w_game.min_y = 1;
    w_game.max_x = w_game.x - 2;
    w_game.min_x = 1;
    // define o array dos pilares
    // bolqueia o spawn do segundo pilar até o delay de spawn dele terminar
    struct pilar pilar[] = {{0, w_game.max_x}, {bloqueio_spawn, w_game.max_x}};
    if ((initscr()) == NULL )
    {
        fprintf(stderr, "erro ao iniciar a janela\n");
        return 1;
    };
    game = subwin(stdscr, w_game.y, w_game.x, (LINES - w_game.y) / 2, (COLS - w_game.x) / 2);
    stats = subwin(stdscr, w_stats.y, w_stats.x, (LINES-55) / 2, (COLS-170) / 2);
    keypad(game, true);
    wtimeout(game, 0);
    curs_set(0);
    // define a posição inicial do personagem
    bird.y = w_game.y / 2;
    bird.x = 19;
    bird.v_y = 0;
    // desenhos iniciais
    box(game, 0, 0);    
    box(stats, 0, 0);
    sprintf(pontuacao, "Pontos: %d", pontos);
    mvwaddstr(stats, w_stats.y/2, (w_stats.x-9)/2, pontuacao);
    mvwaddch(game, bird.y, bird.x, 'O');
    wrefresh(game);
    // determinação do frametime
    double t_passado = d_framtime();
    double t_atual;
    double frametime;
    while (game_over != true)
    {
        // recebe a constante que representa a tecla apertada pelo player
        entrada = wgetch(game);
        //determina o tempo que o último ciclo demorou para terminar criando uma variavel para uniformizar o tempo dos quadros 
        t_atual = d_framtime();
        frametime = t_atual - t_passado;
        delay_pontuacao += frametime;
        t_passado = t_atual;
        delay_segundo_pilar += frametime;
        //mapea a tecla de pulo e habilita o pulo
        if(delay_pontuacao>1){
            pontos += 1;
            delay_pontuacao = 0;
        }
        if (entrada == ' ' && bird.v_y>J+4)
        {
            jump = true;
        }
        // definição do buffer da pontuação
        sprintf(pontuacao, "Pontos: %d", pontos);
        //aplica a gravidade && limita os movimentos da tela && aplica a lógica do pulo
        mov(&bird, &jump, &game_over, w_game, frametime);
        gravidade(&bird, w_game, frametime);
        // delay para a criação do segundo pilar
        for(int i=0;i<2;i++){
            if(i==1 && delay_segundo_pilar<d_2p){
                continue;
            }
            else{
                pilares(&game_over, &s_pilar[i], &pilar[i], bird, w_game, frametime);    
            }
        }
        // desenha o personagem, a caixa da tela principal e apaga as alterações obsoletas
        werase(game);
        werase(stats);
        mvwaddch(game, (int)bird.y, (int)bird.x, 'O');
        //desenho dos pilares
        for(int i=0;i<2;i++){
            d_pilares(game, &game_over, &pilar[i], &pontos, bird, frametime, w_game);
        }
        box(game, 0, 0);
        box(stats, 0, 0);
        mvwaddstr(stats, w_stats.y/2, (w_stats.x-9)/2, pontuacao);
        wrefresh(game);
        wrefresh(stats);
    }
    wclear(game);
    gameover = subwin(stdscr, w_gameover.y, w_gameover.x, (LINES-w_gameover.y)/2, (COLS-w_gameover.x)/2);
    box(gameover, 0, 0);
    mvwaddstr(gameover, w_gameover.y / 2, (w_gameover.x - 10) / 2, "GAME OVER!");
    wrefresh(gameover);
    wgetch(gameover);
    delwin(game);
    endwin();
    return 0;
}
