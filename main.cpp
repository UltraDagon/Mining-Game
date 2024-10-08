#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <SDL2/SDL.h>
#include "functs.h"

using namespace std;

const int WIDTH = 1080, HEIGHT = 720;
const int RUNNING_ACCEL = 1;

struct
{
    //Maybe put global access to renderer in here? not sure pros/cons of it
    SDL_FPoint offset = {0, 0};
    float zoom = 0;
} Camera;

struct Tile
{
    public:
        string material;
        bool full;
        
        Tile( string _material = "air", bool _full = true )
        {
            material = _material;
            full = _full;
        }
};

//https://stackoverflow.com/questions/1729699/how-do-i-correctly-use-sdl-freesurface-when-dealing-with-a-vector-of-surfaces
class image_cache
{
    private:
        map< string, SDL_Surface* > cache_;

    public:
        //Filename is input
        SDL_Surface* get_image( string file )
        {
            map< string, SDL_Surface* >::iterator i = cache_.find( file );
            if ( i == cache_.end( ) ) {
                SDL_Surface* surf = SDL_LoadBMP( file.c_str( ) );
                i = cache_.insert( i, make_pair( file, surf ) );
            }
            return i->second;
        }
        
        void flush( )
        {
            map< string, SDL_Surface* >::iterator i = cache_.begin();
            for ( ; i != cache_.end( ); ++i )
                { SDL_FreeSurface( i->second ); }
            cache_.clear() ;
        }

        ~image_cache( ) { flush( ); }
};

class World
{
    private:
        map< pair< int, int >, Tile> tiles;
    
    public:
        int width;
        int height;
        int scale;
        int corner_x;
        int corner_y;
        SDL_Renderer *renderer;

        World( SDL_Renderer &_renderer, int _scale, int _width, int _height )
        {
            renderer = &_renderer;
            scale = _scale;
            width = _width;
            height = _height;
            corner_x = 100;
            corner_y = 180;

            for ( int w = 0; w < width; w++ )
            {
                for ( int h = 0; h < height; h++ )
                {
                    tiles[ pair( w, h ) ] = Tile( "dirt" );
                }
            }

            for ( int w = 0; w < width; w++ )
            {
                tiles[ pair(w, 0) ].material = "air";
                if (w%2 == 0)
                    tiles[ pair(w, 0) ].full = false;
            }

            tiles[ pair(3, 2) ].material = "air";
            //tiles[ pair(4, 2) ].material = "air";
            tiles[ pair(3, 1) ].material = "air";
            tiles[ pair(4, 1) ].material = "air";
            tiles[ pair(5, 1) ].material = "air";
            tiles[ pair(6, 2) ].material = "air";
            tiles[ pair(6, 3) ].material = "air";
            tiles[ pair(7, 1) ].material = "air";
            tiles[ pair(5, 2) ].material = "air";
            //tiles[ pair(5, 2) ].full = false;
            //tiles[ pair(7, 2) ].full = false;
            tiles[ pair(2, 0) ].material = "grass";
            tiles[ pair(1, 0) ].material = "grass";
        }

        Tile tile_at( pair< int, int > pos )
        {
            return tiles[ pos ];
        }

        void render( )
        {   
            for ( int w = 0; w < width; w++ )
            {
                for ( int h = 0; h < height; h++ )
                {
                    int pos_x = corner_x + 12*scale*w;
                    int pos_y = corner_y + 14*scale*h + 7*scale*( w % 2 ); // Lowers every other tile by half height

                    if ( tiles[ pair( w, h ) ].material == "dirt" )
                        { WORLD_RenderBasic( *renderer, scale, pos_x, pos_y, SDL_Color{ 150, 75, 0, 255 }, SDL_Color{ 200, 100, 0, 255 }, tiles[ pair( w, h) ].full ); }
                    else if ( tiles[ pair( w, h ) ].material == "grass" )
                        { WORLD_RenderBasic( *renderer, scale, pos_x, pos_y, SDL_Color{ 31, 91, 47, 255 }, SDL_Color{ 47, 159, 63, 255 }, tiles[ pair( w, h) ].full ); }
                }
            }
        }

