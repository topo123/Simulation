#include <glad/glad.h>
#include <InputHandler.hpp>
#include <States.hpp>
#include <Arena.hpp>
#include <iostream>


AppInfo* init_sim_info(GLFWwindow* window, Arena* arena)
{
	arena = init_arena(sizeof(Mouse) + sizeof(AppInfo));

	AppInfo* glfw_info = (AppInfo*)allocate(arena, sizeof(AppInfo));

	glfw_info->mouse_data = (Mouse*)allocate(arena, sizeof(Mouse));
	glfw_info->mouse_data->scale = {1.5, 1.5};
	glfw_info->mouse_data->draw_size = {3, 3};
	glfw_info->mouse_data->index = 0;
	glfw_info->sim_state = RUNNING;
	glfw_info->draw_state = DEBUG_DRAW;
	glfw_info->win_state = NORMAL_WIN;
	glfw_info->mat = SAND;

	glfwSetWindowUserPointer(window, glfw_info);

	return glfw_info;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	AppInfo* state = static_cast<AppInfo*>(glfwGetWindowUserPointer(window));


	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		state->sim_state = state->sim_state == PAUSED? RUNNING: PAUSED;
	}
	else if(key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		state->mat = MatType::SAND;
	}
	else if(key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
		state->mat = MatType::WATER;
	}
	else if(key == GLFW_KEY_3 && action == GLFW_PRESS)
	{
		state->mat = MatType::STONE;
	}
	else if(key == GLFW_KEY_4 && action == GLFW_PRESS)
	{
		state->mat = MatType::ACID;
	}
	else if(key == GLFW_KEY_5 && action == GLFW_PRESS)
	{
		state->mat = MatType::SMOKE;
	}
	else if(key == GLFW_KEY_6 && action == GLFW_PRESS)
	{
		state->mat = MatType::WOOD;
	}
	else if(key == GLFW_KEY_7 && action == GLFW_PRESS)
	{
		state->mat = MatType::FIRE;
	}
	else if(key == GLFW_KEY_8 && action == GLFW_PRESS)
	{
		state->mat = MatType::OIL;
	}
	else if(key == GLFW_KEY_9 && action == GLFW_PRESS)
	{
		state->mat = MatType::FLAMMABLE_GAS;
	}
	else if(key == GLFW_KEY_G && action == GLFW_PRESS)
	{
		state->draw_state = state->draw_state == DEBUG_DRAW? NORMAL_DRAW: DEBUG_DRAW;
	}
	else if(key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		state->sim_state = SAVING;
	}
	else if(key == GLFW_KEY_EQUAL)
	{
		state->mouse_data->draw_size.x += 2;
		state->mouse_data->draw_size.y += 2;
		std::cout << "Draw size is: " << state->mouse_data->draw_size.x << std::endl;
	}
	else if(key == GLFW_KEY_MINUS)
	{
		state->mouse_data->draw_size.x = state->mouse_data->draw_size.x - 2 <= 0? 1: state->mouse_data->draw_size.x - 2;
		state->mouse_data->draw_size.y = state->mouse_data->draw_size.y - 2 <= 0? 1: state->mouse_data->draw_size.y - 2;
		std::cout << "Draw size is: " << state->mouse_data->draw_size.x << std::endl;
	}
}

void cursor_pos_callback(GLFWwindow* window, double xPos, double yPos)
{
	Mouse* mouse = static_cast<AppInfo*>(glfwGetWindowUserPointer(window))->mouse_data;

	if(mouse->state == LEFT_PRESSED)
	{
		mouse->state = LEFT_PRESSED_DRAGGED;
	}
	else if(mouse->state == RIGHT_PRESSED)
	{
		mouse->state = RIGHT_PRESSED_DRAGGED;
	}


	if(mouse->index < MOUSE_BUFFER_SIZE && (mouse->state == RIGHT_PRESSED_DRAGGED || mouse->state == LEFT_PRESSED_DRAGGED))
	{
		mouse->mouse_buffer[mouse->index].x = mouse->curr_mouse_pos.x;
		mouse->mouse_buffer[mouse->index].y = mouse->curr_mouse_pos.y;
		mouse->index ++;
	}
	else{
		mouse->index = 0;
	}

	mouse->curr_mouse_pos.x = xPos * mouse->scale.x;
	mouse->curr_mouse_pos.y = yPos * mouse->scale.y;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	std::cout << "Framebuffer width and height: " << width << ", " << height << std::endl;
	glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	Mouse* mouse = static_cast<AppInfo*>(glfwGetWindowUserPointer(window))->mouse_data;
	if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		mouse->state = LEFT_PRESSED;
	}
	else if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		mouse->state = RELEASED;
	}

	if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		mouse->state = RIGHT_PRESSED;
	}
	else if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		mouse->state = RELEASED;
	}
}


void process_input(AppInfo* app_info, World& world)
{		
	glfwPollEvents();

	SimState& state = app_info->sim_state;
	Mouse* mouse = app_info->mouse_data;

	if(mouse->state == LEFT_PRESSED_DRAGGED)
	{
		for(size_t i = 0; i < mouse->index; i ++)
		{
			if(i == mouse->index - 1)
			{
				world.create_materials(mouse->mouse_buffer[i], mouse->curr_mouse_pos, mouse->draw_size, app_info->mat);
			}
			else{
				world.create_materials(mouse->mouse_buffer[i], mouse->mouse_buffer[i + 1], mouse->draw_size, app_info->mat);
			}
		}
		mouse->index = 0;
	}
	else if(mouse->state == LEFT_PRESSED)
	{
		std::cout << "Left button pushed but not dragged\n";
		world.create_materials(mouse->curr_mouse_pos, mouse->curr_mouse_pos, mouse->draw_size, app_info->mat);
	}

	if(mouse->state == RIGHT_PRESSED || mouse->state == RIGHT_PRESSED_DRAGGED)
	{
		world.delete_materials(mouse->curr_mouse_pos.x, mouse->curr_mouse_pos.y, mouse->draw_size.x, mouse->draw_size.y);
	}

	if(state == SAVING)
	{
		world.save_world("test_world1.sim");
		app_info->sim_state = RUNNING;
	}

}
