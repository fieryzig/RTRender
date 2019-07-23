#include "Render.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

void Renderer::render(const int width, const int height, const float fov, const char* filepath)
{
	std::vector<Vec3f> framebuffer(width*height);

#pragma omp parallel for
	for (size_t j = 0; j < height; j++) { // actual rendering loop
		for (size_t i = 0; i < width; i++) {
			float dir_x = (i + 0.5) - width / 2.;
			float dir_y = -(j + 0.5) + height / 2.;    // this flips the image at the same time
			float dir_z = -height / (2.*tan(fov / 2.));
			Ray ray(Vec3f(0, 0, 0), Vec3f(dir_x, dir_y, dir_z).normalize());
			framebuffer[i + j * width] = cast_ray(ray);
		}
	}

	std::vector<unsigned char> pixmap(width*height * 3);
	for (size_t i = 0; i < height*width; ++i) {
		Vec3f &c = framebuffer[i];
		float max = std::max(c[0], std::max(c[1], c[2]));
		if (max > 1) c = c * (1. / max);
		for (size_t j = 0; j < 3; j++) {
			pixmap[i * 3 + j] = (unsigned char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
		}
	}
	
	stbi_write_jpg(filepath, width, height, 3, pixmap.data(), 100);
}

bool Renderer::scene_intersect(const Ray & ray, HitInfo & info)
{
	for (auto obj : scene->objects) {
		if (obj->model->ray_model_intersect(ray.ori, ray.dir, info.hit, info.N, obj->world)) {
			info.material = obj->material;
			return true;
		}
	}
	return false;
}

Vec3f Renderer::cast_ray(const Ray & ray)
{
	HitInfo hit_info;

	if (ray.dep > 10 || !scene_intersect(ray, hit_info)) {
		return Vec3f(0.2, 0.7, 0.8);
	}

	auto reflect_ray = reflect(ray, hit_info);
	auto refract_ray = refract(ray, hit_info);
	auto reflect_color = cast_ray(reflect_ray);
	auto refract_color = cast_ray(refract_ray);

	auto hit = hit_info.hit;
	auto material = hit_info.material;
	auto N = hit_info.N;
	float diffuse_light_intensity = 0, specular_light_intensity = 0;
	
	auto lights = scene->lights;
	for (size_t i = 0; i < lights.size(); i++) {
		Vec3f light_dir = (lights[i]->position - hit).normalize();
		float light_distance = (lights[i]->position - hit).norm();

		Vec3f shadow_orig = light_dir * N < 0 ? hit - N * 1e-3 : hit + N * 1e-3;
		HitInfo shadow_hit;
		if (scene_intersect(Ray(shadow_orig, light_dir), shadow_hit) && (shadow_hit.hit - shadow_orig).norm() < light_distance)
			continue;

		diffuse_light_intensity += lights[i]->intensity * std::max(0.f, light_dir*N);
		specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, N)*ray.dir), material.specular_exponent)*lights[i]->intensity;
	}
	return material.diffuse_color * diffuse_light_intensity * material.albedo[0] + Vec3f(1., 1., 1.)*specular_light_intensity * material.albedo[1] + reflect_color * material.albedo[2] + refract_color * material.albedo[3];

}
