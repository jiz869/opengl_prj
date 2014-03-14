// These are the "input" to our shader.  They are read from the vertex
// arrays that we specified in the C++ code.
//#version 120

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
varying vec4 depthPosition;

uniform mat4 shadowModelView;
uniform mat4 shadowProjection;

uniform mat4 meshMatrix;

mat3 Transpose(mat3 inMatrix)
{
    vec3 i0 = inMatrix[0];
    vec3 i1 = inMatrix[1];
    vec3 i2 = inMatrix[2];

    mat3 outMatrix = mat3(
            vec3(i0.x, i1.x, i2.x),
            vec3(i0.y, i1.y, i2.y),
            vec3(i0.z, i1.z, i2.z)
            );
    return outMatrix;
}

mat4 TransposeM4(mat4 inMatrix)
{
    vec4 i0 = inMatrix[0];
    vec4 i1 = inMatrix[1];
    vec4 i2 = inMatrix[2];
    vec4 i3 = inMatrix[3];

    mat4 outMatrix = mat4(
            vec4(i0.x, i1.x, i2.x, i3.x),
            vec4(i0.y, i1.y, i2.y, i3.y),
            vec4(i0.z, i1.z, i2.z, i3.z),
            vec4(i0.w, i1.w, i2.w, i3.w)
            );
    return outMatrix;
}

void main() {

	// Transform the vertex to get the eye-space position of the vertex
	vec4 eyeTemp = gl_ModelViewMatrix * vec4(positionIn, 1);
	//vec4 eyeTemp = gl_ModelViewMatrix * meshMatrix * vec4(positionIn, 1);
	//vec4 eyeTemp = shadowModelView * meshMatrix * vec4(positionIn, 1);

	eyePosition = eyeTemp.xyz;
    mat3 tbn = mat3(gl_NormalMatrix*tangentIn, gl_NormalMatrix*bitangentIn, gl_NormalMatrix*normalIn);
    //tbn = transpose(tbn);
    tbn = Transpose(tbn);

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

    //shadow map
    mat4 bias = mat4(0.5, 0.0, 0.0, 0.0, 
                     0.0, 0.5, 0.0, 0.0,
                     0.0, 0.0, 0.5, 0.0,
                     0.5, 0.5, 0.5, 1.0 );
    //depthPosition = bias * shadowProjection * shadowModelView * vec4(positionIn, 1);
    //depthPosition = bias * gl_ProjectionMatrix * gl_ModelViewMatrix * vec4(positionIn, 1);
    //mat4 transShadowModelView = TransposeM4(shadowModelView);
    depthPosition = bias * shadowProjection * shadowModelView * meshMatrix * vec4(positionIn, 1);
}

