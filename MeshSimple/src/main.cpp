#include <base.h>
#include <Scene.h>
#include <Object.h>
#include <Model.h>
#include <Shader.h>
#include <Render.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <thread>


auto MainCamera = std::make_shared<Camera>(glm::vec3(0,0,1.5f));
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void processInput(GLFWwindow *window, float deltaTime);
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

int main() 
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(1920, 1080, "RTRender", nullptr, nullptr);
	if (window == nullptr) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();
	ImGui::StyleColorsDark();
	
	glEnable(GL_DEPTH_TEST);
	
	//glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);


	Material      ivory(1.0, Vec4f(0.6, 0.3, 0.1, 0.0), Vec3f(0.4, 0.4, 0.3), 50.);
	Material      glass(1.5, Vec4f(0.0, 0.5, 0.1, 0.8), Vec3f(0.6, 0.7, 0.8), 125.);
	Material red_rubber(1.0, Vec4f(0.9, 0.1, 0.0, 0.0), Vec3f(0.3, 0.1, 0.1), 10.);
	Material     mirror(1.0, Vec4f(0.0, 10.0, 0.8, 0.0), Vec3f(1.0, 1.0, 1.0), 1425.);

	/*
	Material mats[5] = { ivory, glass, mirror, red_rubber, ivory };

	std::shared_ptr<Object> cubes[4];
	for (int i = 0; i < 4; i++) {
		cubes[i] = std::make_shared<Object>();
		cubes[i]->model = std::make_shared<Model>("cube.obj");
		
		cubes[i]->material = mats[i];
		cubes[i]->shader = std::make_shared<Shader>("vertexshader.glsl", "fragshader.glsl");
		cubes[i]->render_init();
		//cubes[i]->set_world(glm::translate(glm::mat4(1), glm::vec3(-4 + i * 2, -4 + i * 3, -10)));
	}
	cubes[0]->set_world(glm::translate(glm::mat4(1), glm::vec3(-3, 0, -16)));
	cubes[1]->set_world(glm::translate(glm::mat4(1), glm::vec3(-1.0, -1.5, -12)));
	cubes[2]->set_world(glm::scale(glm::translate(glm::mat4(1), glm::vec3(0, 0, -30)), glm::vec3(10,10,1)));
	cubes[3]->set_world(glm::translate(glm::mat4(1), glm::vec3(4, 0, -5)));
	*/
	auto basic_shader = std::make_shared<Shader>("vertexshader.glsl", "fragshader.glsl");

	auto flamingo = std::make_shared<Object>();
	flamingo->model = std::make_shared<Model>("small.obj");
	flamingo->material = red_rubber;
	flamingo->shader = basic_shader;
	flamingo->render_init();
	flamingo->set_world(glm::translate(glm::mat4(1), glm::vec3(0,0.1,-1.5)));

	auto cube_model = std::make_shared<Model>("cube.obj");
	
	auto cube = std::make_shared<Object>();
	cube->model = cube_model;
	cube->material = ivory;
	cube->shader = basic_shader;
	cube->render_init();
	cube->set_world(glm::translate(glm::scale(glm::mat4(1), glm::vec3(0.7,0.05,0.7)), glm::vec3(0, -10, -2)));

	auto y = 0.32f;
	auto left_cube = std::make_shared<Object>();
	left_cube->model = cube_model;
	left_cube->material = glass;
	left_cube->shader = basic_shader;
	left_cube->render_init();
	left_cube->set_world(glm::translate(glm::scale(glm::mat4(1), glm::vec3(0.01, 0.5, 0.3)), glm::vec3(-50, y, -2)));

	auto right_cube = std::make_shared<Object>();
	right_cube->model = cube_model;
	right_cube->material = mirror;
	right_cube->shader = basic_shader;
	right_cube->render_init();
	right_cube->set_world(glm::translate(glm::scale(glm::mat4(1), glm::vec3(0.01, 0.5, 0.3)), glm::vec3(50, y, -2)));

	auto back_cube = std::make_shared<Object>();
	back_cube->model = cube_model;
	back_cube->material = mirror;
	back_cube->shader = basic_shader;
	back_cube->render_init();
	back_cube->set_world(glm::translate(glm::scale(glm::mat4(1), glm::vec3(0.7, 0.6, 0.05)), glm::vec3(0, y, -50)));

	std::shared_ptr<Scene> scene_ptr = std::make_shared<Scene>();
	scene_ptr->camera = MainCamera;
	scene_ptr->add_object(flamingo);
	scene_ptr->add_object(cube);
	scene_ptr->add_object(left_cube);
	scene_ptr->add_object(right_cube);
	scene_ptr->add_object(back_cube);
	/*
	for (auto cube : cubes) {
		cube->set_world(glm::rotate(cube->world, M_PI / 4, glm::vec3(0,1,0)));
		scene_ptr->add_object(cube);
	}
	*/

	scene_ptr->add_light(std::make_shared<Light>(Vec3f(0, 0, 0), 10));
	scene_ptr->add_light(std::make_shared<Light>(Vec3f(30, 50, -25), 1.8));
	//scene_ptr->add_light(std::make_shared<Light>(Vec3f(30, 20, 30), 1.7));

	Renderer raytracer;
	raytracer.set_scene(scene_ptr);

	float delTime = 0.0f, lastTime = 0.0f, lastHit = 0.0f;
	bool wireframe = true, gui_flag = true;
	float ratio = 1.0f, last_ratio = 1.0f;
	char save_buf[20] = "out.obj", open_buf[20] = "flamingo.obj";
	while (!glfwWindowShouldClose(window))
	{
		float curTime = glfwGetTime();
		delTime = curTime - lastTime;
		lastTime = curTime;
		
		glfwPollEvents();
		
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("RTRender", &gui_flag);
		ImGui::Text("Ray Trace Render Demo");
		
		ImGui::InputText("[Open] Filename", open_buf, IM_ARRAYSIZE(open_buf));
		if (ImGui::Button("Open")) {
			//object_ptr->model = std::make_shared<Model>(open_buf);
			//object_ptr->render_init();
		}
		
		if (ImGui::Button("Render")) {
			raytracer.render();
		}
		if (ImGui::Button("Close")) {
			glfwSetWindowShouldClose(window, true);
		}
		ImGui::Text("FPS: %f", 1.0f / delTime);
		//ImGui::Text("Faces: %d\n", object_ptr->model->indices.size() / 3);
		ImGui::End();

		ImGui::Render();
		int width, height;
		glfwMakeContextCurrent(window);
		glfwGetFramebufferSize(window, &width, &height);
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
		glViewport(0, 0, width, height);
		processInput(window, delTime);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
			float now = glfwGetTime();
			if (now - lastHit > 0.5) {
				wireframe ^= 1;
				lastHit = now;
			}
		}
		scene_ptr->render(wireframe);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		glfwMakeContextCurrent(window);
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (MainCamera->firstMouse)
	{
		MainCamera->lastX = xpos;
		MainCamera->lastY = ypos;
		MainCamera->firstMouse = false;
	}

	float xoffset = xpos - MainCamera->lastX;
	float yoffset = MainCamera->lastY - ypos; // reversed since y-coordinates go from bottom to top

	MainCamera->lastX = xpos;
	MainCamera->lastY = ypos;

	MainCamera->ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	MainCamera->ProcessMouseScroll(yoffset);
}

void processInput(GLFWwindow *window, float deltaTime)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		MainCamera->ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		MainCamera->ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		MainCamera->ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		MainCamera->ProcessKeyboard(RIGHT, deltaTime);
}