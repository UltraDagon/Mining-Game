#include "functs.h"

// Remove this, SDL_RenderPolygon needs to be drawn zig zag
void old_SDL_RenderPolygon( SDL_Renderer &renderer, std::vector< SDL_Vertex > verts)
{
    float center_x = 0;
    float center_y = 0;

    for ( int i = 0; i < verts.size( ); i++ )
    {
        center_x += verts[ i ].position.x;
        center_y += verts[ i ].position.y;
    }

    center_x = center_x / verts.size( );
    center_y = center_y / verts.size( );
    
    std::vector < SDL_Vertex > polygon_verts;
    SDL_Vertex current_vert;
    SDL_Vertex next_vert;
    SDL_Vertex center_vert;

    for ( int i = 0; i < verts.size( ); i++ )
    {
        if ( i < verts.size( ) - 1 )
        {
            next_vert = { SDL_FPoint{ verts[i+1].position.x, verts[i+1].position.y }, verts[i].color, SDL_FPoint{ 0 } };
        } else {
            next_vert = { SDL_FPoint{ verts[0].position.x, verts[0].position.y }, verts[i].color, SDL_FPoint{ 0 } };
        }
        
        current_vert = { SDL_FPoint{ verts[i].position.x, verts[i].position.y }, verts[i].color, SDL_FPoint{ 0 } };
        center_vert = { SDL_FPoint{ center_x, center_y }, verts[i].color, SDL_FPoint{ 0 } };

        polygon_verts.emplace_back( current_vert );
        polygon_verts.emplace_back( next_vert );
        polygon_verts.emplace_back( center_vert );
    }

    SDL_RenderGeometry( &renderer, nullptr, polygon_verts.data( ), polygon_verts.size( ), nullptr, 0 );
};

void SDL_RenderPolygon( SDL_Renderer &renderer, std::vector< SDL_Vertex > verts)
{
    std::vector < SDL_Vertex > polygon_verts;
    SDL_Vertex vert_a;
    SDL_Vertex vert_b;
    SDL_Vertex vert_c;

    for ( int i = 0; i < verts.size( ) - 2; i++ )
    {
        vert_a = { SDL_FPoint{ verts[i].position.x, verts[i].position.y }, verts[i].color, SDL_FPoint{ 0 } };
        vert_b = { SDL_FPoint{ verts[i+1].position.x, verts[i+1].position.y }, verts[i+1].color, SDL_FPoint{ 0 } };
        vert_c = { SDL_FPoint{ verts[i+2].position.x, verts[i+2].position.y }, verts[i+2].color, SDL_FPoint{ 0 } };

        polygon_verts.emplace_back( vert_a );
        polygon_verts.emplace_back( vert_b );
        polygon_verts.emplace_back( vert_c );
    }

    SDL_RenderGeometry( &renderer, nullptr, polygon_verts.data( ), polygon_verts.size( ), nullptr, 0 );
};

void WORLD_RenderBasic( SDL_Renderer &renderer, int scale, float x_pos, float y_pos, SDL_Color outer_color, SDL_Color inner_color, bool full )
{
    std::vector< SDL_Vertex > outer_verts;
    std::vector< SDL_Vertex > inner_verts;

    if ( full )
    {
        outer_verts =
        {
            { SDL_FPoint{ x_pos + 8*scale, y_pos }, outer_color, SDL_FPoint{ 0 }, }, // 1
            { SDL_FPoint{ x_pos + 4*scale, y_pos + 7*scale }, outer_color, SDL_FPoint{ 0 }, }, // 2
            { SDL_FPoint{ x_pos + 4*scale, y_pos - 7*scale }, outer_color, SDL_FPoint{ 0 }, }, // 6
            { SDL_FPoint{ x_pos - 4*scale, y_pos + 7*scale }, outer_color, SDL_FPoint{ 0 }, }, // 3
            { SDL_FPoint{ x_pos - 4*scale, y_pos - 7*scale }, outer_color, SDL_FPoint{ 0 }, }, // 5
            { SDL_FPoint{ x_pos - 8*scale, y_pos }, outer_color, SDL_FPoint{ 0 }, }, // 4
        };
        
        inner_verts =
        {
            { SDL_FPoint{ x_pos + 6*scale, y_pos }, inner_color, SDL_FPoint{ 0 }, },
            { SDL_FPoint{ x_pos + 3*scale, y_pos + 5.25f*scale }, inner_color, SDL_FPoint{ 0 }, },
            { SDL_FPoint{ x_pos + 3*scale, y_pos - 5.25f*scale }, inner_color, SDL_FPoint{ 0 }, },
            { SDL_FPoint{ x_pos - 3*scale, y_pos + 5.25f*scale }, inner_color, SDL_FPoint{ 0 }, },
            { SDL_FPoint{ x_pos - 3*scale, y_pos - 5.25f*scale }, inner_color, SDL_FPoint{ 0 }, },
            { SDL_FPoint{ x_pos - 6*scale, y_pos }, inner_color, SDL_FPoint{ 0 }, },
        };
    }
    else
    {
        outer_verts =
        {
            { SDL_FPoint{ x_pos + 8*scale, y_pos }, outer_color, SDL_FPoint{ 0 }, },
            { SDL_FPoint{ x_pos + 4*scale, y_pos + 7*scale }, outer_color, SDL_FPoint{ 0 }, },
            { SDL_FPoint{ x_pos - 8*scale, y_pos }, outer_color, SDL_FPoint{ 0 }, },
            { SDL_FPoint{ x_pos - 4*scale, y_pos + 7*scale }, outer_color, SDL_FPoint{ 0 }, },
        };
        
        inner_verts =
        {
            { SDL_FPoint{ x_pos + 5*scale, y_pos + 1.75f*scale }, inner_color, SDL_FPoint{ 0 }, },
            { SDL_FPoint{ x_pos + 3*scale, y_pos + 5.25f*scale }, inner_color, SDL_FPoint{ 0 }, },
            { SDL_FPoint{ x_pos - 5*scale, y_pos + 1.75f*scale }, inner_color, SDL_FPoint{ 0 }, },
            { SDL_FPoint{ x_pos - 3*scale, y_pos + 5.25f*scale }, inner_color, SDL_FPoint{ 0 }, },
        };
    }

    SDL_RenderPolygon(renderer, outer_verts);
    SDL_RenderPolygon(renderer, inner_verts);
};