        Tile tile_at( int x, int y )
        { return tiles[ pair( x, y ) ]; }

        void destroy_tile( int x, int y )
        {   
            tiles[ pair( x, y ) ].material = "air";
        }

        void create_tile( int x, int y, string mat )
        {   
            tiles[ pair( x, y ) ].material = mat;
        }
};

class Player
{
    private:
        image_cache* images;
        int scale;
        bool grounded;
        bool jumping;
        bool mining;
        double accelX; // -1 for left, 0 for still, 1 for right
        double animFrame;
        double animMaxFrame;
        double animSpeedScale = 0.1; // 1 is 1 frame per 1/60 seconds, 0.1 is 1 frame per 1/6 seconds
        string currentAnim;
        pair< int, int > mining_tile;

        vector < pair< int, int > > generate_scalar_vector ( vector < pair< int, int > > vertices, vector < pair< int, int > > axis )
        {
            pair< int, int > next;
            pair< int, int > add;
            bool axis_has;
            int i;
            int j;

            for ( int k = 0; k < vertices.size( ); k++ )
            {
                if ( k < vertices.size( ) - 1 )
                { next = vertices[ k + 1 ]; }
                else
                { next = vertices[ 0 ]; }

                i = vertices[ k ].first - next.first;
                j = vertices[ k ].second - next.second;

                if ( i == 0 )
                { add = pair( 0, 1 ); }
                else if ( j == 0 )
                { add = pair( 1, 0 ); }
                else
                {
                    if ( i < 0 )
                    { i *= -1; j *= -1; }

                    add = pair( j, i );
                }

                axis_has = false;
                for(const pair< int, int >& k : axis)
                {
                    if (add == k)
                    { axis_has = true; }
                }

                if ( !axis_has )
                { axis.emplace_back( add ); }
            }
            return axis;
        }

        //Takes in exact coords of polygons on screen
        bool SAT_collision_check ( vector < pair< int, int > > obj_a, vector < pair< int, int > > obj_b)
        {
            vector < pair< int, int > > axis;
            axis = generate_scalar_vector( obj_a, axis );
            axis = generate_scalar_vector( obj_b, axis );

            int obj_a_min;
            int obj_a_max;
            int obj_b_min;
            int obj_b_max;

            pair < int, int > k;

            for ( const pair< int, int >& k : axis )
            {
                obj_a_min = dot_product( obj_a[ 0 ], k );
                obj_a_max = dot_product( obj_a[ 0 ], k );
                for ( int a = 1; a < obj_a.size( ); a++ )
                {
                    if ( dot_product( obj_a[ a ], k ) < obj_a_min )
                    { obj_a_min = dot_product( obj_a[ a ], k ); }
                    if ( dot_product( obj_a[ a ], k ) > obj_a_max )
                    { obj_a_max = dot_product( obj_a[ a ], k ); }
                }
                
                obj_b_min = dot_product( obj_b[ 0 ], k );
                obj_b_max = dot_product( obj_b[ 0 ], k );
                for ( int b = 1; b < obj_b.size( ); b++ )
                {
                    if ( dot_product( obj_b[ b ], k ) < obj_b_min )
                    { obj_b_min = dot_product( obj_b[ b ], k ); }
                    if ( dot_product( obj_b[ b ], k ) > obj_b_max )
                    { obj_b_max = dot_product( obj_b[ b ], k ); }
                }

                if ( ( obj_a_max < obj_b_min || obj_b_max < obj_a_min ) )
                { return false; }
            }

            return true;
        }
        
        void change_animation( string texture_name )
        {
            if ( currentAnim != texture_name )
            {
                SDL_Surface* surface = images->get_image( "assets/"+texture_name+".bmp" );
                currentAnim = texture_name;
                player_texture = SDL_CreateTextureFromSurface( renderer, surface );
                animFrame = 0;
                animMaxFrame = surface->w / (scale*15);
            }
        }

