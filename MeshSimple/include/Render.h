#pragma once

#include <base.h>
#include <Scene.h>
#include <Material.h>
#include <geometry.h>

#include <algorithm>

struct Ray
{
	Vec3f ori, dir;
	int dep = 0;
	
	Ray(Vec3f ori, Vec3f dir, int dep = 0) :
		ori(ori), dir(dir), dep(dep) {}
};

struct HitInfo
{
	HitInfo() {
		hit = N = Vec3f();
		material = Material();
	};
	Vec3f hit, N;
	Material material;
};

static Vec3f reflect(const Vec3f &I, const Vec3f &N) {
	return I - N * 2.f*(I*N);
}

static Ray reflect(const Ray &ray, const HitInfo &info) {
	auto I = ray.dir, N = info.N;
	auto reflect_dir = I - N * 2.f*(I*N);
	auto reflect_ori = reflect_dir * N < 0 ? info.hit - N * 1e-3 : info.hit + N * 1e-3;
	return Ray(reflect_ori, reflect_dir, ray.dep + 1);
}

static Vec3f refract(const Vec3f &I, const Vec3f &N, const float eta_t, const float eta_i = 1.f) { // Snell's law
	float cosi = -std::max(-1.f, std::min(1.f, I*N));
	if (cosi < 0) return refract(I, -N, eta_i, eta_t); // if the ray comes from the inside the object, swap the air and the media
	float eta = eta_i / eta_t;
	float k = 1 - eta * eta*(1 - cosi * cosi);
	return k < 0 ? Vec3f(1, 0, 0) : I * eta + N * (eta*cosi - sqrtf(k)); // k<0 = total reflection, no ray to refract. I refract it anyways, this has no physical meaning
}

static Ray refract(const Ray &ray, const HitInfo &info) {
	auto I = ray.dir, N = info.N;
	auto eta_t = info.material.refractive_index;
	auto refract_dir = refract(I, N, eta_t);
	Vec3f refract_ori = refract_dir * N < 0 ? info.hit - N * 1e-3 : info.hit + N * 1e-3;
	return Ray(refract_ori, refract_dir, ray.dep + 1);
}

class Renderer
{
public:
	void render(const int width = 1024, const int height = 768, const float fov = M_PI / 3.f, const char* filepath = "tmp.jpg");
	bool scene_intersect(const Ray& ray, HitInfo& info);
	void set_scene(std::shared_ptr<Scene> _scene) {
		scene = _scene;
	}

private:
	Vec3f cast_ray(const Ray& ray);
private:
	std::shared_ptr<Scene> scene = nullptr;
};

