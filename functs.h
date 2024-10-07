#include <SDL2/SDL.h>
#include <vector>

#ifndef FUNCTS_H
#define FUNCTS_H

void SDL_RenderPolygon( SDL_Renderer &renderer, std::vector< SDL_Vertex > verts );

void new_SDL_RenderPolygon( SDL_Renderer &renderer, std::vector< SDL_Vertex > verts );

void WORLD_RenderBasic( SDL_Renderer &renderer, int scale, float x_pos, float y_pos, SDL_Color outer_color, SDL_Color inner_color, bool full);

void WORLD_RenderCursor( SDL_Renderer &renderer, int scale, float x_pos, float y_pos, SDL_Color color );

int dot_product( std::pair < int, int > a, std::pair < int, int > b );

#endif