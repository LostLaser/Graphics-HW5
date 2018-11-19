#version 120
varying vec3 vNor;
varying vec3 vPos;
varying vec3 vColor;
uniform vec3 light1;
uniform vec3 eye;

void main()
{
    gl_FragColor = vec4(vColor[0], vColor[1], vColor[2], 1.0);
}