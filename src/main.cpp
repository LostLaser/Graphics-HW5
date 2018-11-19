#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include "BMPReader.h"
#define PI 3.14159265358

GLFWwindow *pWindow = NULL;
int windowWidth = 800;
int windowHeight = 600;
int horiz_view = 90, vert_view = 0, vert_factor = 1;
int light_move_factor_x = 1, light_move_factor_y = 1;
float a_reflectivity = 0.0, d_reflectivity = 0.0;
glm::vec3 ka = glm::vec3(0.2, 0.2, 0.2), kd = glm::vec3(0.75, 0.75, 0.75), ks = glm::vec3(0.9, 0.9, 0.9);
float alpha = 32.0;
glm::vec3 view_point = glm::vec3(0, 0, 10);
glm::vec3 ambient_color = glm::vec3(1, 1, 1);
glm::vec3 light1 = glm::vec3(0, 0, 10);
glm::vec3 light1_color = glm::vec3(1, 1, 1);
glm::vec3 light2 = glm::vec3(-20, 0, -10);
glm::vec3 light2_color = glm::vec3(1, 1, 1);
glm::vec3 light3 = glm::vec3(15, 15, 0);
glm::vec3 light3_color = glm::vec3(1, 1, 1);
glm::vec3 obj_Color = glm::vec3(0.5, 0.5, 0.5);
std::string shade_mode = "flat";
std::string norm_mode = "1";

struct Face
{
	int v[3];				// Vertex indices. Start from 0.
	glm::vec2 vt[3];		// Texture coordinates.
	glm::vec3 vn[3];		// Vertex normals.
};

std::vector<glm::vec3> vertex_vector;
std::vector<Face> face_vector;

// A very simple obj file reader. v_arr and f_arr are outputs.
// v_arr contains all vertices.
// f_arr contains all faces. 
bool objReader(const char *obj_file_name, std::vector<glm::vec3> &v_arr, std::vector<Face> &f_arr)
{
	v_arr.clear();
	f_arr.clear();
	std::ifstream ifs;
	ifs.open(obj_file_name);
	if (ifs.bad())
	{
		std::cout << "Fail to open the obj file!" << std::endl;
		return false;
	}
	bool has_vt = false;
	std::vector<glm::vec2> vt_arr;
	glm::vec3 p;
	glm::vec2 vt;
	Face f;
	int vt_i, vt_j, vt_k;
	char c;
	std::string line, type;
	std::stringstream ss;
	while (!ifs.eof())
	{
		getline(ifs, line);
		ss.clear();
		ss.str(line);
		type = "";
		ss >> type;
		if (type == "v")
		{
			ss >> p.x >> p.y >> p.z;
			v_arr.push_back(p);
		}
		else if (type == "vt")
		{
			if (!has_vt)
				has_vt = true;
			ss >> vt.x >> vt.y;
			vt_arr.push_back(vt);
		}
		else if(type == "f")
		{
			if (has_vt)
			{
				ss >> f.v[0] >> c >> vt_i >> f.v[1] >> c >> vt_j >> f.v[2] >> c >> vt_k;
				f.vt[0] = vt_arr[vt_i - 1];
				f.vt[1] = vt_arr[vt_j - 1];
				f.vt[2] = vt_arr[vt_k - 1];
			}
			else
			{
				ss >> f.v[0] >> f.v[1] >> f.v[2];
			}
			f.v[0]--;
			f.v[1]--;
			f.v[2]--;
			f_arr.push_back(f);
		}
	}
	ifs.close();
	return true;
}

