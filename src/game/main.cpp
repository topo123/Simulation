#include <Shader.hpp>
#include <cassert>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Material.hpp>
#include <PoolArena.hpp>
#include <World.hpp>


double mouse_x, mouse_y;
bool paused = false;
bool debug_mode = false;
bool mousePressed = false;
bool erase = false;
MatType material_type = MatType::SAND;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_pos_callback(GLFWwindow* window, double xPos, double yPos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		paused = !paused;
	}
	else if(key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		material_type = MatType::SAND;
	}
	else if(key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
		material_type = MatType::WATER;
	}
	else if(key == GLFW_KEY_3 && action == GLFW_PRESS)
	{
		material_type = MatType::STONE;
	}
	else if(key == GLFW_KEY_4 && action == GLFW_PRESS)
	{
		material_type = MatType::ACID;
	}
	else if(key == GLFW_KEY_5 && action == GLFW_PRESS)
	{
		material_type = MatType::SMOKE;
	}
	else if(key == GLFW_KEY_6 && action == GLFW_PRESS)
	{
		material_type = MatType::WOOD;
	}
	else if(key == GLFW_KEY_7 && action == GLFW_PRESS)
	{
		material_type = MatType::FIRE;
	}
	else if(key == GLFW_KEY_8 && action == GLFW_PRESS)
	{
		material_type = MatType::OIL;
	}
	else if(key == GLFW_KEY_G && action == GLFW_PRESS)
	{
		debug_mode = !debug_mode;
	}
}

void cursor_pos_callback(GLFWwindow* window, double xPos, double yPos)
{
	mouse_x = xPos;
	mouse_y = yPos;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		mousePressed = true;
	}
	else if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		mousePressed = false;
	}

	if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		erase = true;
	}
	else if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		erase = false;
	}
}

void glfwErrorCallback(int error, const char* description) {
	std::cout << "GLFW Error (" << error << "): " << description << '\n';
}



int init_window(GLFWwindow** window)
{
	glfwSetErrorCallback(glfwErrorCallback);
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	*window = glfwCreateWindow(800, 600, "LearnOpenGl", NULL, NULL);
	if(window == NULL)
	{
		std::cout << "Failed to created window\n";
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(*window);
	if(glfwGetCurrentContext() == NULL)
	{
		std::cout << "No current context\n";
	}

	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD\n";
		const char* description;
		int code = glfwGetError(&description);
		if (description)
			std::cout << "GLFW error code: " << code << ", description: " << description << '\n';
		glfwTerminate();
		return -1;
	}

	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(*window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(*window, mouse_button_callback);
	glfwSetCursorPosCallback(*window, cursor_pos_callback);
	glfwSetKeyCallback(*window, key_callback);
	return 0;
}

void game_loop()
{
	GLFWwindow* window = nullptr;
	int success = init_window(&window);
	assert(success == 0);

	std::cout << std::to_string(sizeof(Material)) << '\n';
	std::cout << "Creating material arena\n";
	PoolArena* arena = init_pool(800 * 600, sizeof(Material));
	std::cout << "Creating world\n";
	World world;
	world.init_world(80, 60, 800, 600, arena);

	const unsigned int UPS = 120;
	const unsigned int FPS = 60;
	const float FPS_SLICE = 1.0f/FPS;
	const float UPS_SLICE = 1.0f/UPS;
	const unsigned int max_updates = 30;
	const unsigned int MAX_FRAME_SKIPS = 10;
	unsigned int frames_skip = 0;

	double elapsed_time;
	double time_accumulator = 0.0f;
	int updates = 0;

	int frames = 0;
	double frame_accumulator = 0;

	double nowTime = glfwGetTime();

	while(!glfwWindowShouldClose(window))
	{
		if(mousePressed)
		{
			world.create_materials(mouse_x, mouse_y, 11, 11, material_type);
		}
		if(erase)
		{
			world.delete_materials(mouse_x, mouse_y, 35, 35);
		}

		elapsed_time = glfwGetTime() - nowTime;
		nowTime = glfwGetTime();
		time_accumulator +=  elapsed_time;
		frame_accumulator += elapsed_time;

		while(time_accumulator >= UPS_SLICE && frames_skip < MAX_FRAME_SKIPS && !paused)
		{
			world.update_world(UPS_SLICE);
			time_accumulator -= UPS_SLICE;
			frames_skip ++;
		}
		frames_skip = 0;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		world.draw_world(debug_mode);
		frame_accumulator -= FPS_SLICE;
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);

}

int main()
{
	game_loop();
	glfwTerminate();
}