void WORLD_RenderCursor( SDL_Renderer &renderer, int scale, float x_pos, float y_pos, SDL_Color color )
{
    std::vector< SDL_Vertex > top =
    {
        { SDL_FPoint{ x_pos + 4*scale, y_pos - 7*scale }, color, SDL_FPoint{ 0 }, },
        { SDL_FPoint{ x_pos - 4*scale, y_pos - 7*scale }, color, SDL_FPoint{ 0 }, },
        { SDL_FPoint{ x_pos + 3*scale, y_pos - 5.25f*scale }, color, SDL_FPoint{ 0 }, },
        { SDL_FPoint{ x_pos - 3*scale, y_pos - 5.25f*scale }, color, SDL_FPoint{ 0 }, },
    };
    
    std::vector< SDL_Vertex > top_right =
    {
        { SDL_FPoint{ x_pos + 8*scale, y_pos }, color, SDL_FPoint{ 0 }, },
        { SDL_FPoint{ x_pos + 4*scale, y_pos - 7*scale }, color, SDL_FPoint{ 0 }, },
        { SDL_FPoint{ x_pos + 6*scale, y_pos }, color, SDL_FPoint{ 0 }, },
        { SDL_FPoint{ x_pos + 3*scale, y_pos - 5.25f*scale }, color, SDL_FPoint{ 0 }, },
    };

    std::vector< SDL_Vertex > bottom_right =
    {
        { SDL_FPoint{ x_pos + 8*scale, y_pos }, color, SDL_FPoint{ 0 }, },
        { SDL_FPoint{ x_pos + 4*scale, y_pos + 7*scale }, color, SDL_FPoint{ 0 }, },
        { SDL_FPoint{ x_pos + 6*scale, y_pos }, color, SDL_FPoint{ 0 }, },
        { SDL_FPoint{ x_pos + 3*scale, y_pos + 5.25f*scale }, color, SDL_FPoint{ 0 }, },
    };

    std::vector< SDL_Vertex > bottom =
    {
        { SDL_FPoint{ x_pos + 4*scale, y_pos + 7*scale }, color, SDL_FPoint{ 0 }, },
        { SDL_FPoint{ x_pos - 4*scale, y_pos + 7*scale }, color, SDL_FPoint{ 0 }, },
        { SDL_FPoint{ x_pos + 3*scale, y_pos + 5.25f*scale }, color, SDL_FPoint{ 0 }, },
        { SDL_FPoint{ x_pos - 3*scale, y_pos + 5.25f*scale }, color, SDL_FPoint{ 0 }, },
    };

    std::vector< SDL_Vertex > bottom_left =
    {
        { SDL_FPoint{ x_pos - 8*scale, y_pos }, color, SDL_FPoint{ 0 }, },
        { SDL_FPoint{ x_pos - 4*scale, y_pos + 7*scale }, color, SDL_FPoint{ 0 }, },
        { SDL_FPoint{ x_pos - 6*scale, y_pos }, color, SDL_FPoint{ 0 }, },
        { SDL_FPoint{ x_pos - 3*scale, y_pos + 5.25f*scale }, color, SDL_FPoint{ 0 }, },
    };

    std::vector< SDL_Vertex > top_left =
    {
        { SDL_FPoint{ x_pos - 8*scale, y_pos }, color, SDL_FPoint{ 0 }, },
        { SDL_FPoint{ x_pos - 4*scale, y_pos - 7*scale }, color, SDL_FPoint{ 0 }, },
        { SDL_FPoint{ x_pos - 6*scale, y_pos }, color, SDL_FPoint{ 0 }, },
        { SDL_FPoint{ x_pos - 3*scale, y_pos - 5.25f*scale }, color, SDL_FPoint{ 0 }, },
    };
    
    SDL_RenderPolygon(renderer, top);
    SDL_RenderPolygon(renderer, top_right);
    SDL_RenderPolygon(renderer, bottom_right);
    SDL_RenderPolygon(renderer, bottom);
    SDL_RenderPolygon(renderer, bottom_left);
    SDL_RenderPolygon(renderer, top_left);
};

int dot_product( std::pair < int, int > a, std::pair < int, int > b )
{
    return a.first * b.first + a.second * b.second;
};