// Shader program class. 
// Program::create() must be called to setup the shader.
class Program
{
private:
	GLuint vert_shader, frag_shader, vao, vbo, pid;
	int nTriangles;
public:
	Program() : pid(0) {};
	// parameters are the path to the shaders.
	// Returns true when shaders are set.
	bool create(const char* vertex_shader_name, const char* fragment_shader_name)
	{
		if (pid != 0)
		{
			std::cout << "This program is already created." << std::endl;
			return false;
		}
		GLint succ;
		vert_shader = glCreateShader(GL_VERTEX_SHADER);
		frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
		std::ifstream ifs;
		std::string str;
		std::stringstream ss;


		ifs.open(vertex_shader_name);
		if (ifs.bad())
		{
			std::cout << "Fail to open the vertex shader file!" << std::endl;
			return false;
		}
		ss << ifs.rdbuf();
		ifs.close();
		str = ss.str();
		const char* textvs = str.c_str();
		glShaderSource(vert_shader, 1, &textvs, NULL);
		glCompileShader(vert_shader);
		glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &succ);
		if (succ == GL_FALSE) {
			std::cout << "Fail to compile the vertex shader!" << std::endl;
			GLint msgLength;
			glGetShaderiv(vert_shader, GL_INFO_LOG_LENGTH, &msgLength);
			char* msg = new char[msgLength + 1];
			glGetShaderInfoLog(vert_shader, msgLength, NULL, msg);
			std::cout << msg << std::endl;
			glDeleteShader(vert_shader);
			vert_shader = 0;
			return false;
		}

