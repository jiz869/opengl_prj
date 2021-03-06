#include <cmath>
#include <iostream>
#include <iomanip>
#include "Framework.h"
#include "Shader.h"
#include "VEC3.h"
#include "DepthRenderTarget.h"

#define MODEL_PATH "models/cathedral.3ds"
//#define MODEL_PATH "models/dragon.dae"


// Note: See the SMFL documentation for info on setting up fullscreen mode
// and using rendering settings
// http://www.sfml-dev.org/tutorials/1.6/window-window.php
sf::WindowSettings settings(24, 8, 2);
sf::Window window(sf::VideoMode(800, 600), "Opengl/assimp/sfml", sf::Style::Close, settings);
 
// This is a clock you can use to control animation.  For more info, see:
// http://www.sfml-dev.org/tutorials/1.6/window-time.php
sf::Clock clck;

// This creates an asset importer using the Open Asset Import library.
// It automatically manages resources for you, and frees them when the program
// exits.
Assimp::Importer importer;
const aiScene* scene;
struct aiVector3D scene_min, scene_max, scene_center;

#define aisgl_min(x,y) (x<y?x:y)
#define aisgl_max(x,y) (y>x?y:x)

void initOpenGL();
void loadAssets();
void handleInput();
void renderFrame();

//shadow map
void renderShadowMap();
void setupShadowMap();
void bindShadowMap();
void unbindShadowMap();
//shadow map using DepthRenderTarget
void setupDepthRenderTarget();

void setupShadowLightMatrix();
void setupRenderMatrix();

//debug
void displayShadowTexture();

/////////     test      ///////////////////
static void vertexArrayTest();
/////////     test      ///////////////////
static void renderMesh_glsl(const struct aiScene *sc, const struct aiMesh *mesh);
void renderNode_glsl(const struct aiScene *sc, const struct aiNode *nd, aiMatrix4x4 parentM);

////////////////////////////////////////////////////////////////////////////////////////////
//static GLfloat eye_pos[3], eye_direction[3];
struct mat_textures{
    sf::Image *diffuseMap;
    sf::Image *specularMap;
    sf::Image *normalMap;
};

//index is material index
struct mat_textures mat_tex[50]= { 0 };

VEC3F eye_pos, eye_direction, eye_up;
VEC3F mouse_pos(0.0f, 0.0f, 0.0f);
VEC3F eye_rot(0.0f, 1.0f, 0.0f);
float eye_rot_degree=0.0f;
VEC3F eye_roty(-1.0f, 0.0f, 0.0f);
float eye_rot_degreey=0.0f;


bool show_error = true;

Shader *simpleShader=0;
Shader *normalmapShader=0;
Shader *debugShader=0;

//shadow map
VEC3F shadowlight_pos(0, 5, 30);
GLfloat shadowModelview[16];
GLfloat shadowProjection[16];

GLfloat normalModelview[16];
GLfloat normalProjection[16];

GLuint shadowTextureID;
GLuint fbod;

DepthRenderTarget *depthRender=0;

////////////////////////////////////////////////////////////////////////////////////////////
void identitymat4(GLfloat m[])
{
    int c=0, r=0;
    for(c=0; c<4; ++c) {
        for(r=0; r<4; ++r) {
            if(r==c) m[c*4+r] = 1.0;
            else
                m[c*4+r] = 0.0;
        }
    }
}

void outputmat4(GLfloat m[])
{
    int c=0, r=0;
    for(c=0; c<4; ++c) {
        for(r=0; r<4; ++r) {
            std::cout << m[c*3+r] << "  ";
        }
        std::cout << std::endl;
    }
}

void initEye()
{
    VEC3F ref_point(0.0f, 0.0f, 0.0f);
    //eye_pos.x = 0.0f; eye_pos.y = 0.0f; eye_pos.z = 8.0f;
    eye_pos.x = 0.0f; eye_pos.y = 1.5f; eye_pos.z = 8.0f;
    eye_up.x = 0.0f; eye_up.y = 1.f; eye_up.z = 0;
    eye_direction = ref_point - eye_pos;
    eye_direction.normalize();
}

void initShader()
{
    simpleShader = new Shader("shaders/simple");
    if(!simpleShader->loaded()) {
        std::cerr << "Simple shader failed to load" << std::endl;
        std::cerr << simpleShader->errors() << std::endl;
        exit(-1);
    }

    normalmapShader = new Shader("shaders/phong");
    if(!normalmapShader->loaded()) {
        std::cerr<< "Normal shader failed to load" << std::endl;
        std::cerr << normalmapShader->errors() << std::endl;
        exit(-1);
    }

    debugShader = new Shader("shaders/debug");
    if(!debugShader->loaded()) {
        std::cerr<< "Debug shader failed to load" << std::endl;
        std::cerr << debugShader->errors() << std::endl;
        exit(-1);
    }
}

int main(int argc, char** argv) {

    //initOpenGL();
    initEye();
    loadAssets();
    initShader();
    //setupShadowMap();
    setupDepthRenderTarget();

    // Put your game loop here (i.e., render with OpenGL, update animation)
    while (window.IsOpened()) {
        handleInput();
        //bindShadowMap();
        depthRender->bind();
        renderShadowMap();
        depthRender->unbind();
        //unbindShadowMap();
        
        //displayShadowTexture();
        renderFrame();
        window.Display();
    }

    return 0;
}



void initOpenGL() {
    // Initialize GLEW on Windows, to make sure that OpenGL 2.0 is loaded
#ifdef FRAMEWORK_USE_GLEW
    GLint error = glewInit();
    if (GLEW_OK != error) {
        std::cerr << glewGetErrorString(error) << std::endl;
        exit(-1);
    }
    if (!GLEW_VERSION_2_0 || !GL_EXT_framebuffer_object) {
        std::cerr << "This program requires OpenGL 2.0 and FBOs" << std::endl;
        exit(-1);
    }
#endif

    // This initializes OpenGL with some common defaults.  More info here:
    // http://www.sfml-dev.org/tutorials/1.6/window-opengl.php
    glClearDepth(1.0f);
    glClearColor(0.15f, 0.15f, 0.15f, 0.15f);
    glEnable(GL_DEPTH_TEST);

}


// ----------------------------------------------------------------------------
void get_bounding_box_for_node (const struct aiNode* nd, 
	struct aiVector3D* min, 
	struct aiVector3D* max, 
	struct aiMatrix4x4* trafo
){
	struct aiMatrix4x4 prev;
	unsigned int n = 0, t;

	prev = *trafo;
	aiMultiplyMatrix4(trafo,&nd->mTransformation);

	for (; n < nd->mNumMeshes; ++n) {
		const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
		for (t = 0; t < mesh->mNumVertices; ++t) {

			struct aiVector3D tmp = mesh->mVertices[t];
			aiTransformVecByMatrix4(&tmp,trafo);

			min->x = aisgl_min(min->x,tmp.x);
			min->y = aisgl_min(min->y,tmp.y);
			min->z = aisgl_min(min->z,tmp.z);

			max->x = aisgl_max(max->x,tmp.x);
			max->y = aisgl_max(max->y,tmp.y);
			max->z = aisgl_max(max->z,tmp.z);
		}
	}

	for (n = 0; n < nd->mNumChildren; ++n) {
		get_bounding_box_for_node(nd->mChildren[n],min,max,trafo);
	}
	*trafo = prev;
}

// ----------------------------------------------------------------------------
void get_bounding_box (struct aiVector3D* min, struct aiVector3D* max)
{
	struct aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);

	min->x = min->y = min->z =  1e10f;
	max->x = max->y = max->z = -1e10f;
	get_bounding_box_for_node(scene->mRootNode,min,max,&trafo);
}


void CheckAndLoadTexture(const struct aiScene *scene) 
{
    int i=0;
    for(i=0; i < scene->mNumMaterials; ++i) {
        aiMaterial *material = scene->mMaterials[i];
        aiString fullPath;
        aiString path; 
        aiString tex_path;
        fullPath = "models/";

        if( AI_SUCCESS == material->GetTexture( aiTextureType_DIFFUSE, 0, &tex_path ) ) { 
            //diffuse map
            path = tex_path;
            path.Append("_d.jpg");  
            fullPath.Append( path.data ); 
            mat_tex[i].diffuseMap = new sf::Image(); 
            mat_tex[i].diffuseMap->LoadFromFile( fullPath.data ); 
            mat_tex[i].diffuseMap->Bind(); 
            /*glGenerateMipmap(GL_TEXTURE_2D);*/ 
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
            std::cerr << "material[ "<< i <<" ]  : " << fullPath.data << std::endl; 

            //check normal map
            path = tex_path;
            path.Append("_n.jpg");  
            fullPath = "models/";
            fullPath.Append( path.data ); 
            mat_tex[i].normalMap = new sf::Image();
            if( mat_tex[i].normalMap->LoadFromFile( fullPath.data ) ) {
                mat_tex[i].normalMap->Bind();
                /*glGenerateMipmap(GL_TEXTURE_2D);*/ 
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
            std::cerr << "material[ "<< i <<" ]  : " << fullPath.data << std::endl; 
            }else{
                delete mat_tex[i].normalMap;
                mat_tex[i].normalMap = 0;
            }

            //specular map
            path = tex_path;
            path.Append("_s.jpg");  
            fullPath = "models/";
            fullPath.Append( path.data ); 
            mat_tex[i].specularMap = new sf::Image();
            if( mat_tex[i].specularMap->LoadFromFile( fullPath.data ) ) {
                mat_tex[i].specularMap->Bind();
                /*glGenerateMipmap(GL_TEXTURE_2D);*/ 
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
            std::cerr << "material[ "<< i <<" ]  : " << fullPath.data << std::endl; 
            }else{
                delete mat_tex[i].specularMap;
                mat_tex[i].specularMap = 0;
            }
        }else{  
            mat_tex[i].diffuseMap = 0; 
            mat_tex[i].normalMap = 0; 
            mat_tex[i].specularMap = 0; 
        }
        
    }
}

//#define AI_CONFIG_PP_SBP_REMOVE (aiPrimitiveType_POINTS | aiPrimitiveType_LINES)
void loadAssets() {
    // Read in an asset file, and do some post-processing.  There is much 
    // more you can do with this asset loader, including load textures.
    // More info is here:
    // http://assimp.sourceforge.net/lib_html/usage.html
    importer.SetPropertyInteger( AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE|aiPrimitiveType_POINT );
    scene = importer.ReadFile(MODEL_PATH,  
        aiProcess_SortByPType |
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcessPreset_TargetRealtime_Quality);

    if (!scene || scene->mNumMeshes <= 0) {
        std::cerr << importer.GetErrorString() << std::endl;
        exit(-1);
    }

    //get scale
    get_bounding_box(&scene_min, &scene_max);
    scene_center.x = (scene_min.x + scene_max.x)/2.0f;
    scene_center.y = (scene_min.y + scene_max.y)/2.0f;
    scene_center.z = (scene_min.z + scene_max.z)/2.0f;
        
    //////////////////////////////////////////////////////////////////////////
    // TODO: LOAD YOUR SHADERS/TEXTURES
    //////////////////////////////////////////////////////////////////////////
    CheckAndLoadTexture(scene);

}

void handleInput() {
    //////////////////////////////////////////////////////////////////////////
    // TODO: ADD YOUR INPUT HANDLING HERE. 
    //////////////////////////////////////////////////////////////////////////

    // Event loop, for processing user input, etc.  For more info, see:
    // http://www.sfml-dev.org/tutorials/1.6/window-events.php
    sf::Event evt;
    while (window.GetEvent(evt)) {
        switch (evt.Type) {
        case sf::Event::Closed: 
            // Close the window.  This will cause the game loop to exit,
            // because the IsOpened() function will no longer return true.
            window.Close(); 
            break;
        case sf::Event::Resized: 
            // If the window is resized, then we need to change the perspective
            // transformation and viewport
            glViewport(0, 0, evt.Size.Width, evt.Size.Height);
            break;
        case sf::Event::KeyPressed:
            if( evt.Key.Code == sf::Key::Escape ) {
                window.Close();
                break;
            } else if( evt.Key.Code == sf::Key::W ) {
                //Move camera forward
                eye_pos += eye_direction*0.5f;
            } else if( evt.Key.Code == sf::Key::S ) {
                eye_pos -= eye_direction*0.5f;
            } else if( evt.Key.Code == sf::Key::A ) {
                VEC3F l = eye_up^eye_direction;
                l.normalize();
                eye_pos += l*0.5f;
            }else if( evt.Key.Code == sf::Key::D ) {
                VEC3F r = eye_direction^eye_up;
                r.normalize();
                eye_pos += r*0.5f;
            }else if( evt.Key.Code == sf::Key::Up ) {
                shadowlight_pos.y += 0.5;
            }else if( evt.Key.Code == sf::Key::Down ) {
                shadowlight_pos.y -= 0.5;
            }else if( evt.Key.Code == sf::Key::Left ) {
                shadowlight_pos.z -= 0.5;
            }else if( evt.Key.Code == sf::Key::Right ) {
                shadowlight_pos.z += 0.5;
            }
            break;
        case sf::Event::MouseMoved:
            {
                VEC3F new_mouse_pos( evt.MouseMove.X, evt.MouseMove.Y, 0.0f);
                float dx = new_mouse_pos.x - mouse_pos.x;
                float dy = new_mouse_pos.y - mouse_pos.y;
                mouse_pos = new_mouse_pos;
                
                //if( abs(dx) > abs(dy) ) {
                    /*
                    eye_rot = eye_up;
                    if( dx > 0.1 ) {
                        eye_rot_degree += 1.0;
                    }else if( dx < 0.01 ) {
                        eye_rot_degree -= 1.0;
                    }else {
                    }
                    */
                    VEC3F e_target = eye_pos + eye_direction;
                    eye_roty = eye_up^eye_direction;
                    eye_roty.normalize();
                    if( dx > 2 ) {
                        e_target -= eye_roty*0.05f;
                    }else if( dx < -2 ) {
                        e_target += eye_roty*0.05f;
                    }
                    eye_direction = e_target - eye_pos;
                    eye_direction.normalize();
                    
               // }else{
                    /*
                    eye_roty = eye_up^eye_direction;
                    if( dy > 0.1 ) {
                        eye_rot_degreey -= 1.0;
                    }else if( dy < 0.01 ) {
                        eye_rot_degreey += 1.0;
                    }else{
                    }
                    */
                    eye_roty = eye_up^eye_direction;
                    //VEC3F e_target = eye_pos + eye_direction;
                    e_target = eye_pos + eye_direction;
                    if( dy > 2 ) {
                        e_target -= eye_up*0.05f;
                    }else if( dy < -2 ) {
                        e_target += eye_up*0.05f;
                    }
                        eye_direction = e_target - eye_pos;
                        eye_direction.normalize();
                        eye_up = eye_direction^eye_roty;
                        eye_up.normalize();
                //}
            }
            break;
        default: 
            break;
        }
    }
}


//---------------------------------------------------------- general glBegin/glEnd drawing method -------------------------------------
void setTexture(const struct aiScene *sc, const struct aiMesh* mesh)
{
        unsigned int mi = mesh->mMaterialIndex;
        if( mat_tex[mi].diffuseMap != 0 ) {
            glEnable(GL_TEXTURE_2D);
            mat_tex[mi].diffuseMap->Bind();
        }

}

void renderMesh(const struct aiScene *sc, const struct aiMesh *mesh)
{
    unsigned int f=0;
    unsigned int v=0;
    aiMaterial *material = sc->mMaterials[ mesh->mMaterialIndex ];

    if( mesh->HasNormals() ) {
        glEnable(GL_LIGHTING);
    }else{
        glDisable(GL_LIGHTING);
    }
    
    //check and load texture
    setTexture(sc, mesh);

    // set material colors
    aiColor3D color;
    GLfloat gl_color[3];
    if( AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, color) ) {
        gl_color[0] = color.r;
        gl_color[1] = color.g;
        gl_color[2] = color.b;
        glMaterialfv(GL_FRONT, GL_DIFFUSE, gl_color);
    }else {
        std::cerr << "material get color_diffuse failed" << std::endl;
    }

    //specular material
    if( AI_SUCCESS == material->Get(AI_MATKEY_COLOR_SPECULAR, color) ) {
        gl_color[0] = color.r;
        gl_color[1] = color.g;
        gl_color[2] = color.b;
        //glMaterialfv(GL_FRONT, GL_SPECULAR, gl_color);
    }else {
        std::cerr << "material get color_specular failed" << std::endl;
    }

    //ambient material
    if( AI_SUCCESS == material->Get(AI_MATKEY_COLOR_AMBIENT, color) ) {
        gl_color[0] = color.r;
        gl_color[1] = color.g;
        gl_color[2] = color.b;
        //glMaterialfv(GL_FRONT, GL_AMBIENT, gl_color);
    }else {
        std::cerr << "material get color_ambient failed" << std::endl;
    }

    for(f=0; f < mesh->mNumFaces; ++f) {
        const struct aiFace *face = &mesh->mFaces[f];

        glBegin(GL_TRIANGLES);
        for(v=0; v < face->mNumIndices; ++v) {
            int index = face->mIndices[v];
            if(mesh->mNormals != NULL) {
                glNormal3fv(&mesh->mNormals[index].x);
            }
            glVertex3fv(&mesh->mVertices[index].x);
        }

        glEnd();
    }
}

void renderNode(const struct aiScene *sc, const struct aiNode *nd)
{
    struct aiMatrix4x4 m = nd->mTransformation;
    unsigned int i;

    //update transform
    aiTransposeMatrix4(&m); //assimp matrix is row major. Need transpose for gl
    glPushMatrix();
    glMultMatrixf((float*)&m);

    //draw this node
    for(i=0; i < nd->mNumMeshes; ++i) {
        const struct aiMesh *mesh = sc->mMeshes[ nd->mMeshes[i] ];
        renderMesh(sc, mesh);
    }

    //draw all children
    unsigned int n;
    for(n=0; n < nd->mNumChildren; ++n) {
        renderNode( sc, nd->mChildren[n]);
    }

    glPopMatrix();
}

//------------------------------------------ Use vertex array -------------------
void renderMesh_VertexArray(const struct aiScene *sc, const struct aiMesh *mesh  )
{
    int v=0, f=0;
    //aiMaterial *material = sc->mMaterials[ mesh->mMaterialIndex ];
    if( mesh->HasNormals() ) {
        glEnable(GL_LIGHTING);
    }else{
        glDisable(GL_LIGHTING);
    }

    //prepare vertices buffer
    int num_idx = mesh->mNumFaces * 3;
    unsigned int *index = (unsigned int*)malloc( num_idx * sizeof(unsigned int) );
    int vi=0;

    //set index array
    for(vi=0, f=0; f < mesh->mNumFaces; ++f) {
        const struct aiFace *face = &mesh->mFaces[f];
        for(v=0; v < face->mNumIndices; ++v ) {
            index[vi] = face->mIndices[v];
            vi++;
        }
    }

    //bind texture if any
    unsigned int mi = mesh->mMaterialIndex;
    if( mat_tex[mi].diffuseMap != 0 ) {
        glEnable(GL_TEXTURE_2D);
        mat_tex[mi].diffuseMap->Bind();
    }else{
        //std::cerr << "mesh doesn't have a texture" << std::endl;
        glEnable(GL_TEXTURE_2D);
    }

    //transfer vertex attributes
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(aiVector3D), mesh->mVertices);
    glNormalPointer(GL_FLOAT, sizeof(aiVector3D), mesh->mNormals);
    glTexCoordPointer(2, GL_FLOAT, sizeof(aiVector3D), mesh->mTextureCoords[0]);
    glDrawElements(GL_TRIANGLES, mesh->mNumFaces*3, GL_UNSIGNED_INT, &index[0]);
    //GLenum glerr = glGetError();
    //std::cerr << "Opengl error code 0x"<< std::hex << glerr << std::endl;
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

    if( mat_tex[mi].diffuseMap != 0 ) {
        glDisable(GL_TEXTURE_2D);
    }else{
        glDisable(GL_TEXTURE_2D);
    }

    free(index);
}

void renderNode_VertexArray(const struct aiScene *sc, const struct aiNode *nd)
{
    struct aiMatrix4x4 m = nd->mTransformation;
    unsigned int i;

    //update transform
    aiTransposeMatrix4(&m); //assimp matrix is row major. Need transpose for gl
    glPushMatrix();
    glMultMatrixf((float*)&m);

    //draw meshes in this node
    for(i=0; i < nd->mNumMeshes; ++i) {
        const struct aiMesh *mesh = sc->mMeshes[ nd->mMeshes[i] ];
        //renderMesh_VertexArray(sc, mesh);
        renderMesh_glsl(sc, mesh);
    }

    //draw all children
    unsigned int n;
    for(n=0; n < nd->mNumChildren; ++n) {
        renderNode_VertexArray( sc, nd->mChildren[n] ); 
    }

    glPopMatrix();
}

void renderNode_glsl(const struct aiScene *sc, const struct aiNode *nd, aiMatrix4x4 parentM)
{
    struct aiMatrix4x4 m = nd->mTransformation;
    struct aiMatrix4x4 meshM;
    unsigned int i;

    //compute parentM and meshM
    parentM *= m;
    meshM = parentM;
    aiTransposeMatrix4(&meshM);

    //update transform
    aiTransposeMatrix4(&m); //assimp matrix is row major. Need transpose for gl
    glPushMatrix();
    glMultMatrixf((float*)&m);

    //pass meshMatrix 
    GLint meshMatrixId = glGetUniformLocation(normalmapShader->programID(), "meshMatrix");
    glUniformMatrix4fv(meshMatrixId, 1, GL_FALSE, (float*)&meshM);

    //draw meshes in this node
    for(i=0; i < nd->mNumMeshes; ++i) {
        const struct aiMesh *mesh = sc->mMeshes[ nd->mMeshes[i] ];
        renderMesh_glsl(sc, mesh);
    }

    //draw all children
    unsigned int n;
    for(n=0; n < nd->mNumChildren; ++n) {
        renderNode_glsl( sc, nd->mChildren[n], parentM); 
    }

    glPopMatrix();
}

//-------------------------------------------------Use glsl customized shader ----------------------------------------
void renderMesh_glsl(const struct aiScene *sc, const struct aiMesh *mesh)
{
    int v=0, f=0;
    
    aiMaterial *material = sc->mMaterials[ mesh->mMaterialIndex ];
    if( mesh->HasNormals() ) {
        glEnable(GL_LIGHTING);
    }else{
        glDisable(GL_LIGHTING);
    }

    //prepare vertices buffer
    int num_idx = mesh->mNumFaces * 3;
    unsigned int *index = (unsigned int*)malloc( num_idx * sizeof(unsigned int) );
    int vi=0;

    //create index array
    for(vi=0, f=0; f < mesh->mNumFaces; ++f) {
        const struct aiFace *face = &mesh->mFaces[f];
        for(v=0; v < face->mNumIndices; ++v ) {
            index[vi] = face->mIndices[v];
            vi++;
        }
    }

    //set material
    aiColor3D color;
    GLint diffuse = glGetUniformLocation(normalmapShader->programID(), "Kd");
    material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    glUniform3f(diffuse, color.r, color.g, color.b);

    GLint specular = glGetUniformLocation(normalmapShader->programID(), "Ks");
    material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    glUniform3f(specular, color.r, color.g, color.b);

    GLint ambient = glGetUniformLocation(normalmapShader->programID(), "Ka");
    material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    glUniform3f(ambient, color.r, color.g, color.b);

    GLint shininess = glGetUniformLocation(normalmapShader->programID(), "alpha");
    float value;
    if(AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, value)) {
        glUniform1f(shininess, value);
    }else{
        glUniform1f(shininess, 0);
    }

    //set textures
    unsigned int mi = mesh->mMaterialIndex;
    if( mat_tex[mi].diffuseMap != 0 ) {
        GLint diffuse = glGetUniformLocation(normalmapShader->programID(), "diffuseMap");
        glUniform1i(diffuse, 0);    // the diffuse map will be GL_TEXTURE0
        glActiveTexture(GL_TEXTURE0);
        mat_tex[mi].diffuseMap->Bind();
    }

    if( mat_tex[mi].specularMap != 0 ) {
        GLint specular = glGetUniformLocation(normalmapShader->programID(), "specularMap");
        glUniform1i(specular, 1);   // the specular map will be GL_TEXTURE1
        glActiveTexture(GL_TEXTURE1);
        mat_tex[mi].specularMap->Bind();
    }

    if( mat_tex[mi].normalMap != 0 ) {
        GLint normalmap = glGetUniformLocation(normalmapShader->programID(), "normalMap");
        glUniform1i(normalmap, 2);   // the normal map will be GL_TEXTURE2
        glActiveTexture(GL_TEXTURE2);
        mat_tex[mi].normalMap->Bind();
    }

    
    //set mesh data
    
    //set vertex positions
    GLint position = glGetAttribLocation(normalmapShader->programID(), "positionIn");
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, 3, GL_FLOAT, 0, sizeof(aiVector3D), mesh->mVertices);

    //set texure coordinator
    GLint texcoord = glGetAttribLocation(normalmapShader->programID(), "texcoordIn");
    glEnableVertexAttribArray(texcoord);
    glVertexAttribPointer(texcoord, 2, GL_FLOAT, 0, sizeof(aiVector3D), mesh->mTextureCoords[0]);

    //set normals
    if(mesh->HasNormals()) {
        GLint normal = glGetAttribLocation(normalmapShader->programID(), "normalIn");
        glEnableVertexAttribArray(normal);
        glVertexAttribPointer(normal, 3, GL_FLOAT, 0, sizeof(aiVector3D), mesh->mNormals);
    }

    //set tagents
    if(mesh->HasTangentsAndBitangents()) {
        GLint tangent = glGetAttribLocation(normalmapShader->programID(), "tangentIn");
        glEnableVertexAttribArray(tangent);
        glVertexAttribPointer(tangent, 3, GL_FLOAT, 0, sizeof(aiVector3D), mesh->mTangents);

        GLint bitangent = glGetAttribLocation(normalmapShader->programID(), "bitangentIn");
        glEnableVertexAttribArray(bitangent);
        glVertexAttribPointer(bitangent, 3, GL_FLOAT, 0, sizeof(aiVector3D), mesh->mBitangents);
    }

    glDrawElements(GL_TRIANGLES, mesh->mNumFaces*3, GL_UNSIGNED_INT, &index[0]);

    free(index);

}

void setupRenderMatrix()
{
   	const double aspectRatio = ((float) window.GetWidth() / (float)window.GetHeight()), fieldOfView = 45.0;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fieldOfView, aspectRatio, 1.0, 1000.0);  /* Znear and Zfar */
    //glOrtho(-22.0, 22.0, -15.0, 20.0, 1, 1000);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eye_pos.x, eye_pos.y, eye_pos.z,
              eye_pos.x+eye_direction.x, eye_pos.y+eye_direction.y, eye_pos.z+eye_direction.z,
              eye_up.x, eye_up.y, eye_up.z );

    glRotatef(90, 0.0f, 1.0f, 0.0f);
    //set light
    GLfloat lpos[] = {0.0, 11.0, 0.0, 1.0};
    glLightfv(GL_LIGHT0, GL_POSITION, lpos);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);    

}

void renderFrame() {
    //////////////////////////////////////////////////////////////////////////
    // TODO: ADD YOUR RENDERING CODE HERE.  You may use as many .cpp files 
    // in this assignment as you wish.
    //////////////////////////////////////////////////////////////////////////
    //

    //render shadow map first

    ///// reset opengl //////////
    initOpenGL();

    /////////////////////////////
    
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //set light
#if 0
    GLfloat lpos[] = {0.0, 11.0, 0.0, 1.0};
    glLightfv(GL_LIGHT0, GL_POSITION, lpos);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);    
#endif

    //glEnable(GL_LIGHT1);    
    //glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

    glViewport(0, 0, window.GetWidth(), window.GetHeight());
    setupRenderMatrix();
    //setupShadowLightMatrix();
    
    // scale the whole asset to fit into our view frustum 
	//tmp = scene_max.x-scene_min.x;
	//tmp = aisgl_max(scene_max.y - scene_min.y,tmp);
	//tmp = aisgl_max(scene_max.z - scene_min.z,tmp);
	//tmp = 1.f / tmp;
	//glScalef(tmp, tmp, tmp);

    //center the model

    //glRotatef(eye_rot_degreey, eye_roty.x, eye_roty.y, eye_roty.z);
    //glRotatef(eye_rot_degree, eye_rot.x, eye_rot.y, eye_rot.z);
    //glTranslatef(-scene_center.x, -scene_center.y, -scene_center.z);    //move center to the origin
    
    //try various render methods
    
    //renderNode(scene, scene->mRootNode);
    //choose normalMapshader
    glUseProgram(normalmapShader->programID());

    //depth texture
    GLint shadow = glGetUniformLocation(normalmapShader->programID(), "shadowMap");
    glUniform1i(shadow, 3);
    glActiveTexture(GL_TEXTURE3);
    glEnable(GL_TEXTURE_2D);

    //glBindTexture(GL_TEXTURE_2D, shadowTextureID);
    //depthRender
    glBindTexture(GL_TEXTURE_2D, depthRender->textureID());

    glGetFloatv(GL_MODELVIEW_MATRIX, normalModelview); 

    //push matrix
    GLint iProjectionMatrix = glGetUniformLocation(normalmapShader->programID(), "shadowProjection");
    glUniformMatrix4fv(iProjectionMatrix, 1, GL_FALSE, shadowProjection);

    GLint iModelViewMatrix = glGetUniformLocation(normalmapShader->programID(), "shadowModelView");
    glUniformMatrix4fv(iModelViewMatrix, 1, GL_FALSE, shadowModelview);

    glGetFloatv(GL_PROJECTION_MATRIX, normalProjection);

    //renderNode_VertexArray(scene, scene->mRootNode);
    struct aiMatrix4x4 rootMeshM(1.0, 0.0, 0.0, 0.0,
                                 0.0, 1.0, 0.0, 0.0,
                                 0.0, 0.0, 1.0, 0.0,
                                 0.0, 0.0, 0.0, 1.0);
                
    renderNode_glsl(scene, scene->mRootNode, rootMeshM);

    if( show_error == true ) show_error = false;
}

void setupShadowMap()
{
    //create texture
    glGenTextures(1, &shadowTextureID);
    glBindTexture(GL_TEXTURE_2D, shadowTextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, window.GetWidth(), window.GetHeight(), 
            0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
    //glEnable(GL_TEXTURE_2D);       // do we need enable this?

    //attach texture to the framebuffer
    glGenFramebuffersEXT(1, &fbod);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbod);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, shadowTextureID, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    //check the status of the fbo
    if(GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)) {
        std::cerr << "Invalid framebuffer configuration" << std::endl;
        exit(-1);
    }else{
        std::cerr << "framebuffer config success"<< std::endl;
    }
    glDrawBuffer(GL_BACK);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void bindShadowMap()
{
    glPushAttrib(GL_VIEWPORT_BIT);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbod);
    glDrawBuffer(GL_NONE);
    glViewport(0, 0, window.GetWidth(), window.GetHeight());
}

void unbindShadowMap()
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glDrawBuffer(GL_BACK);
    glPopAttrib();
}

void setupShadowLightMatrix()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //bounding box: (xyz) (-21, -3, -13) - (21, 19, 13)
    glOrtho(-15.0, 15.0, -8.0, 30.0, 1, 50);
   	//const double aspectRatio = ((float) window.GetWidth() / (float)window.GetHeight()), fieldOfView = 45.0;
	//gluPerspective(fieldOfView, aspectRatio, 1.0, .0);  /* Znear and Zfar */

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    VEC3F up = shadowlight_pos ^ VEC3F(1.0, 0.0, 0.0);
    //VEC3F up = VEC3F(1.0, 0.0, 0.0) ^ (VEC3F(0.0, 1.5, 8) - shadowlight_pos);
    gluLookAt(shadowlight_pos.x, shadowlight_pos.y, shadowlight_pos.z, 
            0.0, 0.0, 0.0,
            up.x, up.y, up.z);

    //gluLookAt(shadowlight_pos.x, shadowlight_pos.y, shadowlight_pos.z, 
    //        -shadowlight_pos.x, -shadowlight_pos.y, -shadowlight_pos.z,
    //        up.x, up.y, up.z);

    //gluLookAt(eye_pos.x, eye_pos.y, eye_pos.z,
    //          eye_pos.x+eye_direction.x, eye_pos.y+eye_direction.y, eye_pos.z+eye_direction.z,
    //          eye_up.x, eye_up.y, eye_up.z );

    glRotatef(90, 0.0f, 1.0f, 0.0f);

}


void renderMesh_glsl_simple(const struct aiScene *sc, const struct aiMesh *mesh)
{
    int v=0, f=0;
    
    aiMaterial *material = sc->mMaterials[ mesh->mMaterialIndex ];

    //prepare vertices buffer
    int num_idx = mesh->mNumFaces * 3;
    unsigned int *index = (unsigned int*)malloc( num_idx * sizeof(unsigned int) );
    int vi=0;

    //create index array
    for(vi=0, f=0; f < mesh->mNumFaces; ++f) {
        const struct aiFace *face = &mesh->mFaces[f];
        for(v=0; v < face->mNumIndices; ++v ) {
            index[vi] = face->mIndices[v];
            vi++;
        }
    }

    //set vertex positions
    GLint position = glGetAttribLocation(normalmapShader->programID(), "positionIn");
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, 3, GL_FLOAT, 0, sizeof(aiVector3D), mesh->mVertices);

    glDrawElements(GL_TRIANGLES, mesh->mNumFaces*3, GL_UNSIGNED_INT, &index[0]);

    free(index);

}

void renderNode_VertexArray_simple(const struct aiScene *sc, const struct aiNode *nd)
{
    struct aiMatrix4x4 m = nd->mTransformation;
    unsigned int i;

    //update transform
    aiTransposeMatrix4(&m); //assimp matrix is row major. Need transpose for gl
    glPushMatrix();
    glMultMatrixf((float*)&m);

    //pass meshMatrix 
    //GLint meshMatrixId = glGetUniformLocation(simpleShader->programID(), "meshMatrix");
    //glUniformMatrix4fv(meshMatrixId, 1, GL_FALSE, (float*)&m);

    //draw meshes in this node
    for(i=0; i < nd->mNumMeshes; ++i) {
        const struct aiMesh *mesh = sc->mMeshes[ nd->mMeshes[i] ];
        //renderMesh_VertexArray(sc, mesh);
        renderMesh_glsl_simple(sc, mesh);
    }

    //draw all children
    unsigned int n;
    for(n=0; n < nd->mNumChildren; ++n) {
        renderNode_VertexArray_simple( sc, nd->mChildren[n]); 
    }

    glPopMatrix();
}

void renderShadowMap()
{
    //use simple shader
    glUseProgram(simpleShader->programID());

    //setup light camera
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    //setup matrices
    setupShadowLightMatrix();
    //setupRenderMatrix();

    GLenum glerr; 
    glerr = glGetError();

    //save shadow light matrix
    glGetFloatv(GL_PROJECTION_MATRIX, shadowProjection);
    glGetFloatv(GL_MODELVIEW_MATRIX, shadowModelview); 
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    renderNode_VertexArray_simple(scene, scene->mRootNode);

    //restore state
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glCullFace(GL_BACK);
}

void displayShadowTexture()
{
     int w = window.GetWidth();
     int h = window.GetHeight();

     glClearDepth(1.0f);
     glClearColor(0.15f, 0.15f, 0.15f, 0.15f);

     glViewport(0, 0, w, h);
     glUseProgram(debugShader->programID());

     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	 glMatrixMode(GL_PROJECTION);
	 glLoadIdentity();
	 glOrtho(-w/2,w/2,-h/2,h/2,-1000,1000);

	 glMatrixMode(GL_MODELVIEW);
	 glLoadIdentity();

     glActiveTexture(GL_TEXTURE0);
     GLint depthMapId = glGetUniformLocation(debugShader->programID(), "depthMap");
     glUniform1i(depthMapId, 0);
     glBindTexture( GL_TEXTURE_2D, depthRender->textureID() );

	 glBegin(GL_QUADS);

     glTexCoord2f(0,0); glVertex3f(0,0,0);
     glTexCoord2f(0, 1); glVertex3f(0, h/2, 0);
     glTexCoord2f(1, 1); glVertex3f(w/2, h/2, 0);
     glTexCoord2f(1, 0); glVertex3f(w/2, 0, 0);

	 glEnd();
}

void setupDepthRenderTarget()
{
    //depthRender = new DepthRenderTarget( window.GetWidth(), window.GetHeight() ); 
    depthRender = new DepthRenderTarget( 3200, 2400  ); 
}