    public:
        SDL_Renderer* renderer;
        SDL_Texture* player_texture;
        SDL_Rect position;
        SDL_FPoint velocity;

        Player( SDL_Renderer &_renderer, image_cache* images_, int scale_, int pos_x, int pos_y )
        {
            renderer = &_renderer;

            position.x = pos_x;
            position.y = pos_y;
            position.w = 15*scale_;
            position.h = 25*scale_;

            velocity = {0, 0};
            accelX = 0;
            grounded = false;
            jumping = false;
            
            images = images_;
            scale = scale_;
            
            change_animation( "playerIdleRight" );
        }

        ~Player( )
        {
            SDL_DestroyTexture( player_texture );
        }

        void render( )
        {
            SDL_RenderCopy( renderer, player_texture, new SDL_Rect{(int)floor(animFrame)*position.w,0,((int)floor(animFrame)+1)*position.w,position.h}, &position );
            //SDL_RenderCopy( renderer, player_texture, NULL, &position );

            animFrame += animSpeedScale;
            if ( animFrame >= animMaxFrame )
            { animFrame = 0; }

            cout << "Accel:" << accelX << ", Velocity:" << velocity.x << endl;
        }

        //Note for self: make it so that while not grounded, treat collision on blocks under the current left and right walls as left and right walls
        bool colliding( World world, pair< int, int > tile_pos, pair< int, int > offset = pair( 0, 0 ))
        {
            bool tile_full = world.tile_at( tile_pos ).full;

            if ( world.tile_at( tile_pos ).material == "air" )
            { return false; }

            int tile_x = world.corner_x + 12*world.scale*tile_pos.first;
            int tile_y = world.corner_y + 14*world.scale*tile_pos.second + 7*world.scale*( tile_pos.first % 2 );

            vector < pair< int, int > > player_vertices =
            {
                pair( position.x + offset.first, position.y - 1 + offset.second ),
                pair( position.x + offset.first + position.w, position.y - 1 + offset.second ),
                pair( position.x + offset.first + position.w, position.y - 1 + offset.second + position.h ),
                pair( position.x + offset.first, position.y - 1 + offset.second + position.h ),
            };

            vector < pair< int, int > > tile_vertices;
            if ( tile_full )
            {
                tile_vertices = 
                {
                    pair( tile_x + 8*world.scale, tile_y ),
                    pair( tile_x + 4*world.scale, tile_y + 7*world.scale ),
                    pair( tile_x - 4*world.scale, tile_y + 7*world.scale ),
                    pair( tile_x - 8*world.scale, tile_y ),
                    pair( tile_x - 4*world.scale, tile_y - 7*world.scale ),
                    pair( tile_x + 4*world.scale, tile_y - 7*world.scale ),
                };
            }
            else
            {
                tile_vertices = 
                {
                    pair( tile_x + 8*world.scale, tile_y ),
                    pair( tile_x + 4*world.scale, tile_y + 7*world.scale ),
                    pair( tile_x - 4*world.scale, tile_y + 7*world.scale ),
                    pair( tile_x - 8*world.scale, tile_y ),
                };
            }

            return ( SAT_collision_check( player_vertices, tile_vertices ) );
        }

        void walk_right ( bool walk )
        {
            if ( walk )
            {
                accelX = RUNNING_ACCEL;
                change_animation( "playerRunRight" );
            }
            else if ( accelX == RUNNING_ACCEL )
            {
                accelX = 0;
                change_animation( "playerIdleRight" );
            }
        }

        void walk_left ( bool walk )
        {
            if ( walk )
            {
                accelX = -RUNNING_ACCEL;
                change_animation( "playerRunLeft" );
            }
            else if ( accelX == -RUNNING_ACCEL )
            {
                accelX = 0;
                change_animation( "playerIdleLeft" );
            }
        }

        void jump ( bool up )
        {
            if ( up )
            { jumping = true; }
            else
            { jumping = false; }
        }

        void break_tile ( bool mine )
        {
            if (mine)
            { mining = true; }
        }

        void move ( World world )
        {
            int grav = 1; // REMOVE DEBUG TOOL
            
            //Walking
            if (accelX == 0 && velocity.x > 0)
            { velocity.x -= 1; }
            else if (accelX == 0 && velocity.x < 0)
            { velocity.x += 1; }
            if ( abs(velocity.x + accelX) <= 3 )
            { velocity.x += accelX; }

            //grounded = 1;
            if ( jumping && grounded )
            {
                grounded = 0;
                velocity.y = -10;
            }
            
            //gravity
            velocity.y += 1;

            position.y += velocity.y;

            int left_x = ( position.x - world.corner_x ) / ( 12*world.scale );
            double double_floor_y = (double)( position.y + position.h - world.corner_y) / ( double )( 14*world.scale ) - 0.5;
            int floor_y = ceil( double_floor_y );

            //cout << " | " << (int)(2.0*double_floor_y) % 2 << " | ";
            //cout << "Left x: " << left_x << ", Floor y: " << floor_y;
            // right wall
            for (int i = 0; i < velocity.x; i++ )
            {
                if ( ( colliding( world, pair( left_x + 2, floor_y - 1), pair( 1, 0 ) ) && !( !( ( int )( 2.0*double_floor_y ) % 2 ) && left_x % 2 && grounded) ) ||
                     colliding( world, pair( left_x + 2, floor_y - 2), pair( 1, 0 ) ) ||
                     colliding( world, pair( left_x + 2, floor_y - 3), pair( 1, 0 ) ) ||
                     ( colliding( world, pair( left_x + 2, floor_y), pair( 1, 0 ) ) && !grounded ) ||
                     ( colliding( world, pair( left_x + 1, floor_y - 3 ) ) || colliding( world, pair( left_x + 1, floor_y - 2 ) ) && colliding( world, pair( left_x + 2, floor_y ) ) ) // Stops walking up slants when ceiling should stop you
                   )
                { break; }

                position.x += 1;
            }

            // left wall
            for (int i = 0; i > velocity.x; i-- )
            {
                if ( ( colliding( world, pair( left_x, floor_y - 1), pair( -1, 0 ) ) && !( !( ( int )( 2.0*double_floor_y ) % 2 ) && left_x % 2 && grounded) ) ||
                     colliding( world, pair( left_x, floor_y - 2), pair( -1, 0 ) ) ||
                     colliding( world, pair( left_x, floor_y - 3), pair( -2, 0 ) ) ||
                     ( colliding( world, pair( left_x, floor_y), pair( -1, 0 ) ) && !grounded ) ||
                     ( colliding( world, pair( left_x + 1, floor_y - 3 ) ) || colliding( world, pair( left_x + 1, floor_y - 2 ) ) && colliding( world, pair( left_x, floor_y ) ) )
                   )
                { break; }

                position.x -= 1;
            }

            // ceiling
            while ( colliding( world, pair( left_x, floor_y - 3 ), pair( 0, 1 ) ) ||
                    colliding( world, pair( left_x, floor_y - 2 ), pair( 0, 1 ) ) ||
                    colliding( world, pair( left_x + 1, floor_y - 3 ), pair( 0, 1 ) ) ||
                    colliding( world, pair( left_x + 1, floor_y - 2 ), pair( 0, 1 ) ) ||
                    colliding( world, pair( left_x + 2, floor_y - 3 ), pair( 0, 1 ) ) ||
                    colliding( world, pair( left_x + 2, floor_y - 2 ), pair( 0, 1 ) )
                  )
            {
                position.y += 1;
                velocity.y = 0;
            }

            // the floor_y - 1 checks might be avoidable if theres a better way to manage floor_y ^
            grounded = 0;
            while ( colliding( world, pair( left_x, floor_y ) ) ||
                    colliding( world, pair( left_x, floor_y - 1 ) ) ||
                    colliding( world, pair( left_x + 1, floor_y ) ) ||
                    colliding( world, pair( left_x + 1, floor_y - 1 ) ) ||
                    colliding( world, pair( left_x + 2, floor_y ) ) ||
                    colliding( world, pair( left_x + 2, floor_y - 1 ) )
                  )
            {
                position.y -= 1;
                velocity.y = 0;
                grounded = 1;
            }
            
        }

        void mine ( World &world, int mouse_x, int mouse_y )
        {
            int tile_x = ceil( ( double )( mouse_x - world.corner_x ) / ( double )( 12*world.scale ) - 0.5 );
            int tile_y = ceil( ( double )( mouse_y - world.corner_y) / ( double )( 14*world.scale ) + 0.5 * ( ( tile_x + 1 ) % 2 ) ) - 1;

            if ( mining )
            {
                if ( world.tile_at( tile_x, tile_y).material != "air" )
                { world.destroy_tile( tile_x, tile_y ); }
                else
                { world.create_tile( tile_x, tile_y, "grass" ); }
                mining = false;
            }
        }

        //I think i can get rid of sdl_rendederer & renederer here
        void render_cursor ( SDL_Renderer& renderer, World world, int mouse_x, int mouse_y )
        {
            int tile_x = ceil( ( double )( mouse_x - world.corner_x ) / ( double )( 12*world.scale ) - 0.5 );
            int tile_y = ceil( ( double )( mouse_y - world.corner_y) / ( double )( 14*world.scale ) + 0.5 * ( ( tile_x + 1 ) % 2 ) ) - 1;

            //cout << ". Tile x: " << tile_x << ", Tile y: " << tile_y << endl;

            int render_x = world.corner_x + 12*world.scale*tile_x;
            int render_y = world.corner_y + 14*world.scale*tile_y + 7*world.scale*( tile_x % 2 );

            WORLD_RenderCursor( renderer, world.scale, render_x, render_y, SDL_Color{ 0, 200, 0, 63 } );
        }
};

int main( int argc, char *argv[] )
{
    bool keydown = false;
    image_cache images;
    int mouse_x, mouse_y;
    SDL_Init( SDL_INIT_EVERYTHING );

    SDL_Window *window = SDL_CreateWindow( "Hello SDL World", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI );
    SDL_Renderer *renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if ( NULL == window )
    {
        cout << "Could not create window: " << SDL_GetError << endl;
        return 1;
    }

    SDL_Event event;
    
    int scale = 4;
    World world( *renderer, scale, 15, 10 );
    Player player( *renderer, &images, scale, 150, 00 );

    bool running = true;
    double deltaTime,oldTime = 0;
    while ( running )
    {
        while ( SDL_PollEvent( &event ) )
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    running = false;
                    break;

                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    keydown = (SDL_KEYDOWN == event.type);

                    if ( SDLK_d == event.key.keysym.sym )
                    { player.walk_right( keydown ); }

                    if ( SDLK_a == event.key.keysym.sym )
                    { player.walk_left( keydown ); }

                    if ( SDLK_w == event.key.keysym.sym )
                    { player.jump( keydown ); }
                    
                    if ( SDLK_s == event.key.keysym.sym )
                    { player.position.y += 1; }
                    
                    break;
                
                case SDL_MOUSEBUTTONDOWN:
                    player.break_tile( true );
                    break;
            }
        }

        SDL_GetMouseState( &mouse_x, &mouse_y );
        player.move( world );
        player.mine( world, mouse_x, mouse_y );

        SDL_SetRenderDrawColor( renderer, 191, 191, 255, 255 );
        SDL_RenderClear( renderer );
        world.render( );
        player.render( );
        player.render_cursor( *renderer, world, mouse_x, mouse_y );
        SDL_RenderPresent( renderer );

        //Framerate and deltaTime
        deltaTime = clock() - oldTime;
        double fps = (1.0 / deltaTime) * 1000;
        oldTime = clock();
    }

    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_Quit( );

    return EXIT_SUCCESS;
}