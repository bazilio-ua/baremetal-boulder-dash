#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "game.h"
#include <string.h>
#include <assert.h>

typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    uint32_t tft_fb[BACKBUFFER_HEIGHT][BACKBUFFER_WIDTH];
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

volatile uint32_t* frame_buffer_init(void)
{
    /* Initialise the SDL*/
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL could not initialise! SDL_Error: %s\n", SDL_GetError() );
        exit( -1 );
    }

    SDL_SetEventFilter( quit_filter, NULL );

    m->window = SDL_CreateWindow( "Boulder Dash", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                  BACKBUFFER_WIDTH * 3, BACKBUFFER_HEIGHT * 3, 0 );
    assert( m->window );

    m->renderer = SDL_CreateRenderer( m->window, -1, SDL_RENDERER_SOFTWARE );
    assert( m->renderer );

    m->texture = SDL_CreateTexture( m->renderer, SDL_PIXELFORMAT_BGRA8888, SDL_TEXTUREACCESS_STATIC,
                                    BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT );
    assert( m->texture );

    SDL_SetTextureBlendMode( m->texture, SDL_BLENDMODE_BLEND );

    return (void*) m->tft_fb;
}

int frame_buffer_switch(int offset)
{
    (void) offset;

    int rslt = SDL_UpdateTexture( m->texture, NULL, m->tft_fb, BACKBUFFER_WIDTH * sizeof(uint32_t) );
    assert( 0 == rslt );
    rslt = SDL_RenderClear( m->renderer );
    assert( 0 == rslt );

    /* Update the renderer with the texture containing the rendered image */
    rslt = SDL_RenderCopy( m->renderer, m->texture, NULL, NULL );
    assert( 0 == rslt );

    SDL_RenderPresent( m->renderer );

    return 0;
}

bool keyPressed = false;
uint8_t keyVal = 0;
uint8_t poll_controller(uint8_t virtKey)
{
    (void)virtKey;
    SDL_Event event;

    while( SDL_PollEvent( &event ) )
    {
        switch( event.type )
        {
        case SDL_KEYDOWN:
            keyPressed = 1;
            switch( event.key.keysym.sym )
            {
            case SDLK_SPACE :
                keyVal = KEY_FIRE;
                break;

            case SDLK_KP_0 :
            case SDLK_0 :
                keyVal = KEY_ZERO;
                break;

            case SDLK_RIGHT:
            case SDLK_KP_PLUS:
                keyVal = KEY_RIGHT;
                break;

            case SDLK_LEFT:
            case SDLK_KP_MINUS:
                keyVal = KEY_LEFT;
                break;

            case SDLK_UP:
                keyVal = KEY_UP;
                break;

            case SDLK_DOWN:
                keyVal = KEY_DOWN;
                break;

            case SDLK_RETURN2 :
            case SDLK_RETURN :
                keyVal = KEY_FAIL;
                break;

            case SDLK_ESCAPE:
                exit( 1 );
                break;
            }
            break;

        case SDL_KEYUP:
            keyPressed = 0;
            keyVal = 0;
            break;

        default:
            break;

        }
    }

    SDL_Delay( 5 ); /* Sleep for 5 millisecond */

    return keyVal;
}