		ss.str("");
		ifs.open(fragment_shader_name);
		if (ifs.bad())
		{
			std::cout << "Fail to open the vertex shader file!" << std::endl;
			return false;
		}
		ss << ifs.rdbuf();
		ifs.close();
		str = ss.str();
		const char* textfs = str.c_str();
		glShaderSource(frag_shader, 1, &textfs, NULL);
		glCompileShader(frag_shader);
		glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &succ);
		if (succ == GL_FALSE) {
			std::cout << "Fail to compile the fragment shader!" << std::endl;
			GLint msgLength;
			glGetShaderiv(frag_shader, GL_INFO_LOG_LENGTH, &msgLength);
			char* msg = new char[msgLength + 1];
			glGetShaderInfoLog(frag_shader, msgLength, NULL, msg);
			std::cout << msg << std::endl;
			glDeleteShader(frag_shader);
			frag_shader = 0;
			return false;
		}
		pid = glCreateProgram();
		glAttachShader(pid, vert_shader);
		glAttachShader(pid, frag_shader);
		glLinkProgram(pid);
		glDetachShader(pid, vert_shader);
		glDetachShader(pid, frag_shader);
		glUseProgram(pid);
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);	
		return true;
	}

	// Send attributes to shader. 
	void sendAttributes(/* TODO: Fill parameters you need here. */)
	{
		// TODO: Set nTriangles as the number of triangles of the mesh.
		nTriangles = face_vector.size();

		// (3 float for postion + 3 float for normal) * 3 vertices per triangle * nTriangles triangles
		float *data_buffer = new float[6 * 3 * nTriangles]; 
		
		std::vector<glm::vec3> vertex_norm_vector(vertex_vector.size());
		if (norm_mode == "2")
		{
			glm::vec3 face_norm;
			for (int i = 0; i < face_vector.size(); ++i)
			{
				face_norm = cross((vertex_vector[face_vector[i].v[2]] - vertex_vector[face_vector[i].v[1]]), (vertex_vector[face_vector[i].v[0]] - vertex_vector[face_vector[i].v[1]]));
				face_norm = normalize(face_norm);
				for (int k = 0; k < 3; ++k)
				{
					vertex_norm_vector[face_vector[i].v[k]] += face_norm;
				}
			}
			for (int r = 0; r < vertex_norm_vector.size(); ++r)
			{
				vertex_norm_vector[r] = normalize(vertex_norm_vector[r]);
			}
		}

		// TODO: Fill the data buffer.
		// The data buffer is filled triangle by triangle. Inside each triangle, data is filled vertex by vertex.
		// It should be (the memory address increases from the top to the bottom)
		// ====================================
		// position.x  | vertex 1 | triangle 1
		// position.y  |          |
		// position.z  |          |
		// normal.x    |          |
		// normal.y    |          |
		// normal.z    |          |
		// -----------------------|
		// position.x  | vertex 2 |
		// position.y  |          |
		// position.z  |          |
		// normal.x    |          |
		// normal.y    |          |
		// normal.z    |          |
		// -----------------------|
		// position.x  | vertex 3 |
		// position.y  |          |
		// position.z  |          |
		// normal.x    |          |
		// normal.y    |          |
		// normal.z    |          |
		// ====================================
		// position.x  | vertex 1 | triangle 2
		// position.y  |          |
		// position.z  |          |
		// normal.x    |          |
		// normal.y    |          |
		// normal.z    |          |

		glm::vec3 norm;
		for(int i=0;i<face_vector.size();++i)
		{
			if (shade_mode == "flat")
			{
				//getting face normal
				glm::vec3 vert_norm;
				if (norm_mode == "1")
				{
					vert_norm = cross((vertex_vector[face_vector[i].v[2]] - vertex_vector[face_vector[i].v[1]]), (vertex_vector[face_vector[i].v[0]] - vertex_vector[face_vector[i].v[1]]));
				}
				else
				if (norm_mode == "2")
				{
					vert_norm = vertex_norm_vector[face_vector[i].v[0]];
				}
				vert_norm = normalize(vert_norm);

				//setting the normal to the color (don't need to send it to shader)
				norm = obj_Color;
				
				//calculating color
				//AMBIENT
				glm::vec3 ambient;
				ambient = ka * ambient_color;

				//DIFFUSE
				glm::vec3 light1_vec = normalize(light1 - vertex_vector[face_vector[i].v[0]]);
				glm::vec3 light2_vec = normalize(light2 - vertex_vector[face_vector[i].v[0]]);
				glm::vec3 light3_vec = normalize(light3 - vertex_vector[face_vector[i].v[0]]);
				float diff1 = std::max(glm::dot(vert_norm, light1_vec), 0.0f);
				float diff2= std::max(glm::dot(vert_norm, light2_vec), 0.0f);
				float diff3 = std::max(glm::dot(vert_norm, light3_vec), 0.0f);
				glm::vec3 diffuse = (kd * diff1 * light1_color) + (kd * diff2 * light2_color) + (kd * diff3 * light3_color);

				//SPECULAR
				glm::vec3 R_vec1 = 2 * glm::dot(light1_vec, vert_norm) * vert_norm - light1_vec;
				glm::vec3 R_vec2 = 2 * glm::dot(light2_vec, vert_norm) * vert_norm - light2_vec;
				glm::vec3 R_vec3 = 2 * glm::dot(light3_vec, vert_norm) * vert_norm - light3_vec;
				glm::vec3 specular1 = light1_color * ks * pow(std::max(glm::dot(R_vec1, normalize(view_point - vertex_vector[face_vector[i].v[0]])), 0.0f), alpha);
				glm::vec3 specular2 = light2_color * ks * pow(std::max(glm::dot(R_vec2, normalize(view_point - vertex_vector[face_vector[i].v[0]])), 0.0f), alpha);
				glm::vec3 specular3 = light3_color * ks * pow(std::max(glm::dot(R_vec3, normalize(view_point - vertex_vector[face_vector[i].v[0]])), 0.0f), alpha);
				glm::vec3 specular = specular1 + specular2 + specular3;

				//setting the normal inputs to color
				norm = (diffuse + ambient + specular) * norm;
				//norm = (diffuse + ambient) * norm;
				if (norm[0] > 1) { norm[0] = 1; }
				if (norm[1] > 1) { norm[1] = 1; }
				if (norm[2] > 1) { norm[2] = 1; }
			}
			else
			{
				norm = cross((vertex_vector[face_vector[i].v[2]] - vertex_vector[face_vector[i].v[1]]), (vertex_vector[face_vector[i].v[0]] - vertex_vector[face_vector[i].v[1]]));
				norm = normalize(norm);
			}
			for(int k=0;k<3;++k)
			{
				if (norm_mode == "2" && shade_mode != "flat")
				{
					norm = vertex_norm_vector[face_vector[i].v[k]];
				}
				//fill pos.x, pos.y, pos.z;
				//fill nor.x, nor.y, nor.z;
				data_buffer[(i * 18) + (k * 6)] = vertex_vector[face_vector[i].v[k]][0];
				data_buffer[(i * 18) + (k * 6) + 1] = vertex_vector[face_vector[i].v[k]][1];
				data_buffer[(i * 18) + (k * 6) + 2] = vertex_vector[face_vector[i].v[k]][2];
				data_buffer[(i * 18) + (k * 6) + 3] = norm[0];
				data_buffer[(i * 18) + (k * 6) + 4] = norm[1];
				data_buffer[(i * 18) + (k * 6) + 5] = norm[2];
			}
		}

		/*for (int i = 0; i < 144; i+=6)
		{
			std::cout << data_buffer[i] << " ";
			std::cout << data_buffer[i+1] << " ";
			std::cout << data_buffer[i+2] << " ";
			std::cout << data_buffer[i+3] << " ";
			std::cout << data_buffer[i+4] << " ";
			std::cout << data_buffer[i+5] << std::endl;
		}*/
		

		glUseProgram(pid);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glBufferData(GL_ARRAY_BUFFER, 6 * 3 * nTriangles * sizeof(float), data_buffer, GL_STATIC_DRAW);
		glEnableVertexAttribArray(glGetAttribLocation(pid, "Pos"));
		glEnableVertexAttribArray(glGetAttribLocation(pid, "Nor"));

		glVertexAttribPointer(glGetAttribLocation(pid, "Pos"), 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), NULL);
		glVertexAttribPointer(glGetAttribLocation(pid, "Nor"), 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));

		// TODO(Honor Section Only): Texture mapping
		// 1. change data buffer size and the size in glBufferData()
		// 2. filling data buffer with positions, normals and texture coordinates.
		// 3. read referances about glVertexAttribPointer() and change parameters to make sure that texture coordinates has been sent to the shader.
		// 4. load the texture image (use the BMP reader provided in A2).
		// 5. add a attribute variable(vec2) for texture coordinates and a uniform variable(sampler2D) for the texture image in shader files.
		// 6. compute color in the fragment shader.

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	// Send an integer to the shader.
	void sendUniform(int a, const char* name)
	{
		glUseProgram(pid);
		glUniform1i(glGetUniformLocation(pid, name), a);
	}

	// Send a float number to the shader.
	void sendUniform(float a, const char* name)
	{
		glUseProgram(pid);
		glUniform1f(glGetUniformLocation(pid, name), a);
	}

	// Send a vec3 to the shader.
	void sendUniform(float x, float y, float z, const char* name)
	{
		glUseProgram(pid);
		glUniform3f(glGetUniformLocation(pid, name), x, y, z);
	}

	//send a matrix to the shader.
	void sendUniform(glm::mat4 &mat, const char* name)
	{
		glUseProgram(pid);
		glUniformMatrix4fv(glGetUniformLocation(pid, name), 1, GL_FALSE, &(mat[0][0]));
	}

	// TODO(Honor Section Only): Texture mapping
	// Complete this function to send the texture image to the shader.
	// Note: you don't need to know the uniform variable for texture image if you have only on texture.
	void sendUniform(BMPImage &img)
	{
		glUseProgram(pid);
		// TODO: setup and send the texture

	}
	void display()
	{
		glUseProgram(pid);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 3 * nTriangles);
		glBindVertexArray(0);
	}
};

