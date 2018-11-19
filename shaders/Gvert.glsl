#version 120
uniform mat4 mat;
uniform vec3 light1;
uniform vec3 light2;
uniform vec3 light3;
uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float alpha;
uniform vec3 view_point;
uniform vec3 ambient_color;
uniform vec3 light1_color;
uniform vec3 light2_color;
uniform vec3 light3_color;
uniform vec3 obj_Color;

attribute vec3 Pos;
attribute vec3 Nor;
varying vec3 vNor;
varying vec3 vPos;
varying vec3 vColor;

void main()
{
	vNor = Nor;
	vPos = Pos;
	vColor = obj_Color;

	//calculating color
	//AMBIENT
	vec3 ambient;
	ambient = ka * ambient_color;

	//DIFFUSE
	vec3 light1_vec = normalize(light1 - Pos);
	vec3 light2_vec = normalize(light2 - Pos);
	vec3 light3_vec = normalize(light3 - Pos);
	float diff1 = max(dot(Nor, light1_vec), 0.0f);
	float diff2 = max(dot(Nor, light2_vec), 0.0f);
	float diff3 = max(dot(Nor, light3_vec), 0.0f);
	vec3 diffuse = (kd * diff1 * light1_color) + (kd * diff2 * light2_color) + (kd * diff3 * light3_color);

	//SPECULAR
	vec3 R_vec1 = 2 * dot(light1_vec,Nor) * Nor - light1_vec;
	vec3 R_vec2 = 2 * dot(light2_vec,Nor) * Nor - light2_vec;
	vec3 R_vec3 = 2 * dot(light3_vec,Nor) * Nor - light3_vec;
	vec3 specular1 = light1_color * ks * pow(max(dot(R_vec1, normalize(view_point - Pos)),0), alpha);
	vec3 specular2 = light2_color * ks * pow(max(dot(R_vec2, normalize(view_point - Pos)),0), alpha);
	vec3 specular3 = light3_color * ks * pow(max(dot(R_vec3, normalize(view_point - Pos)),0), alpha);
	vec3 specular = specular1 + specular2 + specular3;

	//finalizing color
	vColor = (diffuse + ambient + specular) * vColor;
	if (vColor[0] > 1) { vColor[0] = 1; }
	if (vColor[1] > 1) { vColor[1] = 1; }
	if (vColor[2] > 1) { vColor[2] = 1; }

	gl_Position = mat * vec4(Pos, 1.0);
}