#include <stdio.h>
#include <thread>
#include <iostream>

#include <glad/glad.h>
#include <glfw/glfw3.h>

#include "agent.h"
#include "keyboard_agent.h"
#include "dumb_agent.h"
#include "ai_agent.h"
#include "game.h"
#include "config.h"

static Game game;
static AIAgent agent;
static bool window_minimized = false;

static void glfw_error_callback(int error, const char* description);
static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void glfw_window_iconify_callback(GLFWwindow* window, int iconified);
static void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height);


int main() {
	// One of the few functions that can be called before GLFW initialization.
	glfwSetErrorCallback(glfw_error_callback);

	if (!glfwInit()) {
		std::cerr << "ERROR: GLFW failed to initialize!" << std::endl;
		return -1;
	}

	// Set the minimum required OpenGL version to 3.3 core.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a window.
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Snake", FULLSCREEN ? glfwGetPrimaryMonitor() : NULL, NULL);

	if (window) {		
		// Set key_callback to the current window to process key events.
		glfwSetKeyCallback(window, glfw_key_callback);

		// Set framebuffer size callback. 
		glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);

		// Set window iconify callback.
		glfwSetWindowIconifyCallback(window, glfw_window_iconify_callback);

		// Before using any OpenGL API, we must have a current OpenGL context.
		glfwMakeContextCurrent(window);

		// No v-sync.
		glfwSwapInterval(0);

		// Initialize GLAD after setting current context as it needs a current context to load from. 
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			std::cerr << "ERROR: failed to initialize GLAD." << std::endl;
		}

		game.init(WIDTH, HEIGHT, TILE_WIDTH);

		double dt = 0.0;
		double last_frame_time = 0.0;
		constexpr double MAX_FRAMETIME = 1.0 / FPS;

		while (!glfwWindowShouldClose(window)) {
			// Calculate delta time.
			double begin_frame_time = glfwGetTime();
			dt = begin_frame_time - last_frame_time;
			last_frame_time = begin_frame_time;

			// Poll user events.
			glfwPollEvents();

			// Only update game when window is active.
			if (!window_minimized) {
				// Input.
				game.process_input(agent.getInput());
			
				// Update.
				agent.update(game.update(dt));

				// Render.
				game.render();
			}

			// Swap buffers.
			glfwSwapBuffers(window);

			// Sleep to save cpu cycles.
			double time_elapsed_for_frame = glfwGetTime() - begin_frame_time;

			if (SYNC && time_elapsed_for_frame < MAX_FRAMETIME) {
				double ms = (MAX_FRAMETIME - time_elapsed_for_frame) * 1000.0;

				if (ms > 0) {
					std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<unsigned long>(ms)));
					//std::cout << ms << " ms/sleep. " << time_elapsed_for_frame * 1000 << " elapsed_time/frame. " << dt * 1000 << " dt." << std::endl;
				}
			}
		}
	}

	game.terminate();

	glfwDestroyWindow(window);

	glfwTerminate();

	return 0;
}

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS) {
			agent.keyPress(key);
		}
		else if (action == GLFW_RELEASE) {
			agent.keyRelease(key);
		}
	}
}

static void glfw_error_callback(int error, const char* description) {
	std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

static void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	game.update_dimensions(width, height);
}

static void glfw_window_iconify_callback(GLFWwindow* window, int iconified) {
	window_minimized = iconified;
}