/**
* Author: Matthew Gong
* Assignment: Pong Clone
* Date due: 2023-10-21, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define LOG(argument) std::cout << argument << '\n'
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include <cmath>


const int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;

const float BG_RED     = 0.9608f,
            BG_BLUE    = 0.9608f,
            BG_GREEN   = 0.9608f,
            BG_OPACITY = 1.0f;


const int VIEWPORT_X      = 0,
          VIEWPORT_Y      = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const char PLAYER1_SPRITE_FILEPATH[] = "resources/p1.png";
const char PLAYER2_SPRITE_FILEPATH[] = "resources/p2.png";
const char BALL_SPRITE_FILEPATH[] = "resources/sonic.png";
const char P1_WIN_SPRITE_FILEPATH[] = "resources/p1win.png";
const char P2_WIN_SPRITE_FILEPATH[] = "resources/p2win.png";



const float PLAYER_SPEED = 10.0f;


const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL  = 0,
            TEXTURE_BORDER   = 0;


const float MILLISECONDS_IN_SECOND = 1000.0;

SDL_Window* g_display_window;
bool g_game_is_running = true;

int winner = 0;
bool game_end = false;

ShaderProgram g_pong_program;

glm::mat4 g_view_matrix,
          g_projection_matrix;

float g_previous_ticks  = 0.0f;

float ball_vertices[] = {
    -0.25f, -0.25f, 0.25f, -0.25f, 0.25f, 0.25f,
    -0.25f, -0.25f, 0.25f, 0.25f, -0.25f, 0.25f
};

float text_vertices[] = {
    -1.5f, -0.25f, 1.5f, -0.25f, 1.5f, 0.25f,
    -1.5f, -0.25f, 1.5f, 0.25f, -1.5f, 0.25f
};


float player_vertices[] = {
    -0.1f, -0.75f, 0.1f, -0.75f, 0.1f, 0.75f,
    -0.1f, -0.75f, 0.1f, 0.75f, -0.1f, 0.75f
};


float player_texture_coordinates[] = {
    0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
};
bool isMultiPlayer = true;

const float GAME_MAX_X = 5.0f;
const float GAME_MIN_X = -5.0f;
const float GAME_MAX_Y = 3.75f;
const float GAME_MIN_Y = -3.75f;

const glm::vec3 PLAYER1_INIT_POS = glm::vec3(GAME_MIN_X+0.5, 0.0f, 0.0f),
                PLAYER2_INIT_POS = glm::vec3(GAME_MAX_X-0.5, 0.0f, 0.0f);



GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);

    return textureID;
}
struct Player{
    GLuint texture_id;
    glm::mat4 model_matrix;
    glm::mat4 tran_matrix;
    glm::vec3 position;
    glm::vec3 movement;
    float width;
    float height;
    float movement_speed;
    float* vertices;
    float* texture_coordinates;
    Player(){
    }
    Player(glm::vec3 start_pos, const char SPRITE_LOCATION[], float speed, float w, float h){
        model_matrix = glm::mat4(1.0f);
        model_matrix = glm::translate(model_matrix, start_pos);
        vertices = player_vertices;
        texture_coordinates = player_texture_coordinates;
        texture_id = load_texture(SPRITE_LOCATION);
        position = start_pos;
        movement = glm::vec3(0, 0, 0);
        movement_speed = speed;
        width = w;
        height = h;
        
        
    }
    void translate(float delta_time){
        model_matrix = glm::mat4(1.0f);
        position += movement * movement_speed * delta_time;
        position.y = fmax(GAME_MIN_Y, position.y);
        position.y = fmin(GAME_MAX_Y, position.y);
        model_matrix = glm::translate(model_matrix, position);
//        std::cout << model_matrix[ << std::endl;
    }
    void draw_texture(ShaderProgram& g_program ){
        glVertexAttribPointer(g_program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(g_program.positionAttribute);

        glVertexAttribPointer(g_program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
        glEnableVertexAttribArray(g_program.texCoordAttribute);

        g_pong_program.SetModelMatrix(model_matrix);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    void dummy_mode(){
        if(position.y == GAME_MIN_Y){
            movement = glm::vec3(0,1,0);
        }
        if(position.y == GAME_MAX_Y){
            movement = glm::vec3(0,-1,0);
        }
    }
};
struct Ball{
    GLuint texture_id;
    GLuint p1_win_id;
    GLuint p2_win_id;

    glm::mat4 model_matrix;
    glm::mat4 tran_matrix;
    glm::vec3 position;
    glm::vec3 movement;
    float spin;
    float spin_speed;
    float rotation;
    float width;
    float height;
    float movement_speed;
    float* vertices;
    float* texture_coordinates;
    Ball(){
        reset();
    }
    Ball(const char SPRITE_LOCATION[], const char P1_LOCATION[], const char P2_LOCATION[], float w , float h ){
        model_matrix = glm::mat4(1.0f);
        vertices = ball_vertices;
        texture_coordinates = player_texture_coordinates;
        texture_id = load_texture(SPRITE_LOCATION);
        p1_win_id = load_texture(P1_LOCATION);
        p2_win_id = load_texture(P2_LOCATION);
        width = w;
        height = h;
        reset();
    }
    float generateRandomStart(){
        int random = rand() % 10;
        random += 10;
        if(rand() % 2) random *= -1;

        if(rand()%2) random += 180;
        return glm::radians(float(random));
    }
    void reset(){
        winner = 0;
        rotation = generateRandomStart();
        movement_speed = 1.5f;
        movement = getMovement();
        position = glm::vec3(0,0,0);
        spin = 0;
        spin_speed = 0;
    }
    glm::vec3 getMovement(){
        return glm::vec3(cos(rotation), sin(rotation), 0);
    }

    void draw_texture(ShaderProgram& g_program){
        glVertexAttribPointer(g_program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(g_program.positionAttribute);

        glVertexAttribPointer(g_program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
        glEnableVertexAttribArray(g_program.texCoordAttribute);

        g_pong_program.SetModelMatrix(model_matrix);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        if(winner){
            
            glVertexAttribPointer(g_program.positionAttribute, 2, GL_FLOAT, false, 0, text_vertices);
            glEnableVertexAttribArray(g_program.positionAttribute);

            glVertexAttribPointer(g_program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
            glEnableVertexAttribArray(g_program.texCoordAttribute);
            glm::mat4 text_model = glm::translate(glm::mat4(1.0f), glm::vec3(winner*(GAME_MAX_X-1.5),GAME_MAX_Y-0.5, 0));

            g_pong_program.SetModelMatrix(text_model);
            if(winner == -1)
                glBindTexture(GL_TEXTURE_2D, p1_win_id);
            else
                glBindTexture(GL_TEXTURE_2D, p2_win_id);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }
    bool move(float delta_time, Player& p1, Player& p2){
        model_matrix = glm::mat4(1.0f);
        movement_speed += 0.01;
        spin_speed += 0.03;
        spin += spin_speed*delta_time;
        position +=  movement* movement_speed * delta_time;
        position.y = fmax(GAME_MIN_Y+height/2, position.y);
        position.y = fmin(GAME_MAX_Y-height/2, position.y);
        position.x = fmax(GAME_MIN_X, position.x);
        position.x = fmin(GAME_MAX_X, position.x);
        model_matrix = glm::translate(model_matrix, position);
        model_matrix = glm::rotate(model_matrix, spin, glm::vec3(0.0f, 0.0f, 1.0f));
        
        return detect_colision(p1,p2);
    }
    bool detect_player_colision(Player& p){
        float x_distance = fabs(position.x - p.position.x) - ((width + p.width) / 2.0f);
        float y_distance = fabs(position.y - p.position.y) - ((height + p.height) / 2.0f);

        if (x_distance < 0 && y_distance < 0)
        {
            return true;
        }
        return false;
    }
    bool AreSame(double a, double b)
    {
        return fabs(a - b) < 0.00001;
    }
    bool detect_colision(Player& p1, Player& p2){
        //bounce of top of the wall
        if(AreSame(position.y,GAME_MIN_Y+height/2) || AreSame(position.y, GAME_MAX_Y-height/2))movement.y *= -1;
        if(detect_player_colision(p1) || detect_player_colision(p2)){
            movement.x *= -1;
            position.x = fmax(p1.position.x+0.25, position.x);
            position.x = fmin(p2.position.x-0.25, position.x);
        }
        if(position.x == GAME_MIN_X || position.x == GAME_MAX_X){
            if(position.x == GAME_MIN_X){
                winner = 1;
            }else{
                winner = -1;
            }

            return true;
        }
        return false;
    }
};



Player player1;
Player player2;
Ball ball;

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Pong Mzg9288",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(GAME_MIN_X, GAME_MAX_X, GAME_MIN_Y, GAME_MAX_Y, -1.0f, 1.0f);
    g_pong_program.Load(V_SHADER_PATH, F_SHADER_PATH);

    player1 = Player(PLAYER1_INIT_POS, PLAYER1_SPRITE_FILEPATH,PLAYER_SPEED,0, 1.5);
    player2 = Player(PLAYER2_INIT_POS, PLAYER2_SPRITE_FILEPATH, PLAYER_SPEED, 0, 1.5);
    ball = Ball(BALL_SPRITE_FILEPATH,P1_WIN_SPRITE_FILEPATH,P2_WIN_SPRITE_FILEPATH, 0.5, 0.5);
    
    g_pong_program.SetProjectionMatrix(g_projection_matrix);
    g_pong_program.SetViewMatrix(g_view_matrix);

    glUseProgram(g_pong_program.programID);


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
}


void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running = !g_game_is_running;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_t:
                        isMultiPlayer = !isMultiPlayer;
                        player2.movement = glm::vec3(0,1,0);
                        break;
                    case SDLK_SPACE:
                        if(game_end == true){
                            g_previous_ticks = SDL_GetTicks()/ MILLISECONDS_IN_SECOND;
                            ball.reset();
                        }
                        game_end = false;
                        break;
                }
                break;
        }
    }
    player1.movement = glm::vec3(0,0,0);
    if (isMultiPlayer) player2.movement = glm::vec3(0,0,0);
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
    if (key_state[SDL_SCANCODE_W])
    {
        player1.movement.y += 1;
    }
    else if (key_state[SDL_SCANCODE_S])
    {
        player1.movement.y -= 1;
    }
    if (isMultiPlayer && key_state[SDL_SCANCODE_UP])
    {
        player2.movement.y += 1;
    }
    else if (isMultiPlayer && key_state[SDL_SCANCODE_DOWN])
    {
        player2.movement.y -= 1;
    }
    
}


void update()
{
    /** ———— DELTA TIME CALCULATIONS ———— **/
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    player1.translate(delta_time);
    if(!isMultiPlayer){
        player2.dummy_mode();
    }
    player2.translate(delta_time);
//    std::cout << ball.rotation << " " << ball.movement_speed << std::endl;
    if(ball.move(delta_time, player1, player2)){
        game_end = true;
    }
    

}


void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    player1.draw_texture(g_pong_program);
    player2.draw_texture(g_pong_program);
    ball.draw_texture(g_pong_program);
    
    glDisableVertexAttribArray(g_pong_program.positionAttribute);
    glDisableVertexAttribArray(g_pong_program.texCoordAttribute);
    
    

    SDL_GL_SwapWindow(g_display_window);
}


void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        if(!game_end) update();
        render();
    }

    shutdown();
    return 0;
}
