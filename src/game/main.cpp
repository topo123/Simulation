#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Material.hpp"
#include "Renderer.hpp"
#include "Updater.hpp"

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

void spawn_materials(int center_x, int center_y, int width, int height, MaterialState& mat_state, PoolArena* arena, std::vector<std::vector<Material*>>& grid)
{

	if(width % 2 == 0 || height % 2 == 0)
	{
		return;
	}

	if(center_x < 0 || center_x > 800 || center_y < 0 || center_y > 600)
	{
		return;
	}

	int num_cols = width;
	int num_rows = height;

	int half_x = width/2;
	int half_y = height/2;

	int lX = center_x - half_x;
	int rX = center_x + half_x;
	int tY = center_y - half_y;
	int bY = center_y + half_y;

	if(lX < 0)
	{
		num_cols += lX;
	}
	if(rX > 800)
	{
		num_cols -= rX - 800;
	}
	if(tY < 0)
	{
		num_rows += tY;
	}
	if(bY > 600)
	{
		num_rows -= bY - 600 ;
	}

	assert(num_rows > 0 && num_cols > 0);

	int prev_x = rX;

	for(size_t i = 0; i < num_rows; i ++)
	{
		for(size_t j = 0; j < num_cols; j ++)
		{
			create_material(material_type, &mat_state, arena, grid,  glm::vec2(rX, bY));
			rX --;
		}
		rX = prev_x;
		bY --;
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
	assert(window != NULL);

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

	PoolArena* arena = init_pool(800 * 600, sizeof(Material));
	Renderer render;
	render.initRenderData();


	const unsigned int UPS = 1;
	const float UPS_SLICE = 1.0f/UPS;
	const unsigned int max_updates = 10;
	const unsigned int MAX_FRAME_SKIPS = 10;
	unsigned int frames_skip = 0;

	double nowTime = glfwGetTime();
	double elapsed_time;
	double time_accumulator = 0.0f;

	std::vector<std::vector<Material*>> grid;
	grid.resize(600, std::vector<Material*>(800, nullptr));






	while(!glfwWindowShouldClose(window))
	{
		if(mousePressed)
		{
			spawn_materials((int)mouse_x, (int)mouse_y, 101, 101, mat_state, arena, grid);
		}
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		elapsed_time = glfwGetTime() - nowTime;
		nowTime = glfwGetTime();
		time_accumulator +=  elapsed_time;

		while(time_accumulator >= UPS_SLICE && frames_skip < MAX_FRAME_SKIPS)
		{
			frames_skip ++;
		}

		frames_skip = 0;
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

int main()
{
	game_loop();
}
