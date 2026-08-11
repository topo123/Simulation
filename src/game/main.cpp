#include <Shader.hpp>
#include <cassert>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Material.hpp>
#include <PoolArena.hpp>
#include <InputHandler.hpp>

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
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);		
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
	glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
	glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_FALSE);


#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	*window = glfwCreateWindow(800, 600, "Simulation", NULL, NULL);
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

	glfwSetFramebufferSizeCallback(*window, framebuffer_size_callback);
	glfwSetWindowSizeCallback(*window, window_size_callback);
	glfwSetMouseButtonCallback(*window, mouse_button_callback);
	glfwSetCursorPosCallback(*window, cursor_pos_callback);
	glfwSetKeyCallback(*window, key_callback);
	
	glfwSetWindowSize(*window, 800, 600);

	return 0;
}

void game_loop(std::string name)
{
	GLFWwindow* window = nullptr;
	int success = init_window(&window);

	assert(success == 0);

	Arena app_info_arena;
	AppInfo* app_info = init_sim_info(window, &app_info_arena);  

	PoolArena* arena = init_pool(800 * 600 + 1, sizeof(Material));
	World world;
	world.init_world(10, 10, 800, 600, arena);
	
	if(name != "")
	{
		world.load_world(name);
	} 

	const unsigned int UPS = 120;
	const unsigned int FPS = 60;
	const float FPS_SLICE = 1.0f/FPS;
	const float UPS_SLICE = 1.0f/UPS;
	const unsigned int MAX_FRAME_SKIPS = 5;
	unsigned int frames_skip = 0;

	double elapsed_time;
	double time_accumulator = 0.0f;
	int updates = 0;

	int frames = 0;
	double frame_accumulator = 0;

	double nowTime = glfwGetTime();
	double debug_time = 0;
	double debug_frame = 0;
	double debug_update = 0;

	while(!glfwWindowShouldClose(window))
	{
		process_input(app_info, world);

		elapsed_time = glfwGetTime() - nowTime;

		debug_time += elapsed_time;
		time_accumulator +=  elapsed_time;
		frame_accumulator += elapsed_time;

		nowTime = glfwGetTime();

		while(time_accumulator >= UPS_SLICE && frames_skip < MAX_FRAME_SKIPS)
		{
			world.update_world(UPS_SLICE, app_info->sim_state);
			time_accumulator -= UPS_SLICE;
			debug_update += UPS_SLICE;
			frames_skip ++;
		}

		if(app_info->draw_state == DEBUG_DRAW)
		{
			world.draw_world(true);
		}
		else {
			world.draw_world(false);
		}

		frame_accumulator -= FPS_SLICE;
		debug_frame += FPS_SLICE;

		if(debug_time >= 1.0)
		{
			long frames = ((debug_frame * 60.0f)/debug_time);
			long updates = ((debug_update * 120.0f)/debug_time);
			std::cout << "FPS: " << frames << " | UPS: " << updates << '\n';
			debug_update = 0;
			debug_frame = 0;
			debug_time = 0;
		}

		frames_skip = 0;
		glfwSwapBuffers(window);
	}

}

int main(int argc, char* argv[])
{
	if(argc > 1)
	{
		game_loop(std::string(argv[1]));
	}
	else{

		game_loop("");
	}
	glfwTerminate();
	return 0;
}
