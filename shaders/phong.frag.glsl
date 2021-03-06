// This is a texture sampler.  It lets you sample textures!  The keyword
// "uniform" means constant - sort of.  The uniform variables are the same
// for all fragments in an object, but they can change in between objects.
//test
uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;
uniform sampler2D shadowMap;

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

	// Normalize the normal, and calculate light vector and view vector
	// Note: this is doing a directional light, which is a little different
	// from what you did in Assignment 2.
    vec3 Tn = texture2D(normalMap, texcoord).rgb;


    //decompress the normal
    Tn = Tn*2.0 - 1.0;
		
	// Calculate the diffuse color coefficient, and sample the diffuse texture
	float Rd = max(0.0, dot(tangentL, Tn));
	vec3 Td = texture2D(diffuseMap, texcoord).rgb;
	vec3 diffuse = Rd * Kd * Td * gl_LightSource[0].diffuse.rgb;
	//vec3 diffuse = Kd * Td * gl_LightSource[0].diffuse.rgb;
	//vec3 diffuse = Rd * Kd * Td ;
	
	// Calculate the specular coefficient
	vec3 R = reflect(-tangentL, Td);
	float Rs = pow(max(0.0, dot(tangentV, R)), 1.0);
	vec3 Ts = texture2D(specularMap, texcoord).rgb;
	vec3 specular = Rs * Ks * Ts * gl_LightSource[0].specular.rgb;
	//vec3 specular = Rs * Ts * Ks;
		
	// Ambient is easy
	vec3 ambient = Ka * gl_LightSource[0].ambient.rgb;

    //check depth map 
    vec2 tex_shadow = vec2(depthPosition.x / depthPosition.w, depthPosition.y / depthPosition.w);
    float Zs = texture2D(shadowMap, tex_shadow).z;
    //gl_FragColor = vec4(depthPosition.z / depthPosition.w, 0.0, 0.0, 1.0);
    float depth = depthPosition.z / depthPosition.w;
    depth -= 0.005;
    //gl_FragColor = vec4(depthPosition.z / depthPosition.w, 0.0, 0.0, 1.0);

    //gl_FragColor = vec4(Zs, 0.0, 0.0, 1.0);
    //gl_FragColor = vec4(diffuse, 1.0);

    if( depthPosition.w > 0.0 && Zs < depth ) {
        gl_FragColor = vec4((diffuse + specular ) * 0.5, 1.0);
    } else {
        gl_FragColor = vec4(diffuse + specular, 1.0);
    }
    
    //gl_FragColor = vec4((diffuse + specular) * 0.5, 1.0);
}

