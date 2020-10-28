#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "game.h"
#include "buttons.h"
#include <string.h>
#include <assert.h>

typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    uint16_t tft_fb[2][SCREEN_HEIGHT][SCREEN_WIDTH];
} monitor_t;

static monitor_t monitor = { 0 };
monitor_t *m = &monitor;

int quit_filter(void *userdata, SDL_Event *event)
{
    (void) userdata;

    if( SDL_WINDOWEVENT == event->type )
    {
        if( SDL_WINDOWEVENT_CLOSE == event->window.event )
        {
            exit( 0 );
        }
    }
    else if( SDL_QUIT == event->type )
    {
        exit( 0 );
    }

    return 1;
}

volatile unsigned char* frame_buffer_init(void)
{
    /*Initialize the SDL*/
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL could not initialise! SDL_Error: %s\n", SDL_GetError() );
        exit( -1 );
    }

    SDL_SetEventFilter( quit_filter, NULL );

    m->window = SDL_CreateWindow( "Boulder Dash", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                  SCREEN_WIDTH, SCREEN_HEIGHT, 0 );
    assert( m->window );

    m->renderer = SDL_CreateRenderer( m->window, -1, SDL_RENDERER_SOFTWARE );
    assert( m->renderer );

    m->texture = SDL_CreateTexture( m->renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STATIC,
                                    SCREEN_WIDTH, SCREEN_HEIGHT );
    assert( m->texture );

    SDL_SetTextureBlendMode( m->texture, SDL_BLENDMODE_BLEND );

    return (void*) m->tft_fb;
}

int frame_buffer_switch(int offset)
{
    (void) offset;

    int rslt = SDL_UpdateTexture( m->texture, NULL, m->tft_fb, SCREEN_WIDTH * sizeof(uint16_t) );
    assert( 0 == rslt );
    rslt = SDL_RenderClear( m->renderer );
    assert( 0 == rslt );

    /* Update the renderer with the texture containing the rendered image */
    rslt = SDL_RenderCopy( m->renderer, m->texture, NULL, NULL );
    assert( 0 == rslt );

    SDL_RenderPresent( m->renderer );

    return 0;
}

uint8_t poll_controller(void)
{
    uint8_t output = 0;

    SDL_Event event;

    while( SDL_PollEvent( &event ) )
    {
        switch( event.type )
        {
        case SDL_KEYDOWN:
            switch( event.key.keysym.sym )
            {
            case SDLK_RIGHT:
            case SDLK_KP_PLUS:
                output |= 1;
                break;

            case SDLK_LEFT:
            case SDLK_KP_MINUS:
                output |= 2;
                break;

            case SDLK_UP:
                output |= 8;
                break;

            case SDLK_DOWN:
                output |= 4;
                break;

            case SDLK_ESCAPE:
                exit( 1 );
                break;
            }
            break;

        case SDL_KEYUP:
        default:
            break;

        }
    }

    if( 0 == output )
    {
        SDL_Delay( 5 ); /* Sleep for 5 millisecond */
    }
    return output;
}
