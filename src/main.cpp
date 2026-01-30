#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader.h>
#include <learnopengl/filesystem.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

const auto WINDOW_WIDTH = 1600.0f;
const auto WINDOW_HEIGHT = 900.0f;
glm::vec4 clearColor(0.05f, 0.05f, 0.05f, 1.00f);

// Timing
auto deltaTime = 0.0f;
auto lastFrame = 0.0f;

// Camera
Camera camera(glm::vec3(0.0f, 1.6f, 4.5f));
auto lastX = WINDOW_WIDTH / 2.0f;
auto lastY = WINDOW_HEIGHT / 2.0f;
auto firstMouse = true;
auto fov = 45.0f;

// Light setup
glm::vec3 lightColors[] = {
	{0.0f, 0.05f, 0.83f},
	{0.0f, 0.55f, 0.1f},
	{0.77f, 0.0f, 0.0f},
	{0.89f, 0.89f, 0.89f}
};
bool spotLightState = true;

// Debug menu
bool debugMenuOpen = false;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
bool isKeyPressed(int key);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
unsigned int loadTexture(const char* path);

int main()
{
	glfwInit();
#ifdef USE_GLES
	const char* glsl_version = "#version 300 es";
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);

	// Qualcomm stated that the Adreno GPU will support GLES 3.2,
	// but hinted that certain features of version 3.2 may not work.
	// All versions below 3.2 will be supported.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);

	// won't work
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	// use this instead (?)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	std::clog << "Target: OpenGL ES 3.0" << std::endl;
#else
	const char* glsl_version = "#version 330 core";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	std::clog << "Target: OpenGL 3.3 Desktop" << std::endl;
#endif

	float mainScale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
	GLFWwindow* window = glfwCreateWindow(int(WINDOW_WIDTH * mainScale), int(WINDOW_HEIGHT * mainScale), "OpenGL", NULL, NULL);

	if (window == NULL) {
		std::clog << "Failed to create GLFW window" << std::endl;
		const char* description;
		auto code = glfwGetError(&description);
		if (description) std::clog << "GLFW Error (" << code << "): " << description << std::endl;

		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

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
	stbi_set_flip_vertically_on_load(true);

	// initial cursor mode based on debugMenuOpen
	if (debugMenuOpen) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	else {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	glfwSetCursorPosCallback(window, mouse_callback);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup scaling
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(mainScale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
	style.FontScaleDpi = mainScale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

#ifdef USE_GLES
	Shader shader(FileSystem::getPath("resources/shaders/es/model_loading_shader.vs").c_str(), FileSystem::getPath("resources/shaders/es/model_loading_shader.fs").c_str());
#else
	Shader shader(FileSystem::getPath("resources/shaders/model_loading_shader.vs").c_str(), FileSystem::getPath("resources/shaders/model_loading_shader.fs").c_str());
#endif

	Model theModel(FileSystem::getPath("resources/models/backpack/backpack.obj"));

	while (!glfwWindowShouldClose(window)) {
		auto currentFrametime = glfwGetTime();
		deltaTime = float(currentFrametime) - lastFrame;
		lastFrame = float(currentFrametime);

		processInput(window);

		glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();

		// projection matrix
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(fov), WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
		auto view = camera.GetViewMatrix();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);

		auto model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

		shader.setMat4("model", model);
		theModel.Draw(shader);

		glfwPollEvents();

		if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
		{
			ImGui_ImplGlfw_Sleep(10);
			continue;
		}

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (debugMenuOpen) {
			ImGui::Begin("OpenGL Debug Menu");

			ImGui::ColorEdit3("Clear Color", &clearColor.x);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

			if (ImGui::Button("Quit")) {
				glfwSetWindowShouldClose(window, true);
			}

			ImGui::End();
		}
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
	// Toggle debug menu on key Escape press — edge-triggered to avoid toggling every frame while held
	static bool lastEscapeState = false;
	static bool lastSpotLightState = true;

	bool currentEscapeState = isKeyPressed(glfwGetKey(window, GLFW_KEY_ESCAPE));
	if (currentEscapeState && !lastEscapeState) {
		debugMenuOpen = !debugMenuOpen;

		if (debugMenuOpen) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		else {
			// Re-lock the cursor for camera control and reset firstMouse to avoid jump
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			firstMouse = true;
		}
	}
	lastEscapeState = currentEscapeState;

	// When debug menu is open, do not process camera movement or mouse look
	if (debugMenuOpen) {
		return;
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

	bool currentSpotLightState = isKeyPressed(glfwGetKey(window, GLFW_KEY_F));
	if (currentSpotLightState && !lastSpotLightState) {
		spotLightState = !spotLightState;
	}
	lastSpotLightState = currentSpotLightState;
}

bool isKeyPressed(int key) {
	return key == GLFW_PRESS;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	// Ignore mouse look while debug menu is open (so user can move the cursor freely)
	if (debugMenuOpen) {
		return;
	}

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
