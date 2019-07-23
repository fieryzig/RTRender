#pragma once

#include <geometry.h>
#include <Object.h>
#include <vector>
#include <Camera.h>

struct Light {
	Light(const Vec3f &p, const float i) : position(p), intensity(i) {}
	Vec3f position;
	float intensity;
};

class Scene
{
public:
	void render(bool wireframe = false) {
		for (auto obj : objects) {
			obj->render(wireframe, camera);
		}
	}

	void add_object(std::shared_ptr<Object> obj_ptr) {
		objects.push_back(obj_ptr);
	}

	void add_light(std::shared_ptr<Light> light_ptr) {
		lights.push_back(light_ptr);
	}

	std::vector<std::shared_ptr<Object>> objects;
	std::vector<std::shared_ptr<Light>> lights;
	std::shared_ptr<Camera> camera;
};