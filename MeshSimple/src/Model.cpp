#include <iostream>
#include <cassert>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "model.h"

// fills verts and faces arrays, supposes .obj file to have "f " entries without slashes
Model::Model(const char *filename) : verts(), faces() {
	load_from_obj(filename);
	std::cerr << "# v# " << verts.size() << " f# " << faces.size() << std::endl;
	
	Vec3f min, max;
	get_bbox(min, max);
}

void Model::load_from_obj(const char* filename)
{
	FILE* fp = fopen(filename, "r");
	if (!fp) {
		std::cout << "Loading failed." << std::endl;
	}
	else {
		char buf[256];
		int nVertices = 0, nTriangles = 0;
		int nLines = 0;
		while (fscanf(fp, "%s", buf) != EOF) {
			nLines++;
			switch (buf[0]) {
			case '#':
				fgets(buf, sizeof(buf), fp);
				break;
			case 'v':
				switch (buf[1]) {
				case '\0':
				{
					double x, y, z;
					if (fscanf(fp, "%lf %lf %lf", &x, &y, &z) == 3) {
						vertices.push_back(x);
						vertices.push_back(y);
						vertices.push_back(z);
						verts.push_back(Vec3f(x, y, z));
					}
					else {
						std::cout << "Wrong number of values." << std::endl;
					}
				}
				break;
				default:
					fgets(buf, sizeof(buf), fp);
					break;
				}
				break;
			case 'f':
			{
				GLuint v, n, t;
				GLuint a, b, c;
				if (fscanf(fp, "%s", buf) != 1) {
					std::cout << "Wrong Face!" << std::endl;
				}
				if (strstr(buf, "//")) {
					if (sscanf(buf, "%d//%d", &a, &n) == 2 &&
						fscanf(fp, "%d//%d", &b, &n) == 2 &&
						fscanf(fp, "%d//%d", &c, &n) == 2) {
						nTriangles++;
						indices.push_back(a - 1);
						indices.push_back(b - 1);
						indices.push_back(c - 1);
						faces.push_back(Vec3i(a - 1, b - 1, c - 1));
					}
					else {
						std::cout << "Wrong!" << std::endl;
					}
				}
				else {
					if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
						a = v;
						if (fscanf(fp, "%d/%d/%d", &b, &t, &n) == 3 &&
							fscanf(fp, "%d/%d/%d", &c, &t, &n) == 3)
						{
							nTriangles++;
							indices.push_back(a - 1);
							indices.push_back(b - 1);
							indices.push_back(c - 1);
							faces.push_back(Vec3i(a - 1, b - 1, c - 1));
						}
						else {
							std::cout << "wrong face~" << std::endl;
						}
					}
					else {
						if (sscanf(buf, "%d", &a) == 1 &&
							fscanf(fp, "%d", &b) == 1 &&
							fscanf(fp, "%d", &c) == 1) {
							nTriangles++;
							indices.push_back(a - 1);
							indices.push_back(b - 1);
							indices.push_back(c - 1);
							faces.push_back(Vec3i(a - 1, b - 1, c - 1));
						}
						else {
							std::cout << "wrong face~~" << std::endl;
						}
					}
				}
			}
			break;
			default:
				fgets(buf, sizeof(buf), fp);
				break;
			}
		}
	}
	fclose(fp);
	ori_indicies = indices;
	ori_vertices = vertices;
}

// Moller and Trumbore
bool Model::ray_triangle_intersect(const Vec3f& a, const Vec3f& b, const Vec3f& c, const Vec3f &orig, const Vec3f &dir, float &tnear, Vec3f &N) {
	Vec3f edge1 = b - a;
	Vec3f edge2 = c - a;
	Vec3f pvec = cross(dir, edge2);
	float det = edge1 * pvec;
	if (det < 1e-5) return false;

	Vec3f tvec = orig - a;
	float u = tvec * pvec;
	if (u < 0 || u > det) return false;

	Vec3f qvec = cross(tvec, edge1);
	float v = dir * qvec;
	if (v < 0 || u + v > det) return false;

	tnear = edge2 * qvec * (1. / det);
	if (tnear > 1e-5) {
		N = cross(edge1, edge2);
		N = N * (1.f / N.norm());
		return true;
	}
	return false;
}

bool Model::ray_model_intersect(const Vec3f &orig, const Vec3f &dir, Vec3f &hit, Vec3f &N, const glm::mat4& world) {
	float tnear;
	Vec3f N_tmp;
	for (int i = 0; i < faces.size(); i++) {
		auto a = point(vert(i, 0)), b = point(vert(i, 1)), c = point(vert(i, 2));
		a = world * a; b = world * b; c = world * c;
		bool flag = ray_triangle_intersect(a, b, c, orig, dir, tnear, N_tmp);
		if (flag) {
			N = N_tmp;
			hit = orig + dir * tnear;
			return true;
		}
	}
	return false;
}


