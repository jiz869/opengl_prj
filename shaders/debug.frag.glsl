// This is a texture sampler.  It lets you sample textures!  The keyword
// "uniform" means constant - sort of.  The uniform variables are the same
// for all fragments in an object, but they can change in between objects.
//test
uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;

// Diffuse, ambient, and specular materials.  These are also uniform.
uniform vec3 Kd;
uniform vec3 Ks;
uniform vec3 Ka;
uniform float alpha;

// These are values that OpenGL interpoates for us.  Note that some of these
// are repeated from the fragment shader.  That's because they're passed
// across.
varying vec2 texcoord;
varying vec3 tangentL;  // L in tangent space;
varying vec3 tangentV;
varying vec4 depthPosition;

void main() {
    //vec4 depth = texture2D(depthMap, gl_TexCoord[0].st);
    vec4 depth = texture2D(depthMap, vec2(0.01, 0.05));

    gl_FragColor = vec4(depth.z, 0, 0, 1.0);
}

