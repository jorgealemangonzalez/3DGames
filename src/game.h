/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	This class encapsulates the game, is in charge of creating the game, getting the user input, process the update and render.
*/

#ifndef GAME_H
#define GAME_H

#include "includes.h"
#include "camera.h"
#include "controller.h"
#include "player.h"

#include <fstream>

#define PLAYING 1
#define GAME_OVER 2
#define RESET 3
#define MENU 4      // Salir del juego, cambiar de partida (reiniciar), reanudar
#define MAIN_MENU 5
#define LEVEL 6

#define LEVEL_TUTORIAL "data/level_tutorial.txt"
#define LEVEL_NIVEL1 "data/level1.txt"
#define LEVEL_DEBUG "data/level_debug.txt"


class Game
{
public:
	static Game* instance;

	void resetGame();

	~Game();

	Human* human;
	Enemy* enemy;
	Player* getTeamPlayer(std::string team);
	Player* getEnemyTeamPlayer(std::string team);
	int gameState;
	bool humanWins;		//If true team human wins, else enemy wins
	std::string selectedLevel;

	//window
	SDL_Window* window;
	int window_width;
	int window_height;
    
	long frame;
    float time;
	float elapsed_time;
	int fps;
	float angle;

	bool pause;

	//keyboard state
	const Uint8* keystate;

	//mouse state
	int mouse_state; //tells which buttons are pressed
	Vector2 mouse_position; //last mouse position					// y 0 up (inverted)
	Vector2 mouse_delta; //mouse movement in the last frame
	bool mouse_locked; //tells if the mouse is locked (not seen)
	Vector2 mouse_when_press;										// y 0 down

	bool mouseLeft, mouseRight;	// For some reason 'mouse_state & SDL_BUTTON_RIGHT' doesn't work, so we keep the boolean
	
	Camera* camera; //our global game camera

	Game(SDL_Window* window);
	void init( void );
	void render( void );
	void update( double dt );

	void onKeyPressed( SDL_KeyboardEvent event );
	void onMouseButton( SDL_MouseButtonEvent event );
	void onMouseButtonUp( SDL_MouseButtonEvent event );
	void onMouseWheel( SDL_MouseWheelEvent event);
    void onResize( SDL_Event e );
    
	void setWindowSize(int width, int height);
};


#endif 