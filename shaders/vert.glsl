#version 120
uniform mat4 mat;
uniform vec3 light1;
attribute vec3 Pos;
attribute vec3 Nor;
varying vec3 vNor;
varying vec3 vPos;
varying vec3 vColor;

void main()
{
	vColor = Nor;
	vPos = Pos;

	gl_Position = mat * vec4(Pos, 1.0);
}