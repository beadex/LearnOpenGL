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
#include <learnopengl/material.h>

#include <iostream>

const auto WINDOW_WIDTH = 1600.0f;
const auto WINDOW_HEIGHT = 900.0f;
glm::vec4 clearColor(0.5f, 0.5f, 0.5f, 1.00f);

// Timing
auto deltaTime = 0.0f;
auto lastFrame = 0.0f;

// Camera
Camera camera(glm::vec3(0.0f, 1.6f, 4.5f));
auto lastX = WINDOW_WIDTH / 2.0f;
auto lastY = WINDOW_HEIGHT / 2.0f;
auto firstMouse = true;
auto fov = 45.0f;

// Light cube
Light lightCube = {
	{1.2f, 1.0f, 2.0f},
	{0.2f, 0.2f, 0.2f},
	{0.5f, 0.5f, 0.5f},
	{1.0f, 1.0f, 1.0f},
};
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

// Debug menu
bool debugMenuOpen = false;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
bool isKeyPressed(int key);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void apply_material(Shader& shader, const Material& mat);
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
	Shader objectShader(FileSystem::getPath("resources/shaders/es/object_shader.vs").c_str(), FileSystem::getPath("resources/shaders/es/object_shader.fs").c_str());
	Shader lightCubeShader(FileSystem::getPath("resources/shaders/es/light_cube_shader.vs").c_str(), FileSystem::getPath("resources/shaders/es/light_cube_shader.fs").c_str());
#else
	Shader objectShader(FileSystem::getPath("resources/shaders/object_shader.vs").c_str(), FileSystem::getPath("resources/shaders/object_shader.fs").c_str());
	Shader lightCubeShader(FileSystem::getPath("resources/shaders/light_cube_shader.vs").c_str(), FileSystem::getPath("resources/shaders/light_cube_shader.fs").c_str());
#endif

	unsigned int diffuseMap = loadTexture(FileSystem::getPath("resources/textures/container2.png").c_str());
	unsigned int specularMap = loadTexture(FileSystem::getPath("resources/textures/container2_specular.png").c_str());

	float vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};

	unsigned int VBO, objectVAO;
	glGenVertexArrays(1, &objectVAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(objectVAO);

	// object vertex attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// normal vertex attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// tex coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	unsigned int lightCubeVAO;
	glGenVertexArrays(1, &lightCubeVAO);
	glBindVertexArray(lightCubeVAO);

	// light vertex attribute
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	while (!glfwWindowShouldClose(window)) {
		auto currentFrametime = glfwGetTime();
		deltaTime = float(currentFrametime) - lastFrame;
		lastFrame = float(currentFrametime);

		processInput(window);

		glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		objectShader.use();

		// material properties
		objectShader.setInt("material.diffuse", 0);
		objectShader.setInt("material.specular", 1);
		objectShader.setVec3("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
		objectShader.setFloat("material.shininess", 64.0f);

		// light props
		objectShader.setVec3("light.position", lightCube.position);
		const auto diffuseColor = lightColor * lightCube.diffuse;
		objectShader.setVec3("light.diffuse", diffuseColor);
		objectShader.setVec3("light.ambient", diffuseColor * lightCube.ambient);
		objectShader.setVec3("light.specular", lightCube.specular);

		// camera props
		objectShader.setVec3("viewPos", camera.Position);

		// projection matrix
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(fov), WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
		auto view = camera.GetViewMatrix();
		objectShader.setMat4("projection", projection);
		objectShader.setMat4("view", view);

		auto model = glm::mat4(1.0f);
		objectShader.setMat4("model", model);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularMap);

		glBindVertexArray(objectVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		lightCubeShader.use();
		lightCubeShader.setMat4("projection", projection);
		lightCubeShader.setMat4("view", view);
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightCube.position);
		model = glm::scale(model, glm::vec3(0.2f));
		lightCubeShader.setMat4("model", model);

		glBindVertexArray(lightCubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

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

			ImGui::ColorEdit3("Light Color", &lightColor.x);
			ImGui::DragFloat3("Light Cube Position", &lightCube.position.x, 0.01f);

			ImGui::SameLine();

			if (ImGui::Button("Reset")) {
				lightCube.position = glm::vec3(1.2f, 1.0f, 2.0f);
			}

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


	glDeleteVertexArrays(1, &objectVAO);
	glDeleteVertexArrays(1, &lightCubeVAO);
	glDeleteBuffers(1, &VBO);

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

void apply_material(Shader &shader, const Material &mat) {
	shader.use();

	// Using your requested function format
	shader.setVec3("material.ambient", glm::vec3(mat.ambient.x, mat.ambient.y, mat.ambient.z));
	shader.setVec3("material.diffuse", glm::vec3(mat.diffuse.x, mat.diffuse.y, mat.diffuse.z));
	shader.setVec3("material.specular", glm::vec3(mat.specular.x, mat.specular.y, mat.specular.z));
	shader.setFloat("material.shininess", mat.shininess);
}

unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}