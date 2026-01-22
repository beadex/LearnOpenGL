#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader.h>
#include <learnopengl/filesystem.h>
#include <learnopengl/camera.h>

#include <iostream>

const auto WINDOW_WIDTH = 1600.0f;
const auto WINDOW_HEIGHT = 900.0f;

// Timing
auto deltaTime = 0.0f;
auto lastFrame = 0.0f;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
auto lastX = WINDOW_WIDTH / 2.0f;
auto lastY = WINDOW_HEIGHT / 2.0f;
auto firstMouse = true;
auto fov = 45.0f;

// Light cube
glm::vec3 lightCubePos(1.2f, 1.0f, 2.0f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
bool isKeyPressed(int key);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

int main()
{
	glfwInit();
#ifdef USE_GLES
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);

	// Qualcomm stated that the Adreno GPU will support GLES 3.2,
	// but hinted that certain features of version 3.2 may not work.
	// All versions below 3.2 will be supported.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);

	// won't work
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	// use this instead (?)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	std::clog << "Target: OpenGL ES 3.1" << std::endl;
#else
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	std::clog << "Target: OpenGL 3.3 Desktop" << std::endl;
#endif

	GLFWwindow* window = glfwCreateWindow(int(WINDOW_WIDTH), int(WINDOW_HEIGHT), "LearnOpenGL", NULL, NULL);

	if (window == NULL) {
		std::clog << "Failed to create GLFW window" << std::endl;
		const char* description;
		auto code = glfwGetError(&description);
		if (description) std::clog << "GLFW Error (" << code << "): " << description << std::endl;

		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

#ifdef USE_GLES
	if (!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress)) {
		std::clog << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
#else
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::clog << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
#endif

	glEnable(GL_DEPTH_TEST);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);

#ifdef USE_GLES
	Shader objectShader(FileSystem::getPath("resources/shaders/es/object_shader.vs").c_str(), FileSystem::getPath("resources/shaders/es/object_shader.fs").c_str());
	Shader lightCubeShader(FileSystem::getPath("resources/shaders/es/light_cube_shader.vs").c_str(), FileSystem::getPath("resources/shaders/es/light_cube_shader.fs").c_str());
#else
	Shader objectShader(FileSystem::getPath("resources/shaders/object_shader.vs").c_str(), FileSystem::getPath("resources/shaders/object_shader.fs").c_str());
	Shader lightCubeShader(FileSystem::getPath("resources/shaders/light_cube_shader.vs").c_str(), FileSystem::getPath("resources/shaders/light_cube_shader.fs").c_str());
#endif

	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};

	unsigned int VBO, objectVAO;
	glGenVertexArrays(1, &objectVAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(objectVAO);

	// object vertex attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal vertex attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	unsigned int lightCubeVAO;
	glGenVertexArrays(1, &lightCubeVAO);
	glBindVertexArray(lightCubeVAO);

	// light vertex attribute
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	while (!glfwWindowShouldClose(window)) {
		auto currentFrametime = glfwGetTime();
		deltaTime = float(currentFrametime) - lastFrame;
		lastFrame = float(currentFrametime);

		processInput(window);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float radius = 2.0f;
		float speed = 0.5f; // Adjust this to make it faster or slower
		float phaseTime = currentFrametime * speed;
		
		lightCubePos.x = cos(phaseTime) * radius;
		lightCubePos.z = sin(phaseTime) * radius;

		objectShader.use();
		objectShader.setVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));
		objectShader.setVec3("lightPos", lightCubePos);
		objectShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
		objectShader.setVec3("viewPos", camera.Position);

		// projection matrix
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(fov), WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
		auto view = camera.GetViewMatrix();
		objectShader.setMat4("projection", projection);
		objectShader.setMat4("view", view);

		auto model = glm::mat4(1.0f);
		objectShader.setMat4("model", model);

		glBindVertexArray(objectVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		lightCubeShader.use();
		lightCubeShader.setMat4("projection", projection);
		lightCubeShader.setMat4("view", view);
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightCubePos);
		model = glm::scale(model, glm::vec3(0.2f));
		lightCubeShader.setMat4("model", model);

		glBindVertexArray(lightCubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &objectVAO);
	glDeleteVertexArrays(1, &lightCubeVAO);
	glDeleteBuffers(1, &VBO);

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
	if (isKeyPressed(glfwGetKey(window, GLFW_KEY_ESCAPE))) {
		glfwSetWindowShouldClose(window, true);
	}

	if (isKeyPressed(glfwGetKey(window, GLFW_KEY_W))) {
		camera.MoveCamera(FORWARD, deltaTime);
	}

	if (isKeyPressed(glfwGetKey(window, GLFW_KEY_S))) {
		camera.MoveCamera(BACKWARD, deltaTime);
	}

	if (isKeyPressed(glfwGetKey(window, GLFW_KEY_A))) {
		camera.MoveCamera(LEFT, deltaTime);
	}

	if (isKeyPressed(glfwGetKey(window, GLFW_KEY_D))) {
		camera.MoveCamera(RIGHT, deltaTime);
	}
}

bool isKeyPressed(int key) {
	return key == GLFW_PRESS;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	auto xpos = float(xposIn);
	auto ypos = float(yposIn);
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	auto xoffset = xpos - lastX;
	auto yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.CalculateCameraDirection(xoffset, yoffset);
}