#version 120
uniform mat4 mat;
uniform vec3 light1;
uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float alpha;
uniform vec3 view_point;
uniform vec3 ambient_color;
uniform vec3 light1_color;
uniform vec3 obj_Color;

attribute vec3 Pos;
attribute vec3 Nor;
varying vec3 VNor;
varying vec3 VPos;
varying vec3 VColor;
varying vec3 Vview_point;

void main()
{
	VNor= Nor;
	VPos = Pos;
	VColor = obj_Color;
	Vview_point=view_point;

	gl_Position = mat * vec4(Pos, 1.0);
}