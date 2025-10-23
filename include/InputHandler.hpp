#include "States.hpp"
#include <GLFW/glfw3.h>
#include <Material.hpp>
#include <Arena.hpp>
#include <World.hpp>

#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#define MOUSE_BUFFER_SIZE 10

enum MouseState
{
	RELEASED,
	RIGHT_PRESSED,
	RIGHT_PRESSED_DRAGGED,
	LEFT_PRESSED,
	LEFT_PRESSED_DRAGGED
};

enum WindowState
{
	MINIMIZED, 
	RESIZED,
	NORMAL_WIN
};

struct Mouse{
	vector2 mouse_buffer[MOUSE_BUFFER_SIZE];
	vector2 draw_size {11, 11};
	vector2 curr_mouse_pos {-1, -1};
	fvector2 scale {0, 0};
	MouseState state {RELEASED};
	size_t index;
};

struct AppInfo {
	Mouse* mouse_data;
	vector2 framebuffer_size;
	SimState sim_state;
	DrawState draw_state;
	WindowState win_state;
	MatType mat;
};

AppInfo* init_sim_info(GLFWwindow* window, Arena* arena);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_pos_callback(GLFWwindow* window, double xPos, double yPos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void process_input(AppInfo* app_info, World& world);

#endif
