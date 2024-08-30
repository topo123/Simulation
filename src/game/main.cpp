#include <Shader.hpp>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Material.hpp>
#include <PoolArena.hpp>
#include <World.hpp>


double mouse_x, mouse_y;
bool mousePressed = false;
MatType material_type = MatType::SAND;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_pos_callback(GLFWwindow* window, double xPos, double yPos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		material_type = MatType::SAND;
	}
	else if(key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
		material_type = MatType::WATER;
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
}



int init_window(GLFWwindow** window)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	*window = glfwCreateWindow(800, 600, "LearnOpenGl", NULL, NULL);

	glfwMakeContextCurrent(*window);

	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD\n";
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
	init_window(&window);

	std::cout << std::to_string(sizeof(Material)) << '\n';
	PoolArena* arena = init_pool(800 * 600, sizeof(Material));
	World world;
	world.init_world(80, 60, 800, 600);


	const unsigned int UPS = 100;
	const float UPS_SLICE = 1.0f;
	const unsigned int max_updates = 10;
	const unsigned int MAX_FRAME_SKIPS = 10;
	unsigned int frames_skip = 0;

	double nowTime = glfwGetTime();
	double elapsed_time;
	double time_accumulator = 0.0f;







	while(!glfwWindowShouldClose(window))
	{
		if(mousePressed)
		{
			std::cout << "Creating material" << '\n';
			world.create_materials(mouse_x, mouse_y, 19,  19,  material_type, arena);
		}
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		elapsed_time = glfwGetTime() - nowTime;
		nowTime = glfwGetTime();
		time_accumulator +=  elapsed_time;

		while(time_accumulator >= UPS_SLICE && frames_skip < MAX_FRAME_SKIPS)
		{
			world.update_world();
			time_accumulator -= UPS_SLICE;
			frames_skip ++;
		}

		world.draw_world();
		frames_skip = 0;
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

int main()
{
	game_loop();
}