// You can create several programs that use different shaders.
Program sample_program;

void KeyCallbackFunc(GLFWwindow* pwnd, int key, int scancode, int action, int mode)
{
	char cKey = (char)key;

	// TODO: setup the key callback function.
	if (action == GLFW_PRESS)
	{
		//Changes the x coordinate of the eye
		if (cKey == 'Z')
		{
			horiz_view+=10;
			vert_view = 0;
			sample_program.sendUniform(glm::perspective(3.1415f / 4, float(windowWidth) / float(windowHeight), 0.001f, 1000.0f) * glm::lookAt(glm::vec3(cos(horiz_view*PI/180)*10, 0, sin(horiz_view*PI / 180) * 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)), "mat");
			view_point = glm::vec3(cos(horiz_view*PI / 180) * 10, 0, sin(horiz_view*PI / 180) * 10);
			sample_program.sendAttributes();
			sample_program.sendUniform(view_point[0], view_point[1], view_point[2], "view_point");
		}
		else
		//Changes the y coordinate of the eye
		if (cKey == 'X')
		{
			if (vert_view < 90 && vert_view > -90) {
				vert_view += (10 * vert_factor);
			}
			else
			{
				vert_factor *= -1;
				vert_view += (10 * vert_factor);
			}
			horiz_view = 90;
			sample_program.sendUniform(glm::perspective(3.1415f / 4, float(windowWidth) / float(windowHeight), 0.001f, 1000.0f) * glm::lookAt(glm::vec3(0, sin(vert_view*PI / 180) * 10, cos(vert_view*PI / 180) * 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)), "mat");
			view_point = glm::vec3(0, sin(vert_view*PI / 180) * 10, cos(vert_view*PI / 180) * 10);
			sample_program.sendAttributes();
			sample_program.sendUniform(view_point[0], view_point[1], view_point[2], "view_point");

		}
		else
		//changes the x-coordinates of the lights
		if(cKey == 'A')
		{
			if (light1[0] > -20 && light1[0] < 20)
			{
				light1[0] += (3 * light_move_factor_x);
				light2[0] += (3 * light_move_factor_x);
				light3[0] += (3 * light_move_factor_x);
			}
			else
			{
				light_move_factor_x *= -1;
				light1[0] += (3 * light_move_factor_x);
				light2[0] += (3 * light_move_factor_x);
				light3[0] += (3 * light_move_factor_x);
			}
			sample_program.sendAttributes();
			sample_program.sendUniform(light1[0], light1[1], light1[2], "light1");
			sample_program.sendUniform(light2[0], light2[1], light2[2], "light2");
			sample_program.sendUniform(light3[0], light3[1], light3[2], "light3");
		}
		else
		//changes the x-coordinates of the lights
		if (cKey == 'S')
		{
			if (light1[1] > -20 && light1[1] < 20)
			{
				light1[1] += (3 * light_move_factor_y);
				light2[1] += (3 * light_move_factor_y);
				light3[1] += (3 * light_move_factor_y);
			}
			else
			{
				light_move_factor_y *= -1;
				light1[1] += (3 * light_move_factor_y);
				light2[1] += (3 * light_move_factor_y);
				light3[1] += (3 * light_move_factor_y);
			}
			sample_program.sendAttributes();
			sample_program.sendUniform(light1[0], light1[1], light1[2], "light1");
			sample_program.sendUniform(light2[0], light2[1], light2[2], "light2");
			sample_program.sendUniform(light3[0], light3[1], light3[2], "light3");
		}
		else
		//changes normal calculation to face
		if (cKey == '1')
		{
			norm_mode = "1";
			sample_program.sendAttributes();
		}
		else
		//changes normal calculation to avg
		if (cKey == '2')
		{
			norm_mode = "2";
			sample_program.sendAttributes();
		}
		else
		//flat shading mode
		if (cKey == 'Q')
		{
			horiz_view = 90;
			vert_view = 0;
			shade_mode = "flat";
			Program flat_program;
			bool did_create = flat_program.create("../shaders/vert.glsl", "../shaders/frag.glsl");
			if (did_create)
			{
				sample_program = flat_program;
				sample_program.sendUniform(glm::perspective(3.1415f / 4, float(windowWidth) / float(windowHeight), 0.001f, 1000.0f) * glm::lookAt(glm::vec3(0, 0, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)), "mat");
				sample_program.sendAttributes(/* TODO: Filling parameters*/);
			}
		}
		else
		//gouraud shading mode
		if (cKey == 'W')
		{
			horiz_view = 90;
			vert_view = 0;
			shade_mode = "gouraud";
			Program gouraud_program;
			bool did_create = gouraud_program.create("../shaders/Gvert.glsl", "../shaders/Gfrag.glsl");
			if(did_create) 
			{ 
				sample_program = gouraud_program; 
				sample_program.sendUniform(glm::perspective(3.1415f / 4, float(windowWidth) / float(windowHeight), 0.001f, 1000.0f) * glm::lookAt(glm::vec3(0, 0, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)), "mat");
				sample_program.sendAttributes(/* TODO: Filling parameters*/);
				sample_program.sendUniform(light1[0],light1[1],light1[2], "light1");
				sample_program.sendUniform(light2[0], light2[1], light2[2], "light2");
				sample_program.sendUniform(light3[0], light3[1], light3[2], "light3");
				sample_program.sendUniform(ka[0], ka[1], ka[2], "ka");
				sample_program.sendUniform(kd[0], kd[1], kd[2], "kd");
				sample_program.sendUniform(ks[0], ks[1], ks[2], "ks");
				sample_program.sendUniform(alpha, "alpha");
				sample_program.sendUniform(view_point[0], view_point[1], view_point[2], "view_point");
				sample_program.sendUniform(light1_color[0], light1_color[1], light1_color[2], "light1_color");
				sample_program.sendUniform(light2_color[0], light2_color[1], light2_color[2], "light2_color");
				sample_program.sendUniform(light3_color[0], light3_color[1], light3_color[2], "light3_color");
				sample_program.sendUniform(ambient_color[0], ambient_color[1], ambient_color[2], "ambient_color");
				sample_program.sendUniform(obj_Color[0], obj_Color[1], obj_Color[2], "obj_Color");
			}
		}
		else
		//phong shading mode
		if (cKey == 'E')
		{
			horiz_view = 90;
			vert_view = 0;
			shade_mode = "phong";
			Program phong_program;
			bool did_create = phong_program.create("../shaders/PHvert.glsl", "../shaders/PHfrag.glsl");
			if (did_create)
			{
				sample_program = phong_program;
				sample_program.sendUniform(glm::perspective(3.1415f / 4, float(windowWidth) / float(windowHeight), 0.001f, 1000.0f) * glm::lookAt(glm::vec3(0, 0, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)), "mat");
				sample_program.sendAttributes(/* TODO: Filling parameters*/);
				sample_program.sendUniform(light1[0], light1[1], light1[2], "light1");
				sample_program.sendUniform(light2[0], light2[1], light2[2], "light2");
				sample_program.sendUniform(light3[0], light3[1], light3[2], "light3");
				sample_program.sendUniform(ka[0], ka[1], ka[2], "ka");
				sample_program.sendUniform(kd[0], kd[1], kd[2], "kd");
				sample_program.sendUniform(ks[0], ks[1], ks[2], "ks");
				sample_program.sendUniform(alpha, "alpha");
				sample_program.sendUniform(view_point[0], view_point[1], view_point[2], "view_point");
				sample_program.sendUniform(light1_color[0], light1_color[1], light1_color[2], "light1_color");
				sample_program.sendUniform(light2_color[0], light2_color[1], light2_color[2], "light2_color");
				sample_program.sendUniform(light3_color[0], light3_color[1], light3_color[2], "light3_color");
				sample_program.sendUniform(ambient_color[0], ambient_color[1], ambient_color[2], "ambient_color");
				sample_program.sendUniform(obj_Color[0], obj_Color[1], obj_Color[2], "obj_Color");
			}
		}
	}
	else
	if (action == GLFW_RELEASE)
	{

	}
}

