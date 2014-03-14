
// These are the "input" to our shader.  They are read from the vertex
// arrays that we specified in the C++ code.
//#version 120
attribute vec3 positionIn;
attribute vec2 texcoordIn;
attribute vec3 normalIn;
attribute vec3 tangentIn;
attribute vec3 bitangentIn;

//uniform mat4 meshMatrix;
// These are the "output" values of our shader.  OpenGL will interpolate
// these for us (good riddance project 2!)

void main() {

	// Transform the vertex to get the eye-space position of the vertex
	vec4 eyeTemp = gl_ModelViewMatrix * vec4(positionIn, 1);

	gl_Position = gl_ProjectionMatrix * eyeTemp;
}