int Model::nverts() const {
	return (int)verts.size();
}

int Model::nfaces() const {
	return (int)faces.size();
}

void Model::get_bbox(Vec3f &min, Vec3f &max) {
	min = max = verts[0];
	for (int i = 1; i < (int)verts.size(); ++i) {
		for (int j = 0; j < 3; j++) {
			min[j] = std::min(min[j], verts[i][j]);
			max[j] = std::max(max[j], verts[i][j]);
		}
	}
	std::cerr << "bbox: [" << min << " : " << max << "]" << std::endl;
}

const Vec3f &Model::point(int i) const {
	assert(i >= 0 && i < nverts());
	return verts[i];
}

Vec3f &Model::point(int i) {
	assert(i >= 0 && i < nverts());
	return verts[i];
}

int Model::vert(int fi, int li) const {
	assert(fi >= 0 && fi < nfaces() && li >= 0 && li < 3);
	return faces[fi][li];
}

std::ostream& operator<<(std::ostream& out, Model &m) {
	for (int i = 0; i < m.nverts(); i++) {
		out << "v " << m.point(i) << std::endl;
	}
	for (int i = 0; i < m.nfaces(); i++) {
		out << "f ";
		for (int k = 0; k < 3; k++) {
			out << (m.vert(i, k) + 1) << " ";
		}
		out << std::endl;
	}
	return out;
}

