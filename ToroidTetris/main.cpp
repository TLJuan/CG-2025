// main.cpp — FINAL VERSION — Cylindrical Tetris Torus (2025 Edition)
#include <iostream>
#include <vector>
#include <cmath>
#include "Game.h"  // This already includes Camera.h, Mesh.h, Shader.h, MatrixOperations.h, etc.

// ===================================================================
// GLOBAL GAME & RENDERER
// ===================================================================
TetrisGame g_tetrisGame;
TetrisRenderer* g_renderer = nullptr;
Shader* g_shader = nullptr;
unsigned int g_tetrisAtlas = 0;

// Camera orbit
float ORBIT_RADIUS = 28.0f;
float orbitAngleY = PI / 4.0f;
float orbitAngleX = PI / 6.0f;
const float orbitSpeed = 0.8f;

float CustomRadius = 12.0f;
// Time
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Gravity timer (will use later)
float fallTimer = 0.0f;
float fallDelay = 0.8f;  // seconds per drop (level 1)

// ===================================================================
// GLFW CALLBACKS
// ===================================================================
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Zoom
    if (key == GLFW_KEY_Q && action == GLFW_PRESS) ORBIT_RADIUS = std::max(10.0f, ORBIT_RADIUS - 3.0f);
    if (key == GLFW_KEY_E && action == GLFW_PRESS) ORBIT_RADIUS += 3.0f;

    // Camera orbit (hold keys)
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_A) orbitAngleY += orbitSpeed * deltaTime * 60.0f;
        if (key == GLFW_KEY_D) orbitAngleY -= orbitSpeed * deltaTime * 60.0f;
        if (key == GLFW_KEY_W) orbitAngleX = std::min(PI/2.0f - 0.1f, orbitAngleX + orbitSpeed * deltaTime * 60.0f);
        if (key == GLFW_KEY_S) orbitAngleX = std::max(-PI/2.0f + 0.1f, orbitAngleX - orbitSpeed * deltaTime * 60.0f);
    }

    // === TETRIS CONTROLS (only on PRESS) ===
    if (action != GLFW_PRESS) return;

    switch (key) {
        case GLFW_KEY_LEFT:  
		g_tetrisGame.move(-1, 0);  
		
		//std::cout << "Piece: " << (int)g_tetrisGame.current.type << " Minos:\n";
		//for (auto [px, py] : g_tetrisGame.current.minoPositions()) {
			//std::cout << "(" << px << ", " << py << ") ";
		//}
		//std::cout << "\n";
		
		g_renderer->UpdateCurrentPiece(); 
		g_renderer->UpdateGhost(); 
		break;
        case GLFW_KEY_RIGHT: g_tetrisGame.move( 1, 0);  g_renderer->UpdateCurrentPiece(); g_renderer->UpdateGhost(); break;
        case GLFW_KEY_UP:    g_tetrisGame.rotate(true); g_renderer->UpdateCurrentPiece(); g_renderer->UpdateGhost(); break;
        case GLFW_KEY_DOWN:  g_tetrisGame.move(0, -1);  g_renderer->UpdateCurrentPiece(); g_renderer->UpdateGhost(); break;
        case GLFW_KEY_SPACE: g_tetrisGame.hardDrop();   g_renderer->RebuildBoard();      g_renderer->UpdateCurrentPiece(); g_renderer->UpdateGhost(); break;
        case GLFW_KEY_C:     g_tetrisGame.holdPiece();  g_renderer->UpdateCurrentPiece(); g_renderer->UpdateGhost(); break;
        case GLFW_KEY_R:
            g_tetrisGame.start();
            g_renderer->RebuildBoard();
            g_renderer->UpdateCurrentPiece();
            g_renderer->UpdateGhost();
            std::cout << "=== GAME RESTARTED ===\n";
            break;
		case GLFW_KEY_Z:
			CustomRadius++;
			g_renderer->SetRadius(CustomRadius);
			std::cout << "RAD -" << CustomRadius <<"\n";
			break;
		case GLFW_KEY_X:
			CustomRadius--;
			g_renderer->SetRadius(CustomRadius);
			std::cout << "RAD -" << CustomRadius <<"\n";
			break;
    }
}

// ===================================================================
// MAIN
// ===================================================================
int main() {
    // GLFW init
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    GLFWwindow* window = glfwCreateWindow(1200, 800, "Tetris Torus — Cylindrical 3D Tetris", nullptr, nullptr);
    if (!window) { std::cerr << "Failed to create window\n"; return -1; }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n"; return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Shader
    Shader shader("simple.vs", "simple.fs");
    g_shader = &shader;
    shader.use();
    shader.setInt("entryTexture", 0);

    // Load Tetris Atlas (your 448×64 or 512×512 image with 7 colors)
    int w, h, n;
    unsigned char* data = stbi_load("textures/tetris_atlas.png", &w, &h, &n, 4);
    if (!data) {
        std::cerr << "CRITICAL: Could not load tetris_atlas.png!\n";
        return -1;
    }

    glGenTextures(1, &g_tetrisAtlas);
    glBindTexture(GL_TEXTURE_2D, g_tetrisAtlas);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);

    // === CREATE ALL MINO MESHES ===
    CreateAllMinoMeshes();  // ← this function must exist (see below)

    // Start game & renderer
    g_tetrisGame.start();
    g_renderer = new TetrisRenderer(g_tetrisGame, shader);
    g_renderer->SetRadius(14.0f);

    Camera camera(vec3(0, 10, 30));

    // ===================================================================
    // MAIN LOOP
    // ===================================================================
    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;

        // Auto-fall (gravity)
        fallTimer += deltaTime;
        if (fallTimer >= fallDelay) {
            fallTimer = 0.0f;
            if (!g_tetrisGame.move(0, -1)) {
                g_tetrisGame.lockCurrent();
                g_renderer->RebuildBoard();
                g_renderer->UpdateCurrentPiece();
                g_renderer->UpdateGhost();
            } else {
                g_renderer->UpdateCurrentPiece();
                g_renderer->UpdateGhost();
            }
        }

        // Input & camera
        glfwPollEvents();
        camera.Orbit(orbitAngleY, orbitAngleX, ORBIT_RADIUS, vec3(0, 8, 0));  // look slightly down

        // Clear
        glClearColor(0.08f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Projection
        matrix4 projection;
        projection.Perspective(45.0f * (PI/180.0f), 1200.0f / 800.0f, 0.1f, 200.0f);

        // Render
        g_renderer->Render(camera, projection);

        glfwSwapBuffers(window);
    }

    delete g_renderer;
    glfwTerminate();
    return 0;
}