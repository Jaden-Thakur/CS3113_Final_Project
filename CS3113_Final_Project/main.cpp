/**
* Author: Jaden Thakur
* Assignment: Platformer
* Date due: 2023-12-15, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

// defines

#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define LOG(argument) std::cout << argument << std::endl;
#define FIXED_TIMESTEP 0.0166666f
#define MAP_WIDTH 12
#define MAP_HEIGHT 12


// includes
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
//#include "stb_image.h"
#include <vector>
#include "Entity.h"
#include "Utility.h"
#include "Map.h"
#include "SDL_mixer.h"
#include "cmath"
#include <ctime>
#include <cstdlib>
#include <vector>
#include "Scene.h"
#include "Level0.h"
#include "Level1.h"
#include "Level2.h"
#include "Level3.h"


// ****CONSTANTS****
const int WINDOW_WIDTH = 1000,
WINDOW_HEIGHT = 600;

const float BG_RED = 0.388f, // #63ADF2
BG_GREEN = 0.678f,
BG_BLUE = 0.949f,
BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;

// ****GLOBALS****

Scene* g_active_scene;
Level0* g_level_0;
Level1* g_level_1;
Level2* g_level_2;
Level3* g_level_3;

Entity* player;
Entity* enemies;
Map* map;
Entity* sword;

Mix_Music* bgm;
Mix_Chunk* hit_sfx;

int g_life_count = 3;

bool g_win = false,
g_go = false;

SDL_Window* g_display_window;
bool g_game_is_running = true;


ShaderProgram g_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

// UI stuff
const char HEART_FILEPATH[] = "assets/heart_sprite.png";
const char FONT_FILEPATH[] = "assets/font.png";
Entity* lives;
int ENEMY_COUNT;
std::string endscreen_text;
glm::vec3 font_position;
GLuint font_texture_id;



// ****ADDITIONAL FUNCTIONS****
void switch_to_scene(Scene* scene)
{
    g_active_scene = scene;
    g_active_scene->initialize();
}


// ****BASIC FUNCTIONS****
void initialize()
{
    
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Not So Super Jaden",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    // Viewport Setup 
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(0.5f, 10.5f, -7.5f, 0.5f, -1.0f, 1.0f);

    g_program.set_projection_matrix(g_projection_matrix);
    g_program.set_view_matrix(g_view_matrix);

    // setting shader_program to use
    glUseProgram(g_program.get_program_id());

    // Colors the background as #63ADF2
    glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_OPACITY);

    // Font Stuff
    font_texture_id = Utility::load_texture(FONT_FILEPATH);
    

    // ————— LEVEL SETUP ————— //
    g_level_0 = new Level0();
    g_level_1 = new Level1();
    g_level_2 = new Level2();
    g_level_3 = new Level3();

    g_level_0->m_state.next_scene = g_level_1;
    g_level_1->m_state.next_scene = g_level_2;
    g_level_2->m_state.next_scene = g_level_3;
    g_level_3->m_state.next_scene = g_level_0;

    switch_to_scene(g_level_0);

    player = g_active_scene->m_state.player;
    enemies = g_active_scene->m_state.enemies;
    map = g_active_scene->m_state.map;
    sword = g_active_scene->m_state.sword;


    // LIVES SETUP
    lives = new Entity[g_life_count];
    for (int i = 0; i < g_life_count; i++) {
        lives[i].set_entity_type(LIFE);
        lives[i].set_position(glm::vec3(4.5f - i * 0.5 , -6.5, 0.0f));
        lives[i].m_texture_id = Utility::load_texture(HEART_FILEPATH);
        lives[i].set_height(0.4);
        lives[i].set_width(0.4);
        lives[i].scale();
    }

    /**
    BGM and SFX
    */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    bgm = Mix_LoadMUS("assets/bg_music.mp3");
    Mix_PlayMusic(bgm, -1);
    Mix_VolumeMusic(7.5f);

    hit_sfx = Mix_LoadWAV("assets/hit_fx.wav");
    Mix_VolumeChunk(
        hit_sfx,
        3.5f
    );

    // Blend Stuff
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    
    // Stop player from moving without input
 
    player->set_movement(glm::vec3(0.0f));
    
    // One Click Events
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_game_is_running = false;
            break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                // Quit
                g_game_is_running = false;
                break;
            case SDLK_SPACE:
                if (g_active_scene != g_level_0) {
                    sword->fire_height = player->get_position().y;
                }
                break;
            case SDLK_RETURN:
                if (g_active_scene == g_level_0) {
                    switch_to_scene(g_active_scene->get_next_scene());
                    player = g_active_scene->m_state.player;
                    enemies = g_active_scene->m_state.enemies;
                    map = g_active_scene->m_state.map;
                    sword = g_active_scene->m_state.sword;
                }
                break;
            default:
                break;
            }
        default:
            break;
        }
    }

    // Holding Down Keys
    const Uint8* key_state = SDL_GetKeyboardState(NULL);



    if (key_state[SDL_SCANCODE_A])
    {
        if (!sword->flying) {
            player->move_left();
            player->m_animation_indices = player->m_animation[player->LEFT];
            sword->m_animation_indices = sword->m_animation[sword->LEFT];
            sword->fire_dir = sword->LEFT;
        }
        
    }
    if (key_state[SDL_SCANCODE_D])
    {
        if (!sword->flying) {
            player->move_right();
            player->m_animation_indices = player->m_animation[player->RIGHT];
            sword->m_animation_indices = sword->m_animation[sword->RIGHT];
            sword->fire_dir = sword->RIGHT;
        }
       
    }
    if (key_state[SDL_SCANCODE_W])
    {
        if (!sword->flying) {
            player->move_up();
            sword->fire_dir = sword->UP;
            sword->m_animation_indices = sword->m_animation[sword->UP];
        }
        
        //player->m_animation_indices = player->m_animation[player->RIGHT];
    }
    if (key_state[SDL_SCANCODE_S])
    { 
        if (!sword->flying) {
            player->move_down();
            sword->fire_dir = sword->DOWN;
            sword->m_animation_indices = sword->m_animation[sword->DOWN];
        }
        
        //player->m_animation_indices = player->m_animation[player->RIGHT];
    }

    if (key_state[SDL_SCANCODE_SPACE]) {
        sword->flying = true;
    }
    else {
        sword->flying = false;
    }


    for (int i = 0; i < g_life_count; i++) {
        lives[i].set_position(glm::vec3(player->get_position().x - i * 0.5 - 3.5f, player->get_position().y + 2.5, 0.0f));
    }


    // normalize movement
    if (glm::length(player->get_movement()) > 1.0f)
    {
        player->set_movement(
            glm::normalize(
                player->get_movement()
            )
        );
    }


}

void update()
{
    // delta time
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    delta_time += g_accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP)
    {
            player->update(FIXED_TIMESTEP, player, enemies, g_active_scene->get_number_of_enemies(), map);
            sword->update(FIXED_TIMESTEP, player, enemies, g_active_scene->get_number_of_enemies(), map);

            for (size_t i = 0; i < g_active_scene->get_number_of_enemies(); i++) {
                enemies[i].update(FIXED_TIMESTEP, player, NULL, 0, map);
            }
            
            for (size_t i = 0; i < 3; i++) {
                lives[i].update(FIXED_TIMESTEP, player, NULL, 0, map);
            }
            delta_time -= FIXED_TIMESTEP;
    }

    g_accumulator = delta_time;


    // Level Shifter
    // Level 1 -> 2
    if (player->get_position().x >= 8 && player->get_position().y <= -21 && player->get_position().x <= 9 && player->get_position().y >= -22 && g_active_scene == g_level_1) {
        player->set_velocity(glm::vec3(0.0f, 0.0f, 0.0f));
        switch_to_scene(g_active_scene->get_next_scene());
        player = g_active_scene->m_state.player;
        enemies = g_active_scene->m_state.enemies;
        map = g_active_scene->m_state.map;
        sword = g_active_scene->m_state.sword;
    }
    // Level 2 -> 3
    if (player->get_position().x >= 7 && player->get_position().y <= -8 && player->get_position().x <= 8 && player->get_position().y >= -9 && g_active_scene == g_level_2) {
        player->set_velocity(glm::vec3(0.0f, 0.0f, 0.0f));
        switch_to_scene(g_active_scene->get_next_scene());
        player = g_active_scene->m_state.player;
        enemies = g_active_scene->m_state.enemies;
        map = g_active_scene->m_state.map;
        sword = g_active_scene->m_state.sword;
    }
    // Level 3 -> Endcard
    if (player->get_position().x >= 7.5 && player->get_position().y <= -16 && player->get_position().x <= 8.5 && player->get_position().y >= -17 && g_active_scene == g_level_3) {
        player->set_velocity(glm::vec3(0.0f, 0.0f, 0.0f));
        /*switch_to_scene(g_active_scene->get_next_scene());
        player = g_active_scene->m_state.player;
        enemies = g_active_scene->m_state.enemies;
        map = g_active_scene->m_state.map;
        sword = g_active_scene->m_state.sword;*/
        g_win = true;
    }

    if (player->dead && g_life_count > 0) {
        player->set_position(glm::vec3(player->get_position().x - 2, player->get_position().y, player->get_position().z));
        lives[g_life_count - 1].deactivate();
        g_life_count--;
        player->dead = false;
    }
    
    if (g_life_count == 0) {
        player->deactivate();
        sword->deactivate();
    }

    if (sword->hit) {
        Mix_PlayChannel(-1, hit_sfx, 0);
        sword->hit = false;
    }
    
    LOG(player->get_position().x << ", " << player->get_position().y);

    g_view_matrix = glm::mat4(1.0f);
    g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-player->get_position().x + 5.5, -player->get_position().y - 3, 0.0f));

        
    }
    

    



void render()
{
    g_program.set_view_matrix(g_view_matrix);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(g_program.get_program_id());

    // render level
    g_active_scene->render(&g_program); 

    // render hearts
    if (g_active_scene != g_level_0) {
        for (size_t i = 0; i < g_life_count; i++) {
            lives[i].render(&g_program);
        }
    }
        
    // render UI element based on win or lose
    if (g_win) {
        LOG(g_win)
        endscreen_text = "You Won!";
        font_position = glm::vec3(player->get_position().x - 2.0f, player->get_position().y, 0.0f);
        Utility::draw_text(&g_program, font_texture_id, endscreen_text, 0.5f, 0.000001f, font_position);
        player->deactivate();
        sword->deactivate();
    }
    if (g_life_count == 0) {
        endscreen_text = "You Lost!";
        font_position = glm::vec3(player->get_position().x - 2.0f, player->get_position().y, 0.0f);
        Utility::draw_text(&g_program, font_texture_id, endscreen_text, 0.5f, 0.000001f, font_position);
        player->deactivate();
    }


    // Swap window
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();

    delete g_level_0;
    delete g_level_1;
    delete g_level_2;
    delete g_level_3;
    delete[] lives;

    Mix_FreeChunk(hit_sfx);
    Mix_FreeMusic(bgm);
}

// ****GAME LOOP****
int main(int argc, char* argv[])
{
    initialize();

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}