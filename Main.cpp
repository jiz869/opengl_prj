#include <cmath>
#include "Framework.h"
#include "Shader.h"
#include "VEC3.h"

#define MODEL_PATH "models/cathedral.3ds"


// Note: See the SMFL documentation for info on setting up fullscreen mode
// and using rendering settings
// http://www.sfml-dev.org/tutorials/1.6/window-window.php
sf::WindowSettings settings(24, 8, 2);
sf::Window window(sf::VideoMode(800, 600), "CS248 Rules!", sf::Style::Close, settings);
 
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

////////////////////////////////////////////////////////////////////////////////////////////
//static GLfloat eye_pos[3], eye_direction[3];
VEC3F eye_pos, eye_direction, eye_up;
VEC3F mouse_pos(0.0f, 0.0f, 0.0f);
VEC3F eye_rot(0.0f, 1.0f, 0.0f);
float eye_rot_degree=0.0f;
VEC3F eye_roty(-1.0f, 0.0f, 0.0f);
float eye_rot_degreey=0.0f;


////////////////////////////////////////////////////////////////////////////////////////////
void initEye()
{
    VEC3F ref_point(0.0f, 0.0f, 0.0f);
    eye_pos.x = 0.0f; eye_pos.y = 0.0f; eye_pos.z = 8.0f;
    eye_up.x = 0.0f; eye_up.y = 1.f; eye_up.z = 0;
    eye_direction = ref_point - eye_pos;
    eye_direction.normalize();
}

int main(int argc, char** argv) {

    initOpenGL();
    initEye();
    loadAssets();

    // Put your game loop here (i.e., render with OpenGL, update animation)
    while (window.IsOpened()) {

        handleInput();
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

   	const double aspectRatio = ((float) window.GetWidth() / (float)window.GetHeight()), fieldOfView = 45.0;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fieldOfView, aspectRatio, 1.0, 1000.0);  /* Znear and Zfar */

    glViewport(0, 0, window.GetWidth(), window.GetHeight());
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

void loadAssets() {
    // Read in an asset file, and do some post-processing.  There is much 
    // more you can do with this asset loader, including load textures.
    // More info is here:
    // http://assimp.sourceforge.net/lib_html/usage.html
    scene = importer.ReadFile(MODEL_PATH,  
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
                        e_target -= eye_roty*0.01f;
                    }else if( dx < -2 ) {
                        e_target += eye_roty*0.01f;
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
                        e_target -= eye_up*0.01f;
                    }else if( dy < -2 ) {
                        e_target += eye_up*0.01f;
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

void renderNode2(const struct aiScene *sc, const struct aiNode *nd)
{
    struct aiMatrix4x4 m = nd->mTransformation;
    unsigned int i;
    unsigned int f;
    unsigned int v;

    //update transform
    aiTransposeMatrix4(&m); //assimp matrix is row major. Need transpose for gl
    glPushMatrix();
    glMultMatrixf((float*)&m);

    //draw this node
    for(i=0; i < nd->mNumMeshes; ++i) {
        const struct aiMesh *mesh = sc->mMeshes[ nd->mMeshes[i] ];
        if( mesh->HasNormals() ) {
            glEnable(GL_LIGHTING);
        }else{
            glDisable(GL_LIGHTING);
        }

        //prepare vertices buffer
        float *vertices = (float*)malloc(3*3*sizeof(float)*mesh->mNumFaces);
        float *normals = (float*)malloc(3*3*sizeof(float)*mesh->mNumFaces);
        int vi=0;
        for(vi=0, f=0; f < mesh->mNumFaces; ++f) {
            const struct aiFace *face = &mesh->mFaces[f];
            //for(v=0; v < face->mNumIndices; ++v) {
            for(v=0; v < 3; ++v) {
                int index = face->mIndices[v];
                if(mesh->mNormals != NULL) {
                }

                //copy vetex
                vertices[vi] = mesh->mVertices[index].x;
                //vi++;
                vertices[vi] = mesh->mVertices[index].y;
                //vi++;
                vertices[vi] = mesh->mVertices[index].z;
                //vi++;
            }
        }
        //glEnableClientState(GL_VERTEX_ARRAY);
        free(vertices);
        free(normals);

        for(f=0; f < mesh->mNumFaces; ++f) {
            const struct aiFace *face = &mesh->mFaces[f];
            //GLenum face_mode;

            /*
            switch(face->mNumIndices) {
                case 1: face_mode = GL_POINTS; break;
                case 2: face_mode = GL_LINES; break;
                case 3: face_mode = GL_TRIANGLES; break;
            }
            */
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

    //draw all children
    unsigned int n;
    for(n=0; n < nd->mNumChildren; ++n) {
        renderNode2( sc, nd->mChildren[n]); 
    }

    glPopMatrix();
}

void renderNode(const struct aiScene *sc, const struct aiNode *nd)
{
    struct aiMatrix4x4 m = nd->mTransformation;
    unsigned int i;
    unsigned int f;
    unsigned int v;

    //update transform
    aiTransposeMatrix4(&m); //assimp matrix is row major. Need transpose for gl
    glPushMatrix();
    glMultMatrixf((float*)&m);

    //draw this node
    for(i=0; i < nd->mNumMeshes; ++i) {
        const struct aiMesh *mesh = sc->mMeshes[ nd->mMeshes[i] ];
        if( mesh->HasNormals() ) {
            glEnable(GL_LIGHTING);
        }else{
            glDisable(GL_LIGHTING);
        }

        for(f=0; f < mesh->mNumFaces; ++f) {
            const struct aiFace *face = &mesh->mFaces[f];
            //GLenum face_mode;

            /*
            switch(face->mNumIndices) {
                case 1: face_mode = GL_POINTS; break;
                case 2: face_mode = GL_LINES; break;
                case 3: face_mode = GL_TRIANGLES; break;
            }
            */
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

    //draw all children
    unsigned int n;
    for(n=0; n < nd->mNumChildren; ++n) {
        renderNode( sc, nd->mChildren[n]); 
    }

    glPopMatrix();
}


void renderFrame() {
    //////////////////////////////////////////////////////////////////////////
    // TODO: ADD YOUR RENDERING CODE HERE.  You may use as many .cpp files 
    // in this assignment as you wish.
    //////////////////////////////////////////////////////////////////////////
    //float tmp;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);    // uses default lighting parameters
    glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //gluLookAt(0.f, 0.f,8.f,0.f,0.f,0.0f,0.f,1.f,0.f);
    gluLookAt(eye_pos.x, eye_pos.y, eye_pos.z,
              eye_pos.x+eye_direction.x, eye_pos.y+eye_direction.y, eye_pos.z+eye_direction.z,
              eye_up.x, eye_up.y, eye_up.z );

    // scale the whole asset to fit into our view frustum 
	//tmp = scene_max.x-scene_min.x;
	//tmp = aisgl_max(scene_max.y - scene_min.y,tmp);
	//tmp = aisgl_max(scene_max.z - scene_min.z,tmp);
	//tmp = 1.f / tmp;
	//glScalef(tmp, tmp, tmp);

    //center the model

    //glRotatef(eye_rot_degreey, eye_roty.x, eye_roty.y, eye_roty.z);
    //glRotatef(eye_rot_degree, eye_rot.x, eye_rot.y, eye_rot.z);
    glRotatef(90, 0.0f, 1.0f, 0.0f);
    //glRotatef(90, 1.0f, 0.0f, 0.0f);
    //glTranslatef(-scene_center.x, -scene_center.y, -scene_center.z);    //move center to the origin
    
    //try various render methods
    //renderNode(scene, scene->mRootNode);
    renderNode2(scene, scene->mRootNode);
}








