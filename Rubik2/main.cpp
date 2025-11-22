#include <iostream>
#include <vector>
#include <cmath>
#include "RubikCube.h"
#include "Camera.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
//Update after input?
bool needsUpdate = true;
// Camera Orbit Variables
float ORBIT_RADIUS = 6.0f; 
float orbitAngleY = PI/4.0f; // Horizontal angle (Yaw)
float orbitAngleX = PI/4.0f; // Vertical angle (Pitch)
const float orbitSpeed = 0.05f;
// -- Rubik Cube variables
Axis currentAxis = Axis::Z;
int Slice = 1; 
RubikCube* g_rubikCube = nullptr;
// -- Time/Frame Management
float deltaTime = 0.0f; 
float lastFrame = 0.0f;
// -- GLFW - Window and Input
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
//std::vector<std::string> input_moves;


#include <memory>
#include <array>
#include <cstdio>
#include <string>
std::string RunCommand(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
glfwWindowHint(GLFW_DEPTH_BITS, 24);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Window", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }
	
	Shader cubeShader("simple.vs", "simple.fs");
	
// -- CAMERA SETUP --
	Camera camera(vec3(ORBIT_RADIUS, 0.0f, 0.0f));
	//float currentOrbitY = PI/2.0f;
	//float currentOrbitX = 0.0f; 
	camera.Orbit(orbitAngleY, orbitAngleX, ORBIT_RADIUS, vec3(0.0f));
	glEnable(GL_DEPTH_TEST);
// -- LIGHTS AND MATERIAL CONFIG --
	vec3 lightPos(2.0f, 3.0f, 4.0f); 
    vec3 lightAmbient(0.3f, 0.3f, 0.3f);
    vec3 lightDiffuse(1.0f, 1.0f, 1.0f);
    vec3 lightSpecular(1.0f, 1.0f, 1.0f);
	//Material
	vec3 materialSpecular(0.8f, 0.8f, 0.8f); //More to reflect MORE! *Sigh*///
    float materialShininess = 64.0f;   //More = sharper = smaller highlight

// -- Textures Config --
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);cubeShader.setInt("entryTexture", 0);
	// Texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Load image
	int width, height, nrChannels;
	unsigned char* data = stbi_load("RubikTexture.png", &width, &height, &nrChannels, 0);
	if (data) {
		GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		std::cerr << "Failed to load texture\n";
	}
	stbi_image_free(data);
// -- RubikCube
	RubikCube rubikCube;
	g_rubikCube = &rubikCube;
//----------------Main Loop---------------------
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
		
		// -- Time Logic --
		float currentFrame = glfwGetTime(); //glfw is convenient...
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
        if (needsUpdate) {
			rubikCube.SelectSlice(currentAxis, Slice);//
			camera.Orbit(orbitAngleY, orbitAngleX, ORBIT_RADIUS, vec3(0.0f));
			needsUpdate = false;
		}
		cubeShader.use();
// -- LIGHT CALCULATION --
		// Set Camera Position for Specular Calculations
		cubeShader.setVec3("viewPos", camera.Position); // Assuming Camera class has a public 'Position' member
		// Set Light Uniforms
		cubeShader.setVec3("light.position", lightPos);
		cubeShader.setVec3("light.ambient", lightAmbient);
		cubeShader.setVec3("light.diffuse", lightDiffuse);
		cubeShader.setVec3("light.specular", lightSpecular);
		// Set Material
		cubeShader.setVec3("material.specular", materialSpecular);
		cubeShader.setFloat("material.shininess", materialShininess);
		
		matrix4 projMatrix;
		projMatrix.Perspective(camera.Zoom * (PI / 180.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		cubeShader.setMat4("projection", projMatrix);
		// Set View Matrix (Updates every frame/input)
		cubeShader.setMat4("view", camera.GetViewMatrix());
		// Draw the entire cube
		rubikCube.Update(deltaTime);
		rubikCube.Draw(cubeShader);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		if (g_rubikCube->isRotating) return;
		//std::cout << "Layer Changed \n";
        if (Slice == 0) Slice = 1;
		else if (Slice == 1) Slice = -1;
		else Slice = Slice = 0;
		needsUpdate = true;
    }
	if (key == GLFW_KEY_X && action == GLFW_PRESS)
	{
		if (g_rubikCube->isRotating) return;
		//std::cout << "Axis Changed \n";
		if(currentAxis == Axis::X) currentAxis = Axis::Y;
		else if(currentAxis== Axis::Y) currentAxis = Axis::Z;
		else if(currentAxis== Axis::Z) currentAxis = Axis::X;
		needsUpdate = true;
	}
    if (key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		g_rubikCube->RotateSlice(currentAxis, Slice);
        /*if (g_rubikCube) {
            //g_rubikCube->RotateSliceSnap(currentAxis, Slice, PI / 2.0f);// PI/2.0f is 90 degrees in radians
			if		(currentAxis == Axis::X && Slice == 1 && g_rubikCube->direction > 0) 
				input_moves.push_back("F");
			else if	(currentAxis == Axis::X && Slice == 1 && g_rubikCube->direction < 0) 
				input_moves.push_back("F'");
			else if	(currentAxis == Axis::X && Slice == -1 && g_rubikCube->direction > 0) 
				input_moves.push_back("B");
			else if	(currentAxis == Axis::X && Slice == -1 && g_rubikCube->direction < 0) 
				input_moves.push_back("B'");
			
			else if (currentAxis == Axis::Y && Slice == 1 && g_rubikCube->direction > 0) 
				input_moves.push_back("U");
			else if	(currentAxis == Axis::Y && Slice == 1 && g_rubikCube->direction < 0)
				input_moves.push_back("U'");
			else if	(currentAxis == Axis::Y && Slice == -1 && g_rubikCube->direction > 0) 
				input_moves.push_back("D");
			else if	(currentAxis == Axis::Y && Slice == -1 && g_rubikCube->direction < 0)
				input_moves.push_back("D'");
			
			else if (currentAxis == Axis::Z && Slice == 1 && g_rubikCube->direction > 0) 
				input_moves.push_back("L");
			else if	(currentAxis == Axis::Z && Slice == 1 && g_rubikCube->direction < 0)
				input_moves.push_back("L'");
			else if	(currentAxis == Axis::Z && Slice == -1 && g_rubikCube->direction > 0) 
				input_moves.push_back("R");
			else if	(currentAxis == Axis::Z && Slice == -1 && g_rubikCube->direction < 0)
				input_moves.push_back("R'");
			else
				std::cout << "--------------- INVALID MOVE  --------------- \n";	

        }*/
		needsUpdate = true;
	}
	
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		if (g_rubikCube->isRotating) return;
		std::cout << "--------------- CUBE STATE --------------- \n";
		std::cout << g_rubikCube->GetState() << " \n";
		g_rubikCube->PrintCubeNet(g_rubikCube->GetState());
		//g_rubikCube->PrintCubeNetWithIndex(g_rubikCube->GetState());
		needsUpdate = true;
	}
	if (key == GLFW_KEY_A && action == GLFW_PRESS)
	{
		if (g_rubikCube->isRotating) return;
		g_rubikCube->direction *= -1;
		//std::cout << "DIRECTION CHANGED";
		needsUpdate = true;
	}
	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
	{
		if (g_rubikCube->isRotating) return;
		std::cout << "Scramble Attempted --- NOT IMPLEMENTED \n";
		needsUpdate = true;
	}
	if (key == GLFW_KEY_V && action == GLFW_PRESS)
	{
		if (g_rubikCube->isRotating) return;
		std::cout << "Auto Solver Called ---\n";
		std::string moveList = g_rubikCube->GetMoveList();
		std::string cmd = "python solver2.py \"" + moveList + "\"";
		std::string output = RunCommand(cmd);

		std::cout << "Solver output: " << output << std::endl;
		g_rubikCube->SetSolutionList(output);
		//std::cout << "Solution LIST: " << g_rubikCube->GetSolutionList() << std::endl;
		std::cout << "Moves LIST: " << g_rubikCube->GetMoveList() << std::endl;
		auto  moves = g_rubikCube->ParseMoves(g_rubikCube->GetSolutionList());//g_rubikCube->GetMoveList());
		g_rubikCube->StartExecutingSolution(moves);
		
		needsUpdate = true;
	}
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		ORBIT_RADIUS -= 0.5f;
		needsUpdate = true;
	}
	if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		ORBIT_RADIUS += 0.5f;
		needsUpdate = true;
	}
	// Camera Orbit Control (Directional Keys)
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        orbitAngleY += orbitSpeed; // Orbit Left
        needsUpdate = true;
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        orbitAngleY -= orbitSpeed; // Orbit Right
        needsUpdate = true;
    }
    if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        // Attempt to prevent gimbal lock / going upside down (limit vertical angle)
        orbitAngleX = std::min(orbitAngleX + orbitSpeed, PI/2.0f - 0.01f); // Orbit Up
        needsUpdate = true;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        orbitAngleX = std::max(orbitAngleX - orbitSpeed, -PI/2.0f + 0.01f); // Orbit Down
        needsUpdate = true;
    }
	
}
