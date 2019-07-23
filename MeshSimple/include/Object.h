#pragma once

#include <Shader.h>
#include <Model.h>
#include <Camera.h>
#include <Material.h>

class Object
{
public:
	void render(bool, std::shared_ptr<Camera>);
	void set_world(const glm::mat4& mat) {
		world = mat;
	}

	std::shared_ptr<Model> model;
	std::shared_ptr<Shader> shader;
	
	Material material;

	GLuint VAO, VBO, EBO;

	void render_init();
	void update_mesh();

	glm::mat4 world = glm::mat4(1);
};