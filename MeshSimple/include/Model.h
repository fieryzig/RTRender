#ifndef __MODEL_H__
#define __MODEL_H__
#include <vector>
#include <string>
#include "geometry.h"
#include "base.h"

class Model {
private:
	std::vector<Vec3f> verts;
	std::vector<Vec3i> faces;

public:
	std::vector<GLfloat> vertices, ori_vertices;
	std::vector<unsigned int> indices, ori_indicies;

public:
	Model(const char *filename);
	void load_from_obj(const char* filename);

	int nverts() const;                          // number of vertices
	int nfaces() const;                          // number of triangles

	bool ray_triangle_intersect(const Vec3f& a, const Vec3f& b, const Vec3f& c, const Vec3f &orig, const Vec3f &dir, float &tnear, Vec3f &N);
	bool ray_model_intersect(const Vec3f &orig, const Vec3f &dir, Vec3f &hit, Vec3f &N, const glm::mat4& world);

	const Vec3f &point(int i) const;                   // coordinates of the vertex i
	Vec3f &point(int i);                   // coordinates of the vertex i
	int vert(int fi, int li) const;              // index of the vertex for the triangle fi and local index li
	void get_bbox(Vec3f &min, Vec3f &max); // bounding box for all the vertices, including isolated ones
};

std::ostream& operator<<(std::ostream& out, Model &m);

#endif //__MODEL_H__
/******************************************************
Old Version
*******************************************************

#pragma once

#include <base.h>
#include <geometry.h>

#include <vector>
#include <map>
#include <set>
#include <queue>
#include <algorithm>

struct EdgePair
{
	int a, b;
	double err;
	glm::vec4 point;

	bool operator < (const EdgePair &rhs) const
	{
		return err > rhs.err;
	}
};

struct Face
{
	int a, b, c;
	Face(int _a, int _b, int _c)
	{
		std::vector<int> v{ _a,_b,_c };
		std::sort(v.begin(), v.end());
		a = v[0]; b = v[1]; c = v[2];
	}
	bool operator < (const Face &rhs) const
	{
		if (a != rhs.a) return a < rhs.a;
		if (b != rhs.b) return b < rhs.b;
		return c < rhs.c;
	}
};


class Model
{
public:
	Model(const char* filename) {
		load_from_obj(filename);
	}
	void load_from_obj(const char* filename);
	void calulateQ(int index);
	void simplify(float ratio);
	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;
	void save(const char* filename);

	bool ray_triangle_intersect(const int &fi, const Vec3f &orig, const Vec3f &dir, float &tnear);

private:
	using pii = std::pair<int, int>;
	using sp = std::set<pii>;
	std::vector<sp> G;
	std::vector<std::set<int>> E;
	std::map<int, glm::mat4> Q;
	std::vector<GLfloat> ori_vertices;
	std::vector<unsigned int> ori_indicies;
	std::priority_queue<EdgePair, std::vector<EdgePair>, std::less<EdgePair>> heap;
	std::set<int> Exist;
	void makeG(std::vector<unsigned int> indices);
	void makeE(std::vector<unsigned int> indices);
	void updateHeap(int index);

	void getPoint(int index, glm::vec3 &point)
	{
		point = glm::vec3(vertices[index*3], vertices[index*3+1], vertices[index*3+2]);
	}
};

*/