/*****************************************************
 * Copyright Grégory Mounié 2016                     *
 *           Frédéric Pétrot 2016                    *
 *                                                   *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <SDL/SDL.h>
#include <assert.h>

const int TAILLE_X = 800;
const int TAILLE_Y = 600;

/* parametres de l'affichage du carre */
static int x;
static int y;
static int delta_x;
static int delta_y;
static int taille;
static int couleur;

#define handle_error(msg) \
  do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_SDL_error(msg) \
  do { fprintf(stderr, "SDL :" msg ", ligne %d \n", __LINE__); exit(EXIT_FAILURE); } while (0)

static void put_pixel(SDL_Surface *surface, int x, int y, uint32_t pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    uint8_t *p = (uint8_t *) surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(uint16_t *) p = pixel;
        break;

    case 3:
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(uint32_t *) p = pixel;
        break;
    }
}

static void draw_ball(SDL_Surface *canvas)
{
    /* fond noir sur l'ancienne position */
    int noir = 0;
    for (int k = 0; k < taille; k++)
        for (int l = 0; l < taille; l++)
            put_pixel(canvas, x + k, y + l, noir);


    /* modification de position */
    x += delta_x;
    y += delta_y;

    /* inversion des directions et ajustement */
    if (x + taille > TAILLE_X) {
        x = TAILLE_X - taille;
        delta_x *= -1;
    }

    if (x < 0) {
        x = 0;
        delta_x *= -1;
    }

    if (y + taille > TAILLE_Y) {
        y = TAILLE_Y - taille;
        delta_y *= -1;
    }

    if (y < 0) {
        y = 0;
        delta_y *= -1;
    }

    /* dessin de la balle */
    for (int k = 0; k < taille; k++)
        for (int l = 0; l < taille; l++)
            put_pixel(canvas, x + k, y + l, couleur);
}


int main(int argc, char **argv)
{
    SDL_Surface *canvas = NULL;
    SDL_Surface *ecran = NULL;
    SDL_Event event;

    if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
        handle_SDL_error("Init");

    ecran = SDL_SetVideoMode(TAILLE_X, TAILLE_Y, 32, SDL_SWSURFACE);
    if (ecran == NULL)
        handle_SDL_error("SetVideoMode");

    SDL_WM_SetCaption("Mini carrés", NULL);

    srandom(getpid());

    x = random() % TAILLE_X;
    y = random() % TAILLE_Y;
    delta_x = random() % 5 + 1;
    delta_y = random() % 5 + 1;
    taille = random() % 20 + 10;
    couleur =
        (random() % 256 << 16) | (random() % 256 << 8) | (random() % 255);

  /********************
   * Le tampon a changer pour mettre en place le couplage mémoire
   * entre les différents processus.
   *********************/
    void *tampon = malloc(TAILLE_X * TAILLE_Y * 4);
    if (tampon == NULL)
        handle_error("malloc");

    canvas =
        SDL_CreateRGBSurfaceFrom(tampon, TAILLE_X, TAILLE_Y, 32, TAILLE_X * 4,
                                 0x0, 0x0, 0x0, 0x0);
    if (canvas == NULL)
        handle_SDL_error();

    assert(canvas->pixels == tampon);

    if (SDL_BlitSurface(canvas, NULL, ecran, NULL) == -1)
        handle_SDL_error("BlitSurface");

    if (SDL_Flip(ecran) == -1)
        handle_SDL_error("Flip");

    while (1) {
        SDL_Delay(20);

        draw_ball(canvas);

        if (SDL_BlitSurface(canvas, NULL, ecran, NULL) == -1)
            handle_SDL_error("BlitSurface");

        if (SDL_Flip(ecran) == -1)
            handle_SDL_error("Flip");

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                goto fin;
        }
    }

  fin:
    SDL_FreeSurface(canvas);
    free(tampon);
    SDL_Quit();

    return 0;
}