/******************************************************
Old Version
*******************************************************

#include <Model.h>

// Moller and Trumbore
bool Model::ray_triangle_intersect(const int &fi, const Vec3f &orig, const Vec3f &dir, float &tnear) {
	Vec3f edge1 = point(vert(fi, 1)) - point(vert(fi, 0));
	Vec3f edge2 = point(vert(fi, 2)) - point(vert(fi, 0));
	Vec3f pvec = cross(dir, edge2);
	float det = edge1 * pvec;
	if (det < 1e-5) return false;

	Vec3f tvec = orig - point(vert(fi, 0));
	float u = tvec * pvec;
	if (u < 0 || u > det) return false;

	Vec3f qvec = cross(tvec, edge1);
	float v = dir * qvec;
	if (v < 0 || u + v > det) return false;

	tnear = edge2 * qvec * (1. / det);
	return tnear > 1e-5;
}

void Model::load_from_obj(const char* filename)
{
	FILE* fp = fopen(filename, "r");
	if (!fp) {
		std::cout << "Loading failed." << std::endl;
	}
	else {
		char buf[256];
		int nVertices = 0, nTriangles = 0;
		int nLines = 0;
		while (fscanf(fp, "%s", buf) != EOF) {
			nLines++;
			switch (buf[0]) {
			case '#':
				fgets(buf, sizeof(buf), fp);
				break;
			case 'v':
				switch (buf[1]) {
				case '\0':
				{
					double x, y, z;
					if (fscanf(fp, "%lf %lf %lf", &x, &y, &z) == 3) {
						vertices.push_back(x);
						vertices.push_back(y);
						vertices.push_back(z);
					}
					else {
						std::cout << "Wrong number of values." << std::endl;
					}
				}
				break;
				default:
					fgets(buf, sizeof(buf), fp);
					break;
				}
				break;
			case 'f':
			{
				GLuint v, n, t;
				GLuint a, b, c;
				if (fscanf(fp, "%s", buf) != 1) {
					std::cout << "Wrong Face!" << std::endl;
				}
				if (strstr(buf, "//")) {
					if (sscanf(buf, "%d//%d", &a, &n) == 2 &&
						fscanf(fp, "%d//%d", &b, &n) == 2 &&
						fscanf(fp, "%d//%d", &c, &n) == 2) {
						nTriangles++;
						indices.push_back(a - 1);
						indices.push_back(b - 1);
						indices.push_back(c - 1);
					}
					else {
						std::cout << "Wrong!" << std::endl;
					}
				}
				else {
					if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
						a = v;
						if (fscanf(fp, "%d/%d/%d", &b, &t, &n) == 3 &&
							fscanf(fp, "%d/%d/%d", &c, &t, &n) == 3)
						{
							nTriangles++;
							indices.push_back(a - 1);
							indices.push_back(b - 1);
							indices.push_back(c - 1);
						}
						else {
							std::cout << "wrong face~" << std::endl;
						}
					}
					else {
						if (sscanf(buf, "%d", &a) == 1 &&
							fscanf(fp, "%d", &b) == 1 &&
							fscanf(fp, "%d", &c) == 1) {
							nTriangles++;
							indices.push_back(a - 1);
							indices.push_back(b - 1);
							indices.push_back(c - 1);
						}
						else {
							std::cout << "wrong face~~" << std::endl;
						}
					}
				}
			}
			break;
			default:
				fgets(buf, sizeof(buf), fp);
				break;
			}
		}
	}
	fclose(fp);
	ori_indicies = indices;
	ori_vertices = vertices;
}

void Model::calulateQ(int index)
{
	glm::mat4 Qmat(0);
	glm::vec3 A, B, C;
	getPoint(index, A);
	for (pii tri : G[index]) {
		getPoint(tri.first, B);
		getPoint(tri.second, C);
		glm::vec3 N = glm::cross(A - B, A - C);
		N = glm::normalize(N);
		float d = -glm::dot(N, A);
		glm::vec4 tmp(N, d);
		//Qmat = Qmat + glm::mat4(N.x*N.x, N.x*N.y, N.x*N.z, N.x*d,
		//	N.y*N.x, N.y*N.y, N.y*N.z, N.y*d,
		//	N.z*N.x, N.z*N.y, N.z*N.z, N.z*d,
		//	d*N.x, d*N.y, d*N.z, d*d);
		Qmat = Qmat + glm::outerProduct(tmp,tmp);
	}
	Q[index] = Qmat;
}

void Model::updateHeap(int index)
{
	for (std::set<int>::iterator it = E[index].begin(); it != E[index].end(); it++) {
		int j = *it;
		glm::mat4 Q_bar = Q[index] + Q[j];
		glm::vec3 a, b;
		getPoint(index, a);
		getPoint(j, b);
		glm::vec4 A(a, 1.0f), B(b, 1.0f);
		glm::vec4 C = (A + B) * 0.5f;
		// point[i], point[j], (point[i]+point[j]) / 2
		// 1. point[i]
		float err;
		err = glm::dot(A,(Q_bar * A));
		EdgePair ep1 = EdgePair{ index,j,err,A };
		err = glm::dot(B,(Q_bar * B));
		EdgePair ep2 = EdgePair{ index,j,err,B };
		err = glm::dot(C,(Q_bar * C));
		EdgePair ep3 = EdgePair{ index,j,err,C };
		EdgePair ep = std::min(ep1, ep2);
		ep = std::min(ep, ep3);
		heap.push(ep);
	}
}

void Model::simplify(float ratio)
{
	vertices = ori_vertices;
	indices = ori_indicies;
	makeG(indices);
	makeE(indices);
	Exist.clear();
	for (auto it = indices.begin(); it != indices.end(); it++) Exist.insert(*it);
	for (int i = 0; i < vertices.size() / 3; i++)
	{
		if (Exist.find(i) == Exist.end()) continue;
		calulateQ(i);
	}
	while (!heap.empty()) heap.pop();
	for (int i = 0; i < vertices.size() / 3; i++) {
		if (Exist.find(i) == Exist.end()) continue;
		updateHeap(i);
	}

	int tot = ratio * indices.size() / 6;
	int _tot = tot;
	while (tot && !heap.empty()) {
		EdgePair ep = heap.top();
		heap.pop();
		if (Exist.find(ep.a) != Exist.end() && Exist.find(ep.b) != Exist.end()) {
			tot -= 1;
			Exist.erase(ep.a); Exist.erase(ep.b);
			auto p = ep.point;
			vertices.push_back(p.x); vertices.push_back(p.y); vertices.push_back(p.z);
			int ind = vertices.size() / 3 - 1;
			Exist.insert(ind);
			// union edge
			std::set<int> unionset;
			for (auto it = E[ep.a].begin(); it != E[ep.a].end(); it++) {
				if (Exist.find(*it) == Exist.end()) continue;
				E[*it].erase(ep.a);
				E[*it].insert(ind);
				unionset.insert(*it);
			}
			for (auto it = E[ep.b].begin(); it != E[ep.b].end(); it++) {
				if (Exist.find(*it) == Exist.end()) continue;
				E[*it].erase(ep.b);
				E[*it].insert(ind);
				unionset.insert(*it);
			}
			E.push_back(unionset);
			E[ep.a].clear();
			E[ep.b].clear();
			// union G
			sp unionG;
			for (auto it = G[ep.a].begin(); it != G[ep.a].end(); it++) {
				if (Exist.find(it->first) == Exist.end()) continue;
				if (Exist.find(it->second) == Exist.end()) continue;
				unionG.insert(*it);
				if (G[it->first].find(std::make_pair(ep.a, it->second)) != G[it->first].end()) {
					G[it->first].erase(std::make_pair(ep.a, it->second));
					G[it->first].insert(std::make_pair(ind, it->second));
				}
				if (G[it->first].find(std::make_pair(it->second, ep.a)) != G[it->first].end()) {
					G[it->first].erase(std::make_pair(it->second, ep.a));
					G[it->first].insert(std::make_pair(it->second, ind));
				}
				//G[it->first].insert(std::make_pair(ind, it->second));
				if (G[it->second].find(std::make_pair(ep.a, it->first)) != G[it->second].end()) {
					G[it->second].erase(std::make_pair(ep.a, it->first));
					G[it->second].insert(std::make_pair(ind, it->first));
				}
				if (G[it->second].find(std::make_pair(it->first, ep.a)) != G[it->second].end()) {
					G[it->second].erase(std::make_pair(it->first, ep.a));
					G[it->second].erase(std::make_pair(it->first, ind));
				}
				//G[it->second].insert(std::make_pair(ind, it->first));
			}
			for (auto it = G[ep.b].begin(); it != G[ep.b].end(); it++) {
				if (Exist.find(it->first) == Exist.end()) continue;
				if (Exist.find(it->second) == Exist.end()) continue;
				unionG.insert(*it);
				if (G[it->first].find(std::make_pair(ep.b, it->second)) != G[it->first].end()) {
					G[it->first].erase(std::make_pair(ep.b, it->second));
					G[it->first].insert(std::make_pair(ind, it->second));
				}
				if (G[it->first].find(std::make_pair(it->second, ep.b)) != G[it->first].end()) {
					G[it->first].erase(std::make_pair(it->second, ep.b));
					G[it->first].insert(std::make_pair(it->second, ind));
				}
				//G[it->first].insert(std::make_pair(ind, it->second));
				if (G[it->second].find(std::make_pair(ep.b, it->first)) != G[it->second].end()) {
					G[it->second].erase(std::make_pair(ep.b, it->first));
					G[it->second].insert(std::make_pair(ind, it->first));
				}
				if (G[it->second].find(std::make_pair(it->first, ep.b)) != G[it->second].end()) {
					G[it->second].erase(std::make_pair(it->first, ep.b));
					G[it->second].insert(std::make_pair(it->first, ind));
				}
				//G[it->second].insert(std::make_pair(ind, it->first));
			}
			G.push_back(unionG);
			G[ep.a].clear();
			G[ep.b].clear();
			// calulateQ
			calulateQ(ind);
			//updateHeap
			updateHeap(ind);
		}
	}
	// update indices
	std::cout << "Face: " << indices.size() / 3 << std::endl;
	indices.clear();
	std::set<Face> faces;
	for (int i = 0; i < vertices.size() / 3; i++) {
		for (auto tri : G[i]) {
			if (Exist.find(i) == Exist.end()) continue;
			if (Exist.find(tri.first) == Exist.end()) continue;
			if (Exist.find(tri.second) == Exist.end()) continue;
			if (faces.find(Face(i, tri.first, tri.second)) == faces.end()) {
				indices.push_back(i);
				indices.push_back(tri.first);
				indices.push_back(tri.second);
				faces.insert(Face(i, tri.first, tri.second));
			}
		}
	}
	
	std::cout << "Face: " << indices.size() / 3 << std::endl;
}

void Model::save(const char * filename)
{
	FILE *fp = fopen(filename, "w");
	for (int i = 0; i < vertices.size(); i += 3)
	{
		fprintf(fp, "v %f %f %f\n", vertices[i], vertices[i + 1], vertices[i + 2]);
	}
	for (int i = 0; i < indices.size(); i += 3) {
		fprintf(fp, "f %d %d %d\n", indices[i] + 1, indices[i + 1] + 1, indices[i + 2] + 1);
	}
	fclose(fp);
}

void Model::makeG(std::vector<unsigned int> indices)
{
	G.clear();
	G.resize(vertices.size() / 3);
	for (int i = 0; i < indices.size(); i+=3) {
		int a = indices[i], b = indices[i + 1], c = indices[i + 2];
		G[a].insert(std::make_pair(b, c));
		G[b].insert(std::make_pair(c, a));
		G[c].insert(std::make_pair(a, b));
	}
}

void Model::makeE(std::vector<unsigned int> indices)
{
	E.clear();
	E.resize(vertices.size() / 3);
	for (int i = 0; i < indices.size(); i += 3) {
		int a = indices[i], b = indices[i + 1], c = indices[i + 2];
		E[a].insert(b); E[a].insert(c);
		E[b].insert(a); E[b].insert(c);
		E[c].insert(a); E[c].insert(b);
	}
}

*/
