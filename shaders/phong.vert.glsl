
// These are the "input" to our shader.  They are read from the vertex
// arrays that we specified in the C++ code.
#version 120
attribute vec3 positionIn;
attribute vec2 texcoordIn;
attribute vec3 normalIn;
attribute vec3 tangentIn;
attribute vec3 bitangentIn;

// These are the "output" values of our shader.  OpenGL will interpolate
// these for us (good riddance project 2!)
varying vec2 texcoord;
varying vec3 normal;
varying vec3 eyePosition;
varying vec3 tangentL;
varying vec3 tangentV;

void main() {

	// Transform the vertex to get the eye-space position of the vertex
	vec4 eyeTemp = gl_ModelViewMatrix * vec4(positionIn, 1);
	eyePosition = eyeTemp.xyz;
    mat3 tbn = mat3(gl_NormalMatrix*tangentIn, gl_NormalMatrix*bitangentIn, gl_NormalMatrix*normalIn);
    tbn = transpose(tbn);

	vec3 L = normalize(gl_LightSource[0].position.xyz);
    tangentL = tbn*L;

	vec3 V = normalize(-eyePosition);
    tangentV = tbn*V;

	// Transform again to get the clip-space position.  The gl_Position
	// variable tells OpenGL where the vertex should go.
	gl_Position = gl_ProjectionMatrix * eyeTemp;

	// Transform the normal, just like in Assignment 2.
	//normal = gl_NormalMatrix * normalIn;

	// Just copy the texture coordinates
	texcoord = texcoordIn;
}
