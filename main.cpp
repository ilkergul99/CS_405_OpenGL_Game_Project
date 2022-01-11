#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <map> // yeni geldi

#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/common.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Window.hpp"
#include "GameObj3D.hpp"
#include "ShaderProgram.hpp"
#include "Shader_text.hpp"
#include "Camera.hpp"
#include "parametric-3d/Parametric3DShape.hpp"
#include "CubeData.hpp"
#include "Textures.hpp"
#include "collusion-helpers.hpp"
#include "Scene.hpp"
#include "Skybox.hpp"
#include "shader.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <iostream>
#include <vector>
using namespace std;

// Globals
float deltaTime = 0.0f;
float lastFrame = 0.0f;
int W = 800;
int H = 600;
int u_transform, u_pv, u_frame, u_light_pos, u_light_color;
int moveFront = 0, moveRight = 0;
float mouseX = 0, mouseY = 0;
int gamepoint = 0;
bool anticheck = false;
bool vfc = true;
int speedcheck = 0;
float velocityX = 0, velocityY = 0, velocityZ = 0, velocityZ_reverse = 0;

std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
{
    const auto inv = glm::inverse(proj * view);

    std::vector<glm::vec4> frustumCorners;
    for (unsigned int x = 0; x < 2; ++x)
    {
        for (unsigned int y = 0; y < 2; ++y)
        {
            for (unsigned int z = 0; z < 2; ++z)
            {
                const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
                frustumCorners.push_back(pt / pt.w);
            }
        }
    }

    return frustumCorners;
}



bool viewfrustumculling(const glm::vec4 position, vector<glm::vec4> importante, GameObj3D* object)
{
  glm::vec4 nearbottomleft = importante[0];
  glm::vec4 nearbottomright = importante[4];
  glm::vec4 neartopleft = importante[2];
  glm::vec4 neartopright = importante[6];

  glm::vec4 farbottomleft = importante[1];
  glm::vec4 farbottomright = importante[5];
  glm::vec4 fartopleft = importante[3];
  glm::vec4 fartopright = importante[7];

  glm::vec4 middlepointoffarplane = (farbottomleft + fartopright)*0.5f;
  glm::vec4 middlepointofnearplane = (nearbottomleft + neartopright)*0.5f;

  glm::vec4 middlepointofnearbottomplane = (nearbottomleft + nearbottomright)*0.5f;
  glm::vec4 middlepointofneartopplane = (neartopleft + neartopright)*0.5f;
  glm::vec4 middlepointoffarbottomplane = (farbottomleft + farbottomright)*0.5f;
  glm::vec4 middlepointoffartopplane = (fartopleft + fartopright)*0.5f;



  //For left face
  float leftface_z_size = abs(farbottomleft.z - nearbottomleft.z);
  float leftface_x_size = abs(farbottomleft.x - nearbottomleft.x);
  // For top face
  float topface_z_size = abs(middlepointofneartopplane.z - middlepointoffartopplane.z);
  float topface_y_size = abs(middlepointofneartopplane.y - middlepointoffartopplane.y);
  // For right face
  float rightface_z_size = abs(farbottomright.z - nearbottomright.z);
  float rightface_x_size = abs(farbottomright.x- nearbottomright.x);
  // For bottom face
  float bottomface_z_size = abs(middlepointofneartopplane.z - middlepointoffartopplane.z);
  float bottomface_y_size = abs(middlepointofneartopplane.y - middlepointoffartopplane.y);

  // if we go in +z direction
  if(nearbottomleft.z < farbottomleft.z)
  {
    glm::vec4 minpointofnearplane = middlepointofneartopplane.z == min(middlepointofneartopplane.z, middlepointofnearbottomplane.z) ? middlepointofneartopplane : middlepointofnearbottomplane;
    glm::vec4 maxpointoffarplane = middlepointoffartopplane.z == max(middlepointoffartopplane.z, middlepointoffarbottomplane.z) ? middlepointoffartopplane: middlepointoffarbottomplane;

    //front
    if(object->position().z + 5 < position.z)
    {
      return false;
    }
    
    if(object->position().z + 5 < minpointofnearplane.z )
    {
      if(object->position().y < middlepointofnearplane.y)
      {
        if(object->position().x > nearbottomleft.x)
        {
          return false;
        }
        if(object->position().y < nearbottomleft.y)
        {
          return false;
        }
        if(object->position().x < nearbottomright.x)
        {
          return false;
        }
        if(object->position().y < nearbottomright.y)
        {
          return false;
        }
      }
      else
      {
        if(object->position().x > neartopleft.x)
        {
          return false;
        }
        if(object->position().y > neartopleft.y)
        {
          return false;
        }
        if(object->position().x < neartopright.x)
        {
          return false;
        }
        if(object->position().y > neartopright.y)
        {
          return false;
        }
      }
    }
    // end of front
    if(object->position().z  > maxpointoffarplane.z )
    {
      return false;
    }
    // end of far
    float ratio1 = abs(object->position().z -  nearbottomleft.z)/ leftface_z_size;
    float ratio2 = abs(object->position().z -  middlepointofneartopplane.z)/ topface_z_size;
    float ratio3 = abs(object->position().z -  nearbottomleft.z)/ rightface_z_size;
    float ratio4 = abs(object->position().z -  middlepointofnearbottomplane.z)/ bottomface_z_size;
    if(object->position().x > (nearbottomleft.x + ratio1*leftface_x_size))
    {
      return false;
    }
    if(object->position().y > (middlepointofneartopplane.y + ratio2*topface_y_size))
    {
      return false;
    }
    if(object->position().y < (middlepointofnearbottomplane.y - ratio4*bottomface_y_size))
    {
      return false;
    }
    if(object->position().x < (nearbottomright.x - ratio3*rightface_x_size))
    {
      return false;
    }

    return true;

  }
  // if we go in -z direction
  else{
    glm::vec4 minpointofnearplane = middlepointofneartopplane.z == min(middlepointofneartopplane.z, middlepointofnearbottomplane.z) ? middlepointofneartopplane : middlepointofnearbottomplane;
    glm::vec4 maxpointoffarplane = middlepointoffartopplane.z == max(middlepointoffartopplane.z, middlepointoffarbottomplane.z) ? middlepointoffartopplane: middlepointoffarbottomplane;
    cout << object->position().z << endl;
    cout << position.z << endl;
    cout << minpointofnearplane.z << endl;
    //front
    if(object->position().z > position.z)
    {
        return false;
    }
    if(object->position().z  > minpointofnearplane.z )
    {
      if(object->position().y < middlepointofnearplane.y)
      {
        if(object->position().x < nearbottomleft.x)
        {

          return false;
        }
        if(object->position().y < nearbottomleft.y)
        {

          return false;
        }
        if(object->position().x > nearbottomright.x)
        {

          return false;
        }
        if(object->position().y < nearbottomright.y)
        {

          return false;
        }
      }
      else
      {
        if(object->position().x < neartopleft.x)
        {

          return false;
        }
        if(object->position().y > neartopleft.y)
        {

            return false;
        }
        if(object->position().x > neartopright.x)
        {

            return false;
        }
        if(object->position().y > neartopright.y)
        {


            return false;
        }
      }
    }
    // end of front
    if(object->position().z - 100 < maxpointoffarplane.z)
    {
        cout << "step 5" << endl;
        return false;
    }
    // end of far
    float ratio1 = abs(object->position().z -  nearbottomleft.z)/ leftface_z_size;
    float ratio2 = abs(object->position().z -  middlepointofneartopplane.z)/ topface_z_size;
    float ratio3 = abs(object->position().z -  nearbottomleft.z)/ rightface_z_size;
    float ratio4 = abs(object->position().z -  middlepointofnearbottomplane.z)/ bottomface_z_size;
    if(object->position().x < (nearbottomleft.x - ratio1*leftface_x_size))
    {
        cout << "step 1" << endl;
        return false;
    }
    if(object->position().y > (middlepointofneartopplane.y + ratio2*topface_y_size))
    {
        cout << "step 2" << endl;
        return false;
    }
    if(object->position().y < (middlepointofnearbottomplane.y - ratio4*bottomface_y_size))
    {
        /*
        cout << "Second argument " << middlepointofnearbottomplane.y - ratio4*bottomface_y_size << endl;
        cout << "components are " << middlepointofnearbottomplane.y << " " << ratio3*bottomface_y_size << endl;
        cout << bottomface_y_size << endl;
        cout << nearbottomleft.y << endl;
        cout << farbottomleft.y << endl;
        cout << ratio3<< endl;
        cout << "First argument baby is " << object->position().y << endl;
        cout << "step 3" << endl;
        */
        return false;
    }
    if(object->position().x > (nearbottomright.x + ratio3*rightface_x_size))
    {
        cout << "step 4" << endl;
        return false;
    }

    return true;

  }
}
//yeni geldi
struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;


void keyCallback(GLFWwindow *_, int key, int scancode, int action, int mods)
{
    if (glfwGetKey(_, GLFW_KEY_Q) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(_, true);
    }

    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        moveFront = 1;
        velocityZ += 0.1;

    }
    else if (key == GLFW_KEY_W && action == GLFW_RELEASE)
    {

        moveFront = 0;
        speedcheck = 1;

    }
    if (key == GLFW_KEY_V && action == GLFW_PRESS)
    {
        vfc = false;
    }
    
    if (key == GLFW_KEY_B && action == GLFW_PRESS)
    {
        vfc = true;
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
        moveFront = -1;
        speedcheck = 2;
        velocityZ_reverse -= 0.1;
    }
    else if (key == GLFW_KEY_S && action == GLFW_RELEASE)
    {
        moveFront = 0;

    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        moveRight = 1;
    }
    else if (key == GLFW_KEY_D && action == GLFW_RELEASE)
    {
        moveRight = 0;
    }
    if (key == GLFW_KEY_A && action == GLFW_PRESS)
    {
        moveRight = -1;
        gamepoint--;
    }
    else if (key == GLFW_KEY_A && action == GLFW_RELEASE)
    {
        moveRight = 0;
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        velocityY = 4;
        gamepoint += 10;
    }
}

static void cursorPositionCallback(GLFWwindow *_, double x, double y)
{
    mouseX = 2.0 * ((float)x / Window::width) - 1;
    mouseY = 2.0 * (1 - ((float)y / Window::height)) - 1;
}



int main()
{
    // init window
    Window::init(800, 600, "CS405 Final Demo");

    glfwSetKeyCallback(Window::window, keyCallback);
    glfwSetCursorPosCallback(Window::window, cursorPositionCallback);

    /* Configure OpenGL */
    glClearColor(0.6f, 0.9f, 0.9f, 1.0f);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    //OpenGL State
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //text
    Shader_text shadertext("../includes/textshader/textvertex.vert", "../includes/textshader/textfrag.frag");
    //glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(1000), 0.0f, static_cast<float>(800));
    shadertext.use();

    //text finished
    // init objects
    Model3D sphereModel = Parametric3DShape::generate(ParametricLine::halfCircle, 50, 50);
    Model3D sphereModel2 = Parametric3DShape::generate(ParametricLine::halfCircle, 100, 100);
    Model3D sphereModel3 = Parametric3DShape::generate(ParametricLine::halfCircle, 200, 200);
    Model3D spikeModel = Parametric3DShape::generate(ParametricLine::spikes, 50, 50);
    Model3D spikeModel2 = Parametric3DShape::generate(ParametricLine::spikes, 100, 100);
    Model3D spikeModel3 = Parametric3DShape::generate(ParametricLine::spikes, 200, 200);
    Model3D cubeModel(CubeData::positions, CubeData::normals, CubeData::uvs, CubeData::indices);
    Model3D cubeModel2(CubeData::positions2, CubeData::normals2, CubeData::uvs2, CubeData::indices2);
    Model3D cubeModel3(CubeData::positions3, CubeData::normals3, CubeData::uvs3, CubeData::indices3);

    GameObj3D hero(sphereModel);
    hero.translate(0, 50.0, 13.0);
    hero.scale(1, 1, 1);
    hero.textureId = 2;

    GameObj3D antihero1(sphereModel);
    antihero1.translate(0, 70.0, -5.0);
    antihero1.scale(1, 1, 1);
    antihero1.textureId = 2;

    GameObj3D antihero2(sphereModel);
    antihero2.translate(5, 50.0, 28.0);
    antihero2.scale(1, 1, 1);
    antihero2.textureId = 2;

    GameObj3D antihero3(sphereModel);
    antihero3.translate(-5, 80.0, -10.0);
    antihero3.scale(1, 1, 1);
    antihero2.textureId = 2;

    GameObj3D antihero4(sphereModel);
    antihero4.translate(-5, 100.0, -30.0);
    antihero4.scale(1, 1, 1);
    antihero4.textureId = 2;

    GameObj3D antihero5(sphereModel);
    antihero5.translate(6, 100.0, -8.0);
    antihero5.scale(1, 1, 1);
    antihero5.textureId = 2;

    GameObj3D antihero6(sphereModel);
    antihero6.translate(27, 10.0, -105.0);
    antihero6.scale(2.5, 2, 1);
    antihero6.textureId = 8;
    antihero6.hasGravity = false;

    GameObj3D antihero7(sphereModel);
    antihero7.translate(27, 13.0, -105.0);
    antihero7.scale(2.5, 2, 1);
    antihero7.textureId = 8;
    antihero7.hasGravity = false;

    GameObj3D antihero8(sphereModel);
    antihero8.translate(29, 11.5, -105.0);
    antihero8.scale(2.5, 2, 1);
    antihero8.textureId = 8;
    antihero8.hasGravity = false;

    GameObj3D antihero9(sphereModel);
    antihero9.translate(25, 11.5, -105.0);
    antihero9.scale(2.5, 2, 1);
    antihero9.textureId = 8;
    antihero9.hasGravity = false;

    GameObj3D antihero10(sphereModel);
    antihero10.translate(35, 10.0, -65.0);
    antihero10.scale(2.5, 2, 1);
    antihero10.textureId = 8;
    antihero10.hasGravity = false;

    GameObj3D antihero11(sphereModel);
    antihero11.translate(35, 13.0, -65.0);
    antihero11.scale(2.5, 2, 1);
    antihero11.textureId = 8;
    antihero11.hasGravity = false;

    GameObj3D antihero12(sphereModel);
    antihero12.translate(33, 11.5, -65.0);
    antihero12.scale(2.5, 2, 1);
    antihero12.textureId = 8;
    antihero12.hasGravity = false;

    GameObj3D antihero13(sphereModel);
    antihero13.translate(37, 11.5, -65.0);
    antihero13.scale(2.5, 2, 1);
    antihero13.textureId = 8;
    antihero13.hasGravity = false;

    GameObj3D antihero14(sphereModel);
    antihero14.translate(35, 10.0, -205.0);
    antihero14.scale(2.5, 2, 1);
    antihero14.textureId = 8;
    antihero14.hasGravity = false;

    GameObj3D antihero15(sphereModel);
    antihero15.translate(35, 13.0, -205.0);
    antihero15.scale(2.5, 2, 1);
    antihero15.textureId = 8;
    antihero15.hasGravity = false;

    GameObj3D antihero16(sphereModel);
    antihero16.translate(33, 11.5, -205.0);
    antihero16.scale(2.5, 2, 1);
    antihero16.textureId = 8;
    antihero16.hasGravity = false;

    GameObj3D antihero17(sphereModel);
    antihero17.translate(37, 11.5, -205.0);
    antihero17.scale(2.5, 2, 1);
    antihero17.textureId = 8;
    antihero17.hasGravity = false;

    GameObj3D antihero18(sphereModel);
    antihero18.translate(35, 9.0, -175.0);
    antihero18.scale(3.5, 3, 1);
    antihero18.textureId = 8;
    antihero18.hasGravity = false;

    GameObj3D antihero19(sphereModel);
    antihero19.translate(35, 14.0, -175.0);
    antihero19.scale(3.5, 3, 1);
    antihero19.textureId = 8;
    antihero19.hasGravity = false;

    GameObj3D antihero20(sphereModel);
    antihero20.translate(32, 11.5, -175.0);
    antihero20.scale(3.5, 3, 1);
    antihero20.textureId = 8;
    antihero20.hasGravity = false;

    GameObj3D antihero21(sphereModel);
    antihero21.translate(39, 11.5, -175.0);
    antihero21.scale(3.5, 3, 1);
    antihero21.textureId = 8;
    antihero21.hasGravity = false;
    

    

    scene.push_back(&hero);
    scene.push_back(&antihero1);
    scene.push_back(&antihero2);
    scene.push_back(&antihero3);
    scene.push_back(&antihero4);
    scene.push_back(&antihero5);
    scene.push_back(&antihero6);
    scene.push_back(&antihero7);
    scene.push_back(&antihero8);
    scene.push_back(&antihero9);
    scene.push_back(&antihero10);
    scene.push_back(&antihero11);
    scene.push_back(&antihero12);
    scene.push_back(&antihero13);
    scene.push_back(&antihero14);
    scene.push_back(&antihero15);
    scene.push_back(&antihero16);
    scene.push_back(&antihero17);
    scene.push_back(&antihero18);
    scene.push_back(&antihero19);
    scene.push_back(&antihero20);
    scene.push_back(&antihero21);

    GameObj3D* mine = new GameObj3D(cubeModel);
    scene.push_back(mine);

    GameObj3D* floor = new GameObj3D(cubeModel);
    scene.push_back(floor);
    GameObj3D* secondfloor = new GameObj3D(cubeModel);
    scene.push_back(secondfloor);
    GameObj3D* thirdfloor = new GameObj3D(cubeModel);
    scene.push_back(thirdfloor);
    GameObj3D* finishfloor = new GameObj3D(cubeModel);
    scene.push_back(finishfloor);

    GameObj3D* fire1 = new GameObj3D(sphereModel3);
    fire1 ->translate(-10, 10, -5);
    fire1 ->scale(0.18, 0.18, 0.08);
    fire1->rotate(0, 0, 0);
    fire1  -> textureId = 9;
    fire1 -> hasGravity = false;
    scene.push_back(fire1);
    
    GameObj3D* fire2 = new GameObj3D(sphereModel3);
    fire2 ->translate(-10.16, 10, -5);
    fire2 ->scale(0.18, 0.18, 0.08);
    fire2 ->rotate(0, 0, 0);
    fire2  -> textureId = 9;
    fire2 -> hasGravity = false;
    scene.push_back(fire2);
    
    GameObj3D* fire3 = new GameObj3D(sphereModel3);
    fire3 ->translate(-10.32, 10, -5);
    fire3 ->scale(0.18, 0.18, 0.08);
    fire3 ->rotate(0, 0, 0);
    fire3  -> textureId = 9;
    fire3 -> hasGravity = false;
    scene.push_back(fire3);
    
    GameObj3D* fire4 = new GameObj3D(sphereModel3);
    fire4 ->translate(-10.48, 10, -5);
    fire4 ->scale(0.18, 0.18, 0.08);
    fire4 ->rotate(0, 0, 0);
    fire4  -> textureId = 9;
    fire4 -> hasGravity = false;
    scene.push_back(fire4);
    
    GameObj3D* fire5 = new GameObj3D(sphereModel3);
    fire5 ->translate(-10.08, 9.85, -5);
    fire5 ->scale(0.18, 0.18, 0.08);
    fire5 ->rotate(0, 0, 0);
    fire5  -> textureId = 9;
    fire5 -> hasGravity = false;
    scene.push_back(fire5);
    
    GameObj3D* fire6 = new GameObj3D(sphereModel3);
    fire6 ->translate(-10.24, 9.85, -5);
    fire6 ->scale(0.18, 0.18, 0.08);
    fire6 ->rotate(0, 0, 0);
    fire6  -> textureId = 9;
    fire6 -> hasGravity = false;
    scene.push_back(fire6);
    
    GameObj3D* fire7 = new GameObj3D(sphereModel3);
    fire7 ->translate(-10.40, 9.85, -5);
    fire7 ->scale(0.18, 0.18, 0.08);
    fire7 ->rotate(0, 0, 0);
    fire7  -> textureId = 9;
    fire7 -> hasGravity = false;
    scene.push_back(fire7);
    
    GameObj3D* fire8 = new GameObj3D(sphereModel3);
    fire8 ->translate(-10.08, 10.15, -5);
    fire8 ->scale(0.18, 0.18, 0.08);
    fire8 ->rotate(0, 0, 0);
    fire8  -> textureId = 9;
    fire8 -> hasGravity = false;
    scene.push_back(fire8);
    
    GameObj3D* fire9 = new GameObj3D(sphereModel3);
    fire9 ->translate(-10.24, 10.15, -5);
    fire9 ->scale(0.18, 0.18, 0.08);
    fire9 ->rotate(0, 0, 0);
    fire9  -> textureId = 9;
    fire9 -> hasGravity = false;
    scene.push_back(fire9);
    
    GameObj3D* fire10 = new GameObj3D(sphereModel3);
    fire10 ->translate(-10.40, 10.15, -5);
    fire10 ->scale(0.18, 0.18, 0.08);
    fire10 ->rotate(0, 0, 0);
    fire10  -> textureId = 9;
    fire10 -> hasGravity = false;
    scene.push_back(fire10);
    
    GameObj3D* fire30 = new GameObj3D(sphereModel3);
    fire30 ->translate(-10.50, 10.15, -5);
    fire30 ->scale(0.18, 0.18, 0.08);
    fire30 ->rotate(0, 0, 0);
    fire30  -> textureId = 9;
    fire30 -> hasGravity = false;
    scene.push_back(fire30);
    
    GameObj3D* fire31 = new GameObj3D(sphereModel3);
    fire31 ->translate(-10.04, 10.15, -5);
    fire31 ->scale(0.18, 0.18, 0.08);
    fire31 ->rotate(0, 0, 0);
    fire31  -> textureId = 9;
    fire31 -> hasGravity = false;
    scene.push_back(fire31);
    
    GameObj3D* fire11 = new GameObj3D(sphereModel3);
    fire11 ->translate(-10.08, 10.30, -5);
    fire11 ->scale(0.18, 0.18, 0.08);
    fire11 ->rotate(0, 0, 0);
    fire11  -> textureId = 9;
    fire11 -> hasGravity = false;
    scene.push_back(fire11);
    
    GameObj3D* fire12 = new GameObj3D(sphereModel3);
    fire12 ->translate(-10.24, 10.30, -5);
    fire12 ->scale(0.18, 0.18, 0.08);
    fire12 ->rotate(0, 0, 0);
    fire12  -> textureId = 9;
    fire12 -> hasGravity = false;
    scene.push_back(fire12);
    
    GameObj3D* fire13 = new GameObj3D(sphereModel3);
    fire13 ->translate(-10.40, 10.30, -5);
    fire13 ->scale(0.18, 0.18, 0.08);
    fire13 ->rotate(0, 0, 0);
    fire13  -> textureId = 9;
    fire13 -> hasGravity = false;
    scene.push_back(fire13);
    
    GameObj3D* fire14 = new GameObj3D(sphereModel3);
    fire14 ->translate(-10.08, 10.45, -5);
    fire14 ->scale(.18, 0.18, 0.08);
    fire14 ->rotate(0, 0, 0);
    fire14  -> textureId = 9;
    fire14 -> hasGravity = false;
    scene.push_back(fire14);
    
    GameObj3D* fire15 = new GameObj3D(sphereModel3);
    fire15 ->translate(-10.24, 10.45, -5);
    fire15 ->scale(0.18, 0.18, 0.08);
    fire15 ->rotate(0, 0, 0);
    fire15  -> textureId = 9;
    fire15 -> hasGravity = false;
    scene.push_back(fire15);
    
    GameObj3D* fire16 = new GameObj3D(sphereModel3);
    fire16 ->translate(-10.40, 10.45, -5);
    fire16 ->scale(0.18, 0.18, 0.08);
    fire16 ->rotate(0, 0, 0);
    fire16  -> textureId = 9;
    fire16 -> hasGravity = false;
    scene.push_back(fire16);
    
    
    GameObj3D* fire20 = new GameObj3D(sphereModel3);
    fire20 ->translate(-10.16, 10.58, -5);
    fire20 ->scale(0.18, 0.18, 0.08);
    fire20 ->rotate(0, 0, 0);
    fire20 -> textureId = 9;
    fire20 -> hasGravity = false;
    scene.push_back(fire20);
    
    GameObj3D* fire21 = new GameObj3D(sphereModel3);
    fire21 ->translate(-10.32, 10.58, -5);
    fire21 ->scale(0.18, 0.18, 0.08);
    fire21 ->rotate(0, 0, 0);
    fire21  -> textureId = 9;
    fire21 -> hasGravity = false;
    scene.push_back(fire21);
    
    GameObj3D* fire22 = new GameObj3D(sphereModel3);
    fire22 ->translate(-10.16, 10.71, -5);
    fire22 ->scale(0.18, 0.18, 0.08);
    fire22 ->rotate(0, 0, 0);
    fire22 -> textureId = 9;
    fire22 -> hasGravity = false;
    scene.push_back(fire22);
    
    GameObj3D* fire23 = new GameObj3D(sphereModel3);
    fire23 ->translate(-10.32, 10.71, -5);
    fire23 ->scale(0.18, 0.18, 0.08);
    fire23 ->rotate(0, 0, 0);
    fire23  -> textureId = 9;
    fire23 -> hasGravity = false;
    scene.push_back(fire23);
    
    GameObj3D* fire24 = new GameObj3D(sphereModel3);
    fire24 ->translate(-10.24, 10.84, -5);
    fire24 ->scale(0.18, 0.18, 0.08);
    fire24 ->rotate(0, 0, 0);
    fire24  -> textureId = 9;
    fire24 -> hasGravity = false;
    scene.push_back(fire24);
    
    GameObj3D* fire25 = new GameObj3D(sphereModel3);
    fire25 ->translate(-10.16, 9.70, -5);
    fire25 ->scale(0.18, 0.18, 0.08);
    fire25 ->rotate(0, 0, 0);
    fire25  -> textureId = 9;
    fire25 -> hasGravity = false;
    scene.push_back(fire25);
    
    GameObj3D* fire26 = new GameObj3D(sphereModel3);
    fire26 ->translate(-10.40, 9.70, -5);
    fire26 ->scale(0.18, 0.18, 0.08);
    fire26 ->rotate(0, 0, 0);
    fire26  -> textureId = 9;
    fire26 -> hasGravity = false;
    scene.push_back(fire26);
    
    GameObj3D* fire27 = new GameObj3D(sphereModel3);
    fire27 ->translate(-10.24, 9.65, -5);
    fire27 ->scale(0.18, 0.18, 0.08);
    fire27 ->rotate(0, 0, 0);
    fire27  -> textureId = 9;
    fire27 -> hasGravity = false;
    scene.push_back(fire27);
    
    GameObj3D leftcloud1_1(sphereModel);
    leftcloud1_1.translate(-27, 10.0, -105.0);
    leftcloud1_1.scale(2.5, 2, 1);
    leftcloud1_1.textureId = 8;
    leftcloud1_1.hasGravity = false;

    GameObj3D leftcloud1_2(sphereModel);
    leftcloud1_2.translate(-27, 13.0, -105.0);
    leftcloud1_2.scale(2.5, 2, 1);
    leftcloud1_2.textureId = 8;
    leftcloud1_2.hasGravity = false;

    GameObj3D leftcloud1_3(sphereModel);
    leftcloud1_3.translate(-29, 11.5, -105.0);
    leftcloud1_3.scale(2.5, 2, 1);
    leftcloud1_3.textureId = 8;
    leftcloud1_3.hasGravity = false;

    GameObj3D leftcloud1_4(sphereModel);
    leftcloud1_4.translate(-25, 11.5, -105.0);
    leftcloud1_4.scale(2.5, 2, 1);
    leftcloud1_4.textureId = 8;
    leftcloud1_4.hasGravity = false;


    GameObj3D leftcloud2_1(sphereModel);
    leftcloud2_1.translate(-35, 10.0, -105.0);
    leftcloud2_1.scale(2.5, 2, 1);
    leftcloud2_1.textureId = 8;
    leftcloud2_1.hasGravity = false;

    GameObj3D leftcloud2_2(sphereModel);
    leftcloud2_2.translate(-35, 13.0, -105.0);
    leftcloud2_2.scale(2.5, 2, 1);
    leftcloud2_2.textureId = 8;
    leftcloud2_2.hasGravity = false;

    GameObj3D leftcloud2_3(sphereModel);
    leftcloud2_3.translate(-33, 11.5, -105.0);
    leftcloud2_3.scale(2.5, 2, 1);
    leftcloud2_3.textureId = 8;
    leftcloud2_3.hasGravity = false;

    GameObj3D leftcloud2_4(sphereModel);
    leftcloud2_4.translate(-37, 11.5, -105.0);
    leftcloud2_4.scale(2.5, 2, 1);
    leftcloud2_4.textureId = 8;
    leftcloud2_4.hasGravity = false;
    
    GameObj3D leftcloud3_1(sphereModel);
    leftcloud3_1.translate(-35, 10.0, -205.0);
    leftcloud3_1.scale(2.5, 2, 1);
    leftcloud3_1.textureId = 8;
    leftcloud3_1.hasGravity = false;

    GameObj3D leftcloud3_2(sphereModel);
    leftcloud3_2.translate(-35, 13.0, -205.0);
    leftcloud3_1.scale(2.5, 2, 1);
    leftcloud3_2.textureId = 8;
    leftcloud3_2.hasGravity = false;

    GameObj3D leftcloud3_3(sphereModel);
    leftcloud3_3.translate(-33, 11.5, -205.0);
    leftcloud3_3.scale(2.5, 2, 1);
    leftcloud3_3.textureId = 8;
    leftcloud3_3.hasGravity = false;

    GameObj3D leftcloud3_4(sphereModel);
    leftcloud3_4.translate(-37, 11.5, -205.0);
    leftcloud3_4.scale(2.5, 2, 1);
    leftcloud3_4.textureId = 8;
    leftcloud3_4.hasGravity = false;
    
    
    GameObj3D leftcloud4_1(sphereModel);
    leftcloud4_1.translate(-35, 9.0, -175.0);
    leftcloud4_1.scale(3.5, 3, 1);
    leftcloud4_1.textureId = 8;
    leftcloud4_1.hasGravity = false;

    GameObj3D leftcloud4_2(sphereModel);
    leftcloud4_2.translate(-35, 14.0, -175.0);
    leftcloud4_2.scale(3.5, 3, 1);
    leftcloud4_2.textureId = 8;
    leftcloud4_2.hasGravity = false;

    GameObj3D leftcloud4_3(sphereModel);
    leftcloud4_3.translate(-32, 11.5, -175.0);
    leftcloud4_3.scale(3.5, 3, 1);
    leftcloud4_3.textureId = 8;
    leftcloud4_3.hasGravity = false;

    GameObj3D leftcloud4_4(sphereModel);
    leftcloud4_4.translate(-39, 11.5, -175.0);
    leftcloud4_4.scale(3.5, 3, 1);
    leftcloud4_4.textureId = 8;
    leftcloud4_4.hasGravity = false;
    
    
    
    GameObj3D leftwheel1(spikeModel3);
    leftwheel1.rotate(180,90,270);
    leftwheel1.translate(-9.25, 0, 7.5);
    leftwheel1.scale(2.5, 2.5, 2.5);
    leftwheel1.textureId = 0;
    leftwheel1.hasGravity = false;

    GameObj3D leftwheel2(spikeModel3);
    leftwheel2.rotate(180,90,270);
    leftwheel2.translate(-9.25, 0, 2.5);
    leftwheel2.scale(2.5, 2.5, 2.5);
    leftwheel2.textureId = 0;
    leftwheel2.hasGravity = false;

    GameObj3D leftwheel3(spikeModel3);
    leftwheel3.rotate(180,90,270);
    leftwheel3.translate(-9.25, 0, -2.5);
    leftwheel3.scale(2.5, 2.5, 2.5);
    leftwheel3.textureId = 0;
    leftwheel3.hasGravity = false;

    GameObj3D leftwheel4(spikeModel3);
    leftwheel4.rotate(180,90,270);
    leftwheel4.translate(-9.25, 0, -7.5);
    leftwheel4.scale(2.5, 2.5, 2.5);
    leftwheel4.textureId = 0;
    leftwheel4.hasGravity = false;

    GameObj3D leftwheel5(spikeModel3);
    leftwheel5.rotate(180,90,270);
    leftwheel5.translate(-9.25, 0, -12.5);
    leftwheel5.scale(2.5, 2.5, 2.5);
    leftwheel5.textureId = 0;
    leftwheel5.hasGravity = false;

    GameObj3D leftwheel6(spikeModel3);
    leftwheel6.rotate(180,90,270);
    leftwheel6.translate(-9.25, 0, -17.5);
    leftwheel6.scale(2.5, 2.5, 2.5);
    leftwheel6.textureId = 0;
    leftwheel6.hasGravity = false;
    scene.push_back(&leftwheel6);

    GameObj3D leftwheel7(spikeModel3);
    leftwheel7.rotate(180,90,270);
    leftwheel7.translate(-9.25, 0, -22.5);
    leftwheel7.scale(2.5, 2.5, 2.5);
    leftwheel7.textureId = 0;
    leftwheel7.hasGravity = false;

    GameObj3D leftwheel8(spikeModel3);
    leftwheel8.rotate(180,90,270);
    leftwheel8.translate(-9.25, 0, -27.5);
    leftwheel8.scale(2.5, 2.5, 2.5);
    leftwheel8.textureId = 0;
    leftwheel8.hasGravity = false;

    GameObj3D leftwheel9(spikeModel3);
    leftwheel9.rotate(180,90,270);
    leftwheel9.translate(-9.25, 0, -32.5);
    leftwheel9.scale(2.5, 2.5, 2.5);
    leftwheel9.textureId = 0;
    leftwheel9.hasGravity = false;

    GameObj3D leftwheel10(spikeModel3);
    leftwheel10.rotate(180,90,270);
    leftwheel10.translate(-9.25, 0, -37.5);
    leftwheel10.scale(2.5, 2.5, 2.5);
    leftwheel10.textureId = 0;
    leftwheel10.hasGravity = false;

    GameObj3D leftwheel11(spikeModel3);
    leftwheel11.rotate(180,90,270);
    leftwheel11.translate(-9.25, 0, -42.5);
    leftwheel11.scale(2.5, 2.5, 2.5);
    leftwheel11.textureId = 0;
    leftwheel11.hasGravity = false;

    GameObj3D leftwheel12(spikeModel3);
    leftwheel12.rotate(180,90,270);
    leftwheel12.translate(-9.25, 0, -47.5);
    leftwheel12.scale(2.5, 2.5, 2.5);
    leftwheel12.textureId = 0;
    leftwheel12.hasGravity = false;

    GameObj3D leftwheel13(spikeModel3);
    leftwheel13.rotate(180,90,270);
    leftwheel13.translate(-9.25, 0, -52.5);
    leftwheel13.scale(2.5, 2.5, 2.5);
    leftwheel13.textureId = 0;
    leftwheel13.hasGravity = false;

    GameObj3D leftwheel14(spikeModel3);
    leftwheel14.rotate(180,90,270);
    leftwheel14.translate(-9.25, 0, -57.5);
    leftwheel14.scale(2.5, 2.5, 2.5);
    leftwheel14.textureId = 0;
    leftwheel14.hasGravity = false;

    GameObj3D leftwheel15(spikeModel3);
    leftwheel15.rotate(180,90,270);
    leftwheel15.translate(-9.25, 0, -62.5);
    leftwheel15.scale(2.5, 2.5, 2.5);
    leftwheel15.textureId = 0;
    leftwheel15.hasGravity = false;

    GameObj3D leftwheel16(spikeModel3);
    leftwheel16.rotate(180,90,270);
    leftwheel16.translate(-9.25, 0, -67.5);
    leftwheel16.scale(2.5, 2.5, 2.5);
    leftwheel16.textureId = 0;
    leftwheel16.hasGravity = false;

    GameObj3D leftwheel17(spikeModel3);
    leftwheel17.rotate(180,90,270);
    leftwheel17.translate(-9.25, 0, -72.5);
    leftwheel17.scale(2.5, 2.5, 2.5);
    leftwheel17.textureId = 0;
    leftwheel17.hasGravity = false;

    GameObj3D leftwheel18(spikeModel3);
    leftwheel18.rotate(180,90,270);
    leftwheel18.translate(-9.25, 0, -77.5);
    leftwheel18.scale(2.5, 2.5, 2.5);
    leftwheel18.textureId = 0;
    leftwheel18.hasGravity = false;

    GameObj3D leftwheel19(spikeModel3);
    leftwheel19.rotate(180,90,270);
    leftwheel19.translate(-9.25, 0, -82.5);
    leftwheel19.scale(2.5, 2.5, 2.5);
    leftwheel19.textureId = 0;
    leftwheel19.hasGravity = false;

    GameObj3D leftwheel20(spikeModel3);
    leftwheel20.rotate(180,90,270);
    leftwheel20.translate(-9.25, 0, -87.5);
    leftwheel20.scale(2.5, 2.5, 2.5);
    leftwheel20.textureId = 0;
    leftwheel20.hasGravity = false;

    GameObj3D leftwheel21(spikeModel3);
    leftwheel21.rotate(180,90,270);
    leftwheel21.translate(-9.25, 0, -92.5);
    leftwheel21.scale(2.5, 2.5, 2.5);
    leftwheel21.textureId = 0;
    leftwheel21.hasGravity = false;

    GameObj3D leftwheel22(spikeModel3);
    leftwheel22.rotate(180,90,270);
    leftwheel22.translate(-9.25, 0, -97.5);
    leftwheel22.scale(2.5, 2.5, 2.5);
    leftwheel22.textureId = 0;
    leftwheel22.hasGravity = false;

    GameObj3D leftwheel23(spikeModel3);
    leftwheel23.rotate(180,90,270);
    leftwheel23.translate(-9.25, 0, -102.5);
    leftwheel23.scale(2.5, 2.5, 2.5);
    leftwheel23.textureId = 0;
    leftwheel23.hasGravity = false;

    GameObj3D leftwheel24(spikeModel3);
    leftwheel24.rotate(180,90,270);
    leftwheel24.translate(-9.25, 0, -107.5);
    leftwheel24.scale(2.5, 2.5, 2.5);
    leftwheel24.textureId = 0;
    leftwheel24.hasGravity = false;

    GameObj3D leftwheel25(spikeModel3);
    leftwheel25.rotate(180,90,270);
    leftwheel25.translate(-9.25, 0, -112.5);
    leftwheel25.scale(2.5, 2.5, 2.5);
    leftwheel25.textureId = 0;
    leftwheel25.hasGravity = false;

    GameObj3D leftwheel26(spikeModel3);
    leftwheel26.rotate(180,90,270);
    leftwheel26.translate(-9.25, 0, -117.5);
    leftwheel26.scale(2.5, 2.5, 2.5);
    leftwheel26.textureId = 0;
    leftwheel26.hasGravity = false;

    GameObj3D leftwheel27(spikeModel3);
    leftwheel27.rotate(180,90,270);
    leftwheel27.translate(-9.25, 0, -122.5);
    leftwheel27.scale(2.5, 2.5, 2.5);
    leftwheel27.textureId = 0;
    leftwheel27.hasGravity = false;

    GameObj3D leftwheel28(spikeModel3);
    leftwheel28.rotate(180,90,270);
    leftwheel28.translate(-9.25, 0, -127.5);
    leftwheel28.scale(2.5, 2.5, 2.5);
    leftwheel28.textureId = 0;
    leftwheel28.hasGravity = false;
    
    //RIGHT WHEELS
        
    GameObj3D rightwheel28(spikeModel3);
    rightwheel28.rotate(180,90,270);
    rightwheel28.translate(9.25, 0, -127.5);
    rightwheel28.scale(2.5, 2.5, 2.5);
    rightwheel28.textureId = 0;
    rightwheel28.hasGravity = false;

    GameObj3D rightwheel27(spikeModel3);
    rightwheel27.rotate(180,90,270);
    rightwheel27.translate(9.25, 0, -122.5);
    rightwheel27.scale(2.5, 2.5, 2.5);
    rightwheel27.textureId = 0;
    rightwheel27.hasGravity = false;


    GameObj3D rightwheel26(spikeModel3);
    rightwheel26.rotate(180,90,270);
    rightwheel26.translate(9.25, 0, -117.5);
    rightwheel26.scale(2.5, 2.5, 2.5);
    rightwheel26.textureId = 0;
    rightwheel26.hasGravity = false;


    GameObj3D rightwheel25(spikeModel3);
    rightwheel25.rotate(180,90,270);
    rightwheel25.translate(9.25, 0, -112.5);
    rightwheel25.scale(2.5, 2.5, 2.5);
    rightwheel25.textureId = 0;
    rightwheel25.hasGravity = false;


    GameObj3D rightwheel24(spikeModel3);
    rightwheel24.rotate(180,90,270);
    rightwheel24.translate(9.25, 0, -107.5);
    rightwheel24.scale(2.5, 2.5, 2.5);
    rightwheel24.textureId = 0;
    rightwheel24.hasGravity = false;


    GameObj3D rightwheel23(spikeModel3);
    rightwheel23.rotate(180,90,270);
    rightwheel23.translate(9.25, 0, -102.5);
    rightwheel23.scale(2.5, 2.5, 2.5);
    rightwheel23.textureId = 0;
    rightwheel23.hasGravity = false;


    GameObj3D rightwheel22(spikeModel3);
    rightwheel22.rotate(180,90,270);
    rightwheel22.translate(9.25, 0, -97.5);
    rightwheel22.scale(2.5, 2.5, 2.5);
    rightwheel22.textureId = 0;
    rightwheel22.hasGravity = false;




    GameObj3D rightwheel21(spikeModel3);
    rightwheel21.rotate(180,90,270);
    rightwheel21.translate(9.25, 0, -92.5);
    rightwheel21.scale(2.5, 2.5, 2.5);
    rightwheel21.textureId = 0;
    rightwheel21.hasGravity = false;




    GameObj3D rightwheel20(spikeModel3);
    rightwheel20.rotate(180,90,270);
    rightwheel20.translate(9.25, 0, -87.5);
    rightwheel20.scale(2.5, 2.5, 2.5);
    rightwheel20.textureId = 0;
    rightwheel20.hasGravity = false;

    GameObj3D rightwheel19(spikeModel3);
    rightwheel19.rotate(180,90,270);
    rightwheel19.translate(9.25, 0, -82.5);
    rightwheel19.scale(2.5, 2.5, 2.5);
    rightwheel19.textureId = 0;
    rightwheel19.hasGravity = false;


    GameObj3D rightwheel18(spikeModel3);
    rightwheel18.rotate(180,90,270);
    rightwheel18.translate(9.25, 0, -77.5);
    rightwheel18.scale(2.5, 2.5, 2.5);
    rightwheel18.textureId = 0;
    rightwheel18.hasGravity = false;


    GameObj3D rightwheel17(spikeModel3);
    rightwheel17.rotate(180,90,270);
    rightwheel17.translate(9.25, 0, -72.5);
    rightwheel17.scale(2.5, 2.5, 2.5);
    rightwheel17.textureId = 0;
    rightwheel17.hasGravity = false;

    GameObj3D rightwheel16(spikeModel3);
    rightwheel16.rotate(180,90,270);
    rightwheel16.translate(9.25, 0, -67.5);
    rightwheel16.scale(2.5, 2.5, 2.5);
    rightwheel16.textureId = 0;
    rightwheel16.hasGravity = false;

    GameObj3D rightwheel15(spikeModel3);
    rightwheel15.rotate(180,90,270);
    rightwheel15.translate(9.25, 0, -62.5);
    rightwheel15.scale(2.5, 2.5, 2.5);
    rightwheel15.textureId = 0;
    rightwheel15.hasGravity = false;

    GameObj3D rightwheel14(spikeModel3);
    rightwheel14.rotate(180,90,270);
    rightwheel14.translate(9.25, 0, -57.5);
    rightwheel14.scale(2.5, 2.5, 2.5);
    rightwheel14.textureId = 0;
    rightwheel14.hasGravity = false;


    GameObj3D rightwheel13(spikeModel3);
    rightwheel13.rotate(180,90,270);
    rightwheel13.translate(9.25, 0, -52.5);
    rightwheel13.scale(2.5, 2.5, 2.5);
    rightwheel13.textureId = 0;
    rightwheel13.hasGravity = false;


    GameObj3D rightwheel12(spikeModel3);
    rightwheel12.rotate(180,90,270);
    rightwheel12.translate(9.25, 0, -47.5);
    rightwheel12.scale(2.5, 2.5, 2.5);
    rightwheel12.textureId = 0;
    rightwheel12.hasGravity = false;

    GameObj3D rightwheel11(spikeModel3);
    rightwheel11.rotate(180,90,270);
    rightwheel11.translate(9.25, 0, -42.5);
    rightwheel11.scale(2.5, 2.5, 2.5);
    rightwheel11.textureId = 0;
    rightwheel11.hasGravity = false;



    GameObj3D rightwheel10(spikeModel3);
    rightwheel10.rotate(180,90,270);
    rightwheel10.translate(9.25, 0, -37.5);
    rightwheel10.scale(2.5, 2.5, 2.5);
    rightwheel10.textureId = 0;
    rightwheel10.hasGravity = false;


    GameObj3D rightwheel9(spikeModel3);
    rightwheel9.rotate(180,90,270);
    rightwheel9.translate(9.25, 0, -32.5);
    rightwheel9.scale(2.5, 2.5, 2.5);
    rightwheel9.textureId = 0;
    rightwheel9.hasGravity = false;


    GameObj3D rightwheel8(spikeModel3);
    rightwheel8.rotate(180,90,270);
    rightwheel8.translate(9.25, 0, -27.5);
    rightwheel8.scale(2.5, 2.5, 2.5);
    rightwheel8.textureId = 0;
    rightwheel8.hasGravity = false;


    GameObj3D rightwheel7(spikeModel3);
    rightwheel7.rotate(180,90,270);
    rightwheel7.translate(9.25, 0, -22.5);
    rightwheel7.scale(2.5, 2.5, 2.5);
    rightwheel7.textureId = 0;
    rightwheel7.hasGravity = false;

    GameObj3D rightwheel6(spikeModel3);
    rightwheel6.rotate(180,90,270);
    rightwheel6.translate(9.25, 0, -17.5);
    rightwheel6.scale(2.5, 2.5, 2.5);
    rightwheel6.textureId = 0;
    rightwheel6.hasGravity = false;


    GameObj3D rightwheel5(spikeModel3);
    rightwheel5.rotate(180,90,270);
    rightwheel5.translate(9.25, 0, -12.5);
    rightwheel5.scale(2.5, 2.5, 2.5);
    rightwheel5.textureId = 0;
    rightwheel5.hasGravity = false;


    GameObj3D rightwheel4(spikeModel3);
    rightwheel4.rotate(180,90,270);
    rightwheel4.translate(9.25, 0, -7.5);
    rightwheel4.scale(2.5, 2.5, 2.5);
    rightwheel4.textureId = 0;
    rightwheel4.hasGravity = false;


    GameObj3D rightwheel3(spikeModel3);
    rightwheel3.rotate(180,90,270);
    rightwheel3.translate(9.25, 0, -2.5);
    rightwheel3.scale(2.5, 2.5, 2.5);
    rightwheel3.textureId = 0;
    rightwheel3.hasGravity = false;

    GameObj3D rightwheel2(spikeModel3);
    rightwheel2.rotate(180,90,270);
    rightwheel2.translate(9.25, 0, 2.5);
    rightwheel2.scale(2.5, 2.5, 2.5);
    rightwheel2.textureId = 0;
    rightwheel2.hasGravity = false;


    GameObj3D rightwheel1(spikeModel3);
    rightwheel1.rotate(180,90,270);
    rightwheel1.translate(9.25, 0, 7.5);
    rightwheel1.scale(2.5, 2.5, 2.5);
    rightwheel1.textureId = 0;
    rightwheel1.hasGravity = false;

    


    GameObj3D fullCubeObstacle1(cubeModel3);
    fullCubeObstacle1.translate(0, 1.5, -20);
    fullCubeObstacle1.scale(8,1.5,1.5);
    fullCubeObstacle1.textureId = 3;
    fullCubeObstacle1.hasGravity = false;
    
    GameObj3D fullCubeObstacle2(cubeModel3);
    fullCubeObstacle2.translate(0, 1.5, -50);
    fullCubeObstacle2.scale(8,1.5,1.5);
    fullCubeObstacle2.textureId = 3;
    fullCubeObstacle2.hasGravity = false;

    
    GameObj3D fullSpikeObstacle1(spikeModel3);
    fullSpikeObstacle1.translate(5.2, 0, -120);
    fullSpikeObstacle1.scale(2.5,2.5,2.5);
    fullSpikeObstacle1.textureId = 11;
    fullSpikeObstacle1.hasGravity = false;
    
    GameObj3D fullSpikeObstacle2(spikeModel3);
    fullSpikeObstacle2.translate(0, 0, -120);
    fullSpikeObstacle2.scale(2.5,2.5,2.5);
    fullSpikeObstacle2.textureId = 11;
    fullSpikeObstacle2.hasGravity = false;
    
    //2nd road
    GameObj3D secondFloorCube1(cubeModel2);
    secondFloorCube1.translate(0, 0, -130);
    secondFloorCube1.scale(1, 1, 1);
    secondFloorCube1.textureId = 12;

    GameObj3D secondFloorCube2(cubeModel2);
    secondFloorCube2.translate(-8.5, 0, -137);
    secondFloorCube2.scale(2, 2, 2);
    secondFloorCube2.textureId = 12;

    GameObj3D secondFloorCube3(cubeModel2);
    secondFloorCube3.translate(8.5, 0, -137.5);
    secondFloorCube3.scale(2, 2, 2);
    secondFloorCube3.textureId = 12;

    GameObj3D secondFloorCube4(cubeModel2);
    secondFloorCube4.translate(6.2, 2, -145);
    secondFloorCube4.scale(1, 1, 1);
    secondFloorCube4.textureId = 12;

    GameObj3D secondFloorCube5(cubeModel2);
    secondFloorCube1.translate(-4.5, 2.5, -152.5);
    secondFloorCube1.scale(2.5, 2.5, 2.5);
    secondFloorCube1.textureId = 12;

    GameObj3D secondFloorCube6(cubeModel2);
    secondFloorCube6.translate(-2.4, 0, -160);
    secondFloorCube6.scale(2.3, 2.3, 2.3);
    secondFloorCube6.textureId = 12;

    GameObj3D secondFloorCube7(cubeModel2);
    secondFloorCube7.translate(5, 0, -160);
    secondFloorCube7.scale(1, 1, 1);
    secondFloorCube7.textureId = 12;

    GameObj3D secondFloorCube8(cubeModel2);
    secondFloorCube8.translate(-5.6, 0, -167.5);
    secondFloorCube8.scale(1.9, 1.9, 1.9);
    secondFloorCube8.textureId = 12;

    GameObj3D secondFloorCube9(cubeModel2);
    secondFloorCube9.translate(9.2, 0, -175);
    secondFloorCube9.scale(1, 1, 1);
    secondFloorCube9.textureId = 12;

    GameObj3D secondFloorCube10(cubeModel2);
    secondFloorCube10.translate(-6.7, 0, -182.5);
    secondFloorCube10.scale(1, 1, 1);
    secondFloorCube10.textureId = 12;

    GameObj3D secondFloorCube11(cubeModel2);
    secondFloorCube11.translate(6.5, 0, -190);
    secondFloorCube11.scale(1, 1, 1);
    secondFloorCube11.textureId = 12;

    GameObj3D secondFloorCube12(cubeModel2);
    secondFloorCube12.translate(-4.7, 0, -190);
    secondFloorCube12.scale(1, 1, 1);
    secondFloorCube12.textureId = 12;

    GameObj3D secondFloorCube13(cubeModel2);
    secondFloorCube13.translate(1.7, 0, -197.5);
    secondFloorCube13.scale(1, 1, 1);
    secondFloorCube13.textureId = 12;

    GameObj3D secondFloorCube14(cubeModel2);
    secondFloorCube14.translate(-4.6, 0, -205);
    secondFloorCube14.scale(2.2, 2.2, 2.2);
    secondFloorCube14.textureId = 12;

    GameObj3D secondFloorCube15(cubeModel2);
    secondFloorCube15.translate(5.3, 0, -205);
    secondFloorCube15.scale(1.7, 1.7, 1.7);
    secondFloorCube15.textureId = 12;

    GameObj3D secondFloorCube16(cubeModel2);
    secondFloorCube16.translate(-3.4, 0, -212.5);
    secondFloorCube16.scale(1, 1, 1);
    secondFloorCube16.textureId = 12;

    GameObj3D secondFloorCube17(cubeModel2);
    secondFloorCube17.translate(-1.9, 0, -220);
    secondFloorCube17.scale(1, 1, 1);
    secondFloorCube17.textureId = 12;

    GameObj3D secondFloorCube18(cubeModel2);
    secondFloorCube18.translate(1.0, 0, -220);
    secondFloorCube18.scale(1, 1, 1);
    secondFloorCube18.textureId = 12;

    GameObj3D secondFloorCube19(cubeModel2);
    secondFloorCube19.translate(-2.8, 3, -227.5);
    secondFloorCube19.scale(1, 1, 1);
    secondFloorCube19.textureId = 12;

    GameObj3D secondFloorCube20(cubeModel2);
    secondFloorCube20.translate(8.0, 2.4, -235);
    secondFloorCube20.scale(1, 1, 1);
    secondFloorCube20.textureId = 12;

    GameObj3D secondFloorCube21(cubeModel2);
    secondFloorCube21.translate(-7.3, 2.4, -242.5);
    secondFloorCube21.scale(1, 1, 1);
    secondFloorCube21.textureId = 12;

    GameObj3D secondFloorCube22(cubeModel2);
    secondFloorCube22.translate(5.90, 2.4, -250);
    secondFloorCube22.scale(1, 1, 1);
    secondFloorCube22.textureId = 12;

    GameObj3D secondFloorCube23(cubeModel2);
    secondFloorCube23.translate(-2.0, 0, -250);
    secondFloorCube23.scale(1, 1, 1);
    secondFloorCube23.textureId = 12;

    GameObj3D secondFloorCube24(cubeModel2);
    secondFloorCube24.translate(7.1, 0, -257.5);
    secondFloorCube24.scale(1, 1, 1);
    secondFloorCube24.textureId = 12;

    GameObj3D secondFloorCube25(cubeModel2);
    secondFloorCube25.translate(-5, 0, -265);
    secondFloorCube25.scale(1, 1, 1);
    secondFloorCube25.textureId = 12;

    GameObj3D secondFloorCube26(cubeModel2);
    secondFloorCube26.translate(1.8, 0, -272.5);
    secondFloorCube26.scale(5, 5, 5);
    secondFloorCube26.textureId = 12;

    GameObj3D secondFloorCube27(cubeModel2);
    secondFloorCube27.translate(-8.4, 0, -280);
    secondFloorCube27.scale(1, 1, 1);
    secondFloorCube27.textureId = 12;

    GameObj3D secondFloorCube28(cubeModel2);
    secondFloorCube28.translate(6.9, 1.3, 280);
    secondFloorCube28.scale(1, 1, 1);
    secondFloorCube28.textureId = 12;

    GameObj3D secondFloorCube29(cubeModel2);
    secondFloorCube29.translate(-5.4, 0, -287.5);
    secondFloorCube29.scale(1, 1, 1);
    secondFloorCube29.textureId = 12;

    GameObj3D secondFloorCube30(cubeModel2);
    secondFloorCube30.translate(6.5, 1.5, -295);
    secondFloorCube30.scale(1, 1, 1);
    secondFloorCube30.textureId = 12;

    GameObj3D secondFloorCube31(cubeModel2);
    secondFloorCube31.translate(-4.8, 0, -302.5);
    secondFloorCube31.scale(1.3, 1.3, 1.3);
    secondFloorCube31.textureId = 12;

    GameObj3D secondFloorCube32(cubeModel2);
    secondFloorCube32.translate(6.2, 0, -302.5);
    secondFloorCube32.scale(1, 1, 1);
    secondFloorCube32.textureId = 12;


    GameObj3D thirdfloorSpike1(spikeModel);
    thirdfloorSpike1.translate(4, 0, -350);
    thirdfloorSpike1.scale(4, 20, 4);
    thirdfloorSpike1.textureId = 9;
    thirdfloorSpike1.hasGravity = false;
    

    GameObj3D thirdfloorSpike2(spikeModel);
    thirdfloorSpike2.translate(-4, 0, -370);
    thirdfloorSpike2.scale(4, 20, 4);
    thirdfloorSpike2.textureId = 9;
    thirdfloorSpike2.hasGravity = false;
    
    
    GameObj3D thirdfloorSpike3(spikeModel);
    thirdfloorSpike3.translate(4, 0, -390);
    thirdfloorSpike3.scale(4, 20, 4);
    thirdfloorSpike3.textureId = 9;
    thirdfloorSpike3.hasGravity = false;
    
    
    GameObj3D thirdfloorSpike4(spikeModel);
    thirdfloorSpike4.translate(-4, 0, -410);
    thirdfloorSpike4.scale(4, 20, 4);
    thirdfloorSpike4.textureId = 9;
    thirdfloorSpike4.hasGravity = false;
    
    GameObj3D thirdfloorSpike5(spikeModel);
    thirdfloorSpike5.translate(4, 0, -430);
    thirdfloorSpike5.scale(4, 20, 4);
    thirdfloorSpike5.textureId = 9;
    thirdfloorSpike5.hasGravity = false;
    
    GameObj3D thirdfloorSpike6(spikeModel);
    thirdfloorSpike6.translate(-4, 0, -450);
    thirdfloorSpike6.scale(4, 20, 4);
    thirdfloorSpike6.textureId = 9;
    thirdfloorSpike6.hasGravity = false;
    
    GameObj3D thirdfloorSpike7(spikeModel);
    thirdfloorSpike7.translate(4, 0, -470);
    thirdfloorSpike7.scale(4, 20, 4);
    thirdfloorSpike7.textureId = 9;
    thirdfloorSpike7.hasGravity = false;
    
    GameObj3D thirdfloorSpike8(spikeModel);
    thirdfloorSpike8.translate(4, 0, -470);
    thirdfloorSpike8.scale(4, 20, 4);
    thirdfloorSpike8.textureId = 9;
    thirdfloorSpike8.hasGravity = false;
    
    
    GameObj3D finishFloorConfetti1(cubeModel3);
    finishFloorConfetti1.translate(-8, 0, -520);
    finishFloorConfetti1.scale(1, 9, 1);
    finishFloorConfetti1.textureId = 13;
    finishFloorConfetti1.hasGravity = false;
    
    
    GameObj3D finishFloorConfetti2(cubeModel3);
    finishFloorConfetti2.translate(8, 0, -520);
    finishFloorConfetti2.scale(1, 9, 1);
    finishFloorConfetti2.textureId = 13;
    finishFloorConfetti2.hasGravity = false;
    
    GameObj3D finishFloorConfetti3(cubeModel3);
    finishFloorConfetti3.translate(0, 10, -520);
    finishFloorConfetti3.scale(9, 1, 1);
    finishFloorConfetti3.textureId = 13;
    finishFloorConfetti3.hasGravity = false;

    scene.push_back(&fullCubeObstacle1);
    scene.push_back(&fullCubeObstacle2);
    scene.push_back(&fullSpikeObstacle1);
    scene.push_back(&fullSpikeObstacle2);
    
    scene.push_back(&leftwheel1);
    scene.push_back(&rightwheel28);
    scene.push_back(&rightwheel27);
    scene.push_back(&rightwheel26);
    scene.push_back(&rightwheel25);
    scene.push_back(&rightwheel24);
    scene.push_back(&rightwheel23);
    scene.push_back(&rightwheel22);
    scene.push_back(&rightwheel21);
    scene.push_back(&rightwheel20);
    scene.push_back(&rightwheel19);
    scene.push_back(&rightwheel18);
    scene.push_back(&rightwheel17);
    scene.push_back(&rightwheel16);
    scene.push_back(&rightwheel15);
    scene.push_back(&rightwheel14);
    scene.push_back(&rightwheel13);
    scene.push_back(&rightwheel12);
    scene.push_back(&rightwheel11);
    scene.push_back(&rightwheel10);
    scene.push_back(&rightwheel9);
    scene.push_back(&rightwheel8);
    scene.push_back(&rightwheel7);
    scene.push_back(&rightwheel6);
    scene.push_back(&rightwheel5);
    scene.push_back(&rightwheel4);
    scene.push_back(&rightwheel3);
    scene.push_back(&rightwheel2);
    scene.push_back(&rightwheel1);
    scene.push_back(&leftwheel1);
    scene.push_back(&leftwheel2);
    scene.push_back(&leftwheel3);
    scene.push_back(&leftwheel4);
    scene.push_back(&leftwheel5);
    scene.push_back(&leftwheel6);
    scene.push_back(&leftwheel7);
    scene.push_back(&leftwheel8);
    scene.push_back(&leftwheel9);
    scene.push_back(&leftwheel10);
    scene.push_back(&leftwheel11);
    scene.push_back(&leftwheel12);
    scene.push_back(&leftwheel13);
    scene.push_back(&leftwheel14);
    scene.push_back(&leftwheel15);
    scene.push_back(&leftwheel16);
    scene.push_back(&leftwheel17);
    scene.push_back(&leftwheel18);
    scene.push_back(&leftwheel19);
    scene.push_back(&leftwheel20);
    scene.push_back(&leftwheel21);
    scene.push_back(&leftwheel22);
    scene.push_back(&leftwheel23);
    scene.push_back(&leftwheel24);
    scene.push_back(&leftwheel25);
    scene.push_back(&leftwheel26);
    scene.push_back(&leftwheel27);
    scene.push_back(&leftwheel28);
    scene.push_back(&secondFloorCube1);
    scene.push_back(&secondFloorCube2);
    scene.push_back(&secondFloorCube3);
    scene.push_back(&secondFloorCube4);
    scene.push_back(&secondFloorCube5);
    scene.push_back(&secondFloorCube6);
    scene.push_back(&secondFloorCube7);
    scene.push_back(&secondFloorCube8);
    scene.push_back(&secondFloorCube9);
    scene.push_back(&secondFloorCube10);
    scene.push_back(&secondFloorCube11);
    scene.push_back(&secondFloorCube12);
    scene.push_back(&secondFloorCube13);
    scene.push_back(&secondFloorCube14);
    scene.push_back(&secondFloorCube15);
    scene.push_back(&secondFloorCube16);
    scene.push_back(&secondFloorCube17);
    scene.push_back(&secondFloorCube18);
    scene.push_back(&secondFloorCube19);
    scene.push_back(&secondFloorCube20);
    scene.push_back(&secondFloorCube21);
    scene.push_back(&secondFloorCube22);
    scene.push_back(&secondFloorCube23);
    scene.push_back(&secondFloorCube24);
    scene.push_back(&secondFloorCube25);
    scene.push_back(&secondFloorCube26);
    scene.push_back(&secondFloorCube27);
    scene.push_back(&secondFloorCube28);
    scene.push_back(&secondFloorCube29);
    scene.push_back(&secondFloorCube30);
    scene.push_back(&secondFloorCube31);
    scene.push_back(&secondFloorCube32);
    
    scene.push_back(&leftcloud1_1);
    scene.push_back(&leftcloud1_2);
    scene.push_back(&leftcloud1_3);
    scene.push_back(&leftcloud1_4);
    scene.push_back(&leftcloud2_1);
    scene.push_back(&leftcloud2_2);
    scene.push_back(&leftcloud2_3);
    scene.push_back(&leftcloud2_4);
    scene.push_back(&leftcloud3_1);
    scene.push_back(&leftcloud3_2);
    scene.push_back(&leftcloud3_3);
    scene.push_back(&leftcloud3_4);
    scene.push_back(&leftcloud4_1);
    scene.push_back(&leftcloud4_2);
    scene.push_back(&leftcloud4_3);
    scene.push_back(&leftcloud4_4);
    
    scene.push_back(&thirdfloorSpike1);
    scene.push_back(&thirdfloorSpike2);
    scene.push_back(&thirdfloorSpike3);
    scene.push_back(&thirdfloorSpike4);
    scene.push_back(&thirdfloorSpike5);
    scene.push_back(&thirdfloorSpike6);
    scene.push_back(&thirdfloorSpike7);
    scene.push_back(&thirdfloorSpike8);
    scene.push_back(&thirdfloorSpike7);
    scene.push_back(&finishFloorConfetti1);
    scene.push_back(&finishFloorConfetti2);
    scene.push_back(&finishFloorConfetti3);
    
    GameObj3D denemespike(spikeModel3);
    denemespike.rotate(90,0,0);
    denemespike.translate(0, 6, 33);
    denemespike.scale(6, 6 ,6);
    denemespike.textureId = 9;
    denemespike.hasGravity = false;
    scene.push_back(&denemespike);
    
    floor -> translate(0, -1, 0);
    floor -> scale(10,0.8,130);
    floor -> textureId = 1;
    floor -> friction = 0;
    floor -> bubble = -1.5;
    floor -> hasGravity = false;


    secondfloor -> translate(0, -5, -220);
    secondfloor -> scale(10,0.8,90);
    secondfloor -> textureId = 6;
    secondfloor -> friction = 0.8;
    secondfloor -> bubble = -0.2;
    secondfloor -> hasGravity = false;


    thirdfloor -> translate(0, -2, -410);
    thirdfloor -> scale(10,0.8,100);
    thirdfloor -> textureId = 7;
    thirdfloor -> friction = 0.3;
    thirdfloor -> bubble = -1;
    thirdfloor -> hasGravity = false;

    finishfloor -> translate(0, -2, -521);
    finishfloor -> scale(10,0.8,7);
    finishfloor -> textureId = 4;
    finishfloor -> hasGravity = false;
    
    //scene.push_back(&spike3);
    //scene.push_back(&spike4);
    // light
    glm::vec3 lightPos = glm::vec3(0.0, 20.0, 1.0);
    glm::vec3 lightColor = glm::vec3(0.9, 0.6, 0.9);

    const vector<string> texture_files{"../includes/Textures/tekerlek.jpg", "../includes/Textures/taban.jpg",
        "../includes/Textures/deadpool.png",
        "../includes/Textures/ivy.jpg",
        "../includes/Textures/son.png",
        "../includes/Textures/antrasit.jpg",
        "../includes/Textures/asfalt.jpg",
        "../includes/Textures/desert.jpg",
        "../includes/Textures/cloud.jpg",
        "../includes/Textures/bfire.jpg",
        "../includes/Textures/greendust.jpg",
        "../includes/Textures/rust.jpg",
        "../includes/Textures/drs.jpg",
        "../includes/Textures/confetti.jpg",
        };
    
    // load textures
    vector<unsigned int> textures = Textures::loadTextures(texture_files);

    unsigned int skyboxVAO, skyboxVBO;
    initSkybox(skyboxVAO, skyboxVBO);
    vector<std::string> faces{
        "../includes/textures/right.jpg",
        "../includes/textures/left.jpg",
        "../includes/textures/top.jpg",
        "../includes/textures/bottom.jpg",
        "../includes/textures/front.jpg",
        "../includes/textures/back.jpg"};
    unsigned cubemapTexture = loadCubemap(faces);
    ShaderProgram skyboxShader("../includes/shader/skybox.vert", "../includes/shader/skybox.frag");
    skyboxShader.use();
    auto skybox_texture = glGetUniformLocation(skyboxShader.id, "skybox");
    auto u_pv_sky = glGetUniformLocation(skyboxShader.id, "u_pv");
    glUniform1i(skybox_texture, 0); // 0th unit

    // create shader
    ShaderProgram sp("../includes/shader/vertex.vert", "../includes/shader/frag.frag");
    sp.use();
    u_transform = glGetUniformLocation(sp.id, "u_transform");
    u_pv = glGetUniformLocation(sp.id, "u_pv");
    u_frame = glGetUniformLocation(sp.id, "u_frame");
    u_light_pos = glGetUniformLocation(sp.id, "u_light_pos");
    u_light_color = glGetUniformLocation(sp.id, "u_light_color");
    auto u_texture = glGetUniformLocation(sp.id, "u_texture");
    glUniform1i(u_texture, 0);
    glActiveTexture(GL_TEXTURE0); // active 0th unit

    bool oncheck = false;
    //sp.use();
    float acceleration = 1;
    unsigned a = 0;
    int objectcount = 0;
    int renderedcount = 0;
    // game loop
    while (!Window::isClosed())
    {
        renderedcount = 0;
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // update player and camera
        float speed = 0.5*moveFront*acceleration*(1-hero.friction);
        //cout << hero.acceleration << endl;;
        if(moveFront == 1 || moveFront == -1)
        {
            hero.acceleration += 0.004;
            gamepoint +=10;
            if(acceleration < 2)
            {
                acceleration += 0.004;
            }

        }
        hero.moveFront(speed);
        if(velocityZ > 0) {
            if(CollidesWithSth(hero)) {
                hero.moveFront(-speed);
            }
        }
        if(velocityZ_reverse < 0) {
            if(CollidesWithSth(hero)) {
                hero.moveFront(-2*speed);
            }
        }
        if(moveFront == 0)
        {
            if(acceleration > 1)
            {
                acceleration -= 0.006;
                float speed1 = 0.5*acceleration*(1-hero.friction);
                hero.moveFront(speed1);
                if(velocityZ > 0) {
                    if(CollidesWithSth(hero)) {
                        hero.moveFront(-speed1);
                    }
                }
                if(acceleration < 1)
                {
                    acceleration = 1;
                }
            }
        }
        if(speedcheck == 2)
        {
            if(acceleration > 1)
            {
                acceleration = 1;
            }
        }

        hero.rotate(hero.rotation().x, hero.rotation().y - moveRight *2.0f, hero.rotation().z);
        if(moveFront ==1)
        {
            anticheck = true;
            gamepoint +=1;
        }
        if(anticheck)
        {
            antihero4.moveFront(0.2*(1-antihero4.friction));

            if(gamepoint % 50 == 0)
            {
                antihero3.moveUp(3*(1-antihero3.bubble));
                antihero5.moveUp(3*(1-antihero5.bubble));
            }
        }


        if(velocityY > 0) {
            float temp = velocityY*(1-hero.bubble);
            hero.moveUp(temp);
            if(CollidesWithSth(hero)) {
                hero.moveUp(-temp);
            }
        }


        Camera::position = hero.position() - hero.front() * 13.0f + hero.up() * 5.0f;
        Camera::front = hero.front() + glm::vec3(0, mouseY*0.3, 0);
        Camera::up = glm::cross(Camera::front, hero.right());
        Camera::front = glm::rotateY(Camera::front, -mouseX);
        glm::vec4 campos = glm::vec4(Camera::position.x, Camera::position.y, Camera::position.z,1);

        // update uniforms
        sp.use();
        glUniformMatrix4fv(u_pv, 1, GL_FALSE, glm::value_ptr(Camera::getProjMatrix() * Camera::getViewMatrix()));
        glUniform1i(u_frame, 1);
        glUniform3fv(u_light_pos, 1, glm::value_ptr(lightPos));
        glUniform3fv(u_light_color, 1, glm::value_ptr(lightColor));

        std::vector<glm::vec4> importante = getFrustumCornersWorldSpace(Camera::getProjMatrix(), Camera::getViewMatrix());

        for(int i = 0; i< importante.size(); i++)
        {
            cout << " Now, we are proceeding with index " << i << endl;
            cout << importante[i].x << " " << importante[i].y << " " << importante[i].z << endl;
            cout << campos.x << " " <<campos.y << " " << campos.z << endl;
        }
        objectcount = scene.size();
        // scene draw
        for (std::vector<GameObj3D*>::iterator t = scene.begin(); t != scene.end(); ++t) {

            // get the object
            const int i = t - scene.begin();

            // gravity logic
            GameObj3D* object = scene[i];
            if (object -> hasGravity) {
                object -> moveUp(-0.2);
                if(CollidesWithSth(*object)) {
                    object -> moveUp(0.2);
                }
            }

            // for jump logic
            velocityY -= 0.2;

            //draw the object
            glUniformMatrix4fv(u_transform, 1, GL_FALSE, glm::value_ptr(object->getTransform()));
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textures[object->textureId]);
            //glBindTexture(GL_TEXTURE_2D, textures[10]);
            /*
            if(object->position().z < 500)
            {
                object->draw();
                cout << i << ") "<< object->position().x << " " << object->position().y <<  " " <<object->position().z << endl;
                renderedcount++;
            }
             */
            //object->draw();
            /*
            cout << endl;
            cout << importante[0].x << " " << importante[0].y << " " << importante[0].z << endl;
            cout << importante[1].x << " " << importante[1].y << " " << importante[1].z << endl;
            cout << importante[2].x << " " << importante[2].y << " " << importante[2].z << endl;
            cout << importante[3].x << " " << importante[3].y << " " << importante[3].z << endl;
            cout << importante[4].x << " " << importante[4].y << " " << importante[4].z << endl;
            cout << importante[5].x << " " << importante[5].y << " " << importante[5].z << endl;
            cout << endl;
            cout << campos.x << " " <<campos.y << " " << campos.z << endl;
            cout << object->position().x<< " " << object->position().y << " " << object->position().z << endl;
            cout << endl;
            */
            // level of detail control
            
            // level of detail control
            
            cout << "Frustum of "<< i  << " is " << viewfrustumculling(campos, importante, object) << endl;

            if(vfc == false || viewfrustumculling(campos, importante, object) == true)
            {
                if(i != 0  && i != 22 && i != 23 && i != 24 && i != 25)
                {
                    object->draw();
                    oncheck = true;
                    renderedcount++;
                }
                cout << "frustum" << endl;
                cout << i << ") "<< object->position().x << " " << object->position().y <<  " " <<object->position().z << endl;

            }
            if( i == 0 ||  i == 22 || i == 23 || i == 24 || i == 25)
            {
                object->draw();
                cout << i << ") "<< object->position().x << " " << object->position().y <<  " " <<object->position().z << endl;
                renderedcount++;
            }



            /*
            if(object->position().z > -100 )
            {
                object -> draw();
            }
            else{
                if(hero.position().z < -30)
                {
                    object -> draw();
                }
            }
            */
        }
        cout << "total object count is: " << objectcount << endl;
        cout << "rendered object count is: " << renderedcount << endl;
        if(hero.position().z <= - 521 && hero.position().y >= -5)
        {
            cout << "Congratulations!!! Your final score is " << gamepoint << endl;
        }
        renderedcount = 0;

        // draw skybox
        glDepthFunc(GL_LEQUAL); // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        glUniformMatrix4fv(u_pv_sky, 1, GL_FALSE, glm::value_ptr(Camera::getProjMatrix() * glm::mat4(glm::mat3(Camera::getViewMatrix()))));
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        // update the scene
        Window::refresh();
    }

    Window::terminate();

    return 0;
}