void WindowSizeCallbackFunc(GLFWwindow* pwnd, int width, int height)
{
	windowWidth = width;
	windowHeight = height;
	glViewport(0, 0, windowWidth, windowHeight);
	sample_program.sendUniform(glm::perspective(3.1415f / 4, float(windowWidth) / float(windowHeight), 0.001f, 1000.0f) * glm::lookAt(glm::vec3(0, 0, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)), "mat");
}

void init()
{
	glfwInit();
	glfwSetTime(0.0);
	if (pWindow)
	{
		glfwDestroyWindow(pWindow);
	}
	pWindow = glfwCreateWindow(windowWidth, windowHeight, "Assignment 5 - Jacob Goldsworthy", NULL, NULL);
	glfwMakeContextCurrent(pWindow);
	glewExperimental = true;
	glewInit();
	glViewport(0, 0, windowWidth, windowHeight);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glfwSetKeyCallback(pWindow, KeyCallbackFunc);
	glfwSetWindowSizeCallback(pWindow, WindowSizeCallbackFunc);
	
	sample_program.create("../shaders/vert.glsl", "../shaders/frag.glsl");
}

void display()
{
	sample_program.display(); 
}

int main(int argc, char** argv)
{
	init();
	if (argc != 2 && argc != 3)
	{
		std::cout << "Usage: " << argv[0] << " OBJ_FILE [TEXTURE_FILE]. " << std::endl;
		objReader("../obj/duck.obj", vertex_vector, face_vector);
	}
	else
	{
		objReader(argv[1], vertex_vector, face_vector);
	}
	
	// Simple examples of how to use Program class. Use them where you think necessary. 
	sample_program.sendUniform(glm::perspective(3.1415f / 4, float(windowWidth)/float(windowHeight), 0.001f, 1000.0f) * glm::lookAt(glm::vec3(0, 0, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)), "mat");
	sample_program.sendAttributes(/* TODO: Filling parameters*/);

	while (!glfwWindowShouldClose(pWindow)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		display();
		glfwSwapBuffers(pWindow);
		glfwPollEvents();
	}
	return 0;
}