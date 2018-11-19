#version 120
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

varying vec3 VNor;
varying vec3 VPos;
varying vec3 VColor;
varying vec3 Vview_point;


void main()
{
	//calculating color
	//AMBIENT
	vec3 ambient;
	ambient = ka * ambient_color;

	//DIFFUSE
	vec3 light1_vec = normalize(light1 - VPos);
	vec3 light2_vec = normalize(light2 - VPos);
	vec3 light3_vec = normalize(light3 - VPos);
	float diff1 = max(dot(VNor, light1_vec), 0.0f);
	float diff2 = max(dot(VNor, light2_vec), 0.0f);
	float diff3 = max(dot(VNor, light3_vec), 0.0f);
	vec3 diffuse = (kd * diff1 * light1_color) + (kd * diff2 * light2_color) + (kd * diff3 * light3_color);

	//SPECULAR
	vec3 R_vec1 = 2 * dot(light1_vec, VNor) * VNor - light1_vec;
	vec3 R_vec2 = 2 * dot(light2_vec, VNor) * VNor - light2_vec;
	vec3 R_vec3 = 2 * dot(light3_vec, VNor) * VNor - light3_vec;
	vec3 specular1 = light1_color * ks * pow(max(dot(R_vec1, normalize(view_point - VPos)),0), alpha);
	vec3 specular2 = light2_color * ks * pow(max(dot(R_vec2, normalize(view_point - VPos)),0), alpha);
	vec3 specular3 = light3_color * ks * pow(max(dot(R_vec3, normalize(view_point - VPos)),0), alpha);
	vec3 specular = specular1 + specular2 + specular3;

	//finalizing color
	vec3 color = (diffuse + ambient + specular) * VColor;
	if (color[0] > 1) { color[0] = 1; }
	if (color[1] > 1) { color[1] = 1; }
	if (color[2] > 1) { color[2] = 1; }

    gl_FragColor = vec4(color[0], color[1], color[2], 1.0);
}