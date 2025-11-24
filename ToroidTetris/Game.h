#pragma once
#include "Tetris.h"


unsigned int tetrisAtlas;
int atlasW, atlasH;

void LoadTetrisAtlas() {
    glGenTextures(1, &tetrisAtlas);
    glBindTexture(GL_TEXTURE_2D, tetrisAtlas);

    unsigned char* data = stbi_load("tetris_atlas.png", &atlasW, &atlasH, nullptr, 4);
    if (!data) {
        std::cout << "Failed to load tetris_atlas.png!\n";
        // fallback: create solid red texture
        atlasW = atlasH = 64;
        data = new unsigned char[64*64*4];
        memset(data, 255, 64*64*4); // white fallback
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, atlasW, atlasH, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    stbi_image_free(data);
}

struct MinoUV {
    UVRange uv;
};

const std::array<MinoUV, 9> MinoUVs{{
    { {0.0f,  0.0f, 0.0f, 0.0f} },     // Empty
    { {0.00f, 0.00f, 0.25f, 0.50f} },   // I     → tile (0,0)
    { {0.25f, 0.00f, 0.50f, 0.50f} },   // O
    { {0.50f, 0.00f, 0.75f, 0.50f} },   // T
    { {0.75f, 0.00f, 1.00f, 0.50f} },   // S
    { {0.00f, 0.50f, 0.25f, 1.00f} },   // Z
    { {0.25f, 0.50f, 0.50f, 1.00f} },   // J
    { {0.50f, 0.50f, 0.75f, 1.00f} },   // L
    { {0.75f, 0.50f, 1.00f, 1.00f} }    // Ghost (darker gray)
}};
std::array<Mesh, 9> MinoMeshes;  // index = Mino enum
/*
void CreateAllMinoMeshes() {
    for (int i = 1; i <= 8; ++i) {
        auto uv = MinoUVs[i];
        MinoMeshes[i] = CreateRubikCubieMesh(uv.uv, uv.uv, uv.uv, uv.uv, uv.uv, uv.uv);
        MinoMeshes[i].Setup();
    }
    // Ghost can reuse any mesh — UVs define the look
    MinoMeshes[8] = CreateRubikCubieMesh(MinoUVs[8].uv, MinoUVs[8].uv, MinoUVs[8].uv,
                                         MinoUVs[8].uv, MinoUVs[8].uv, MinoUVs[8].uv);
    MinoMeshes[8].Setup();
}
*/

void CreateAllMinoMeshes() {
    for (int i = 1; i <= 8; ++i) {
        auto uv = MinoUVs[i];
        MinoMeshes[i] = CreateRubikCubieMesh(uv.uv, uv.uv, uv.uv, uv.uv, uv.uv, uv.uv);
        MinoMeshes[i].Setup();
    }
}


/*	void CreateAllCubieMeshes() {
		for (int i = 1; i <= 8; ++i) {  // skip Empty
			Mino m = static_cast<Mino>(i);
			UVRange uv = MinoUVs[i].uv;

			CubieMeshes[i] = CreateRubikCubieMesh(
				uv, uv, uv, uv, uv, uv,  // same UV on all faces
				0, 0, 0
			);
			CubieMeshes[i].Setup();
		}
		// Ghost uses dim version or special tile
		CubieMeshes[8] = CreateRubikCubieMesh(
			MinoUVs[8].uv, MinoUVs[8].uv, MinoUVs[8].uv,
			MinoUVs[8].uv, MinoUVs[8].uv, MinoUVs[8].uv
		);
		CubieMeshes[8].Setup();
	}
*/
/*class Cubies {
public:
    int gridX, gridY;
    Mino type;
    matrix4 modelMatrix;

    Cubies(int x, int y, Mino t) : gridX(x), gridY(y), type(t) {
        UpdateModelMatrix();
    }

    void UpdateModelMatrix(float radius = 12.0f, float size = 1.0f) {
        float angle = (float(gridX) / BOARD_WIDTH) * 2.0f * PI;
        float x = radius * cosf(angle);
        float z = radius * sinf(angle);
        float y = gridY * size;

        modelMatrix.Identity();
        modelMatrix.Translate(x, y, z);
        modelMatrix.Scale(size * 0.98f, size * 0.98f, size * 0.98f);
    }

    void Draw(const Shader& shader) const {
        if (type == Mino::Empty) return;

        shader.setMat4("model", modelMatrix);
        shader.setInt("mode", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tetrisAtlas);
        glBindVertexArray(CubieMeshes[(int)type].VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }

    // Ghost uses wireframe
    void DrawGhost(const Shader& shader) const {
        shader.setMat4("model", modelMatrix);
        shader.setInt("mode", 1);  // Yellow wireframe from your shader

        glBindVertexArray(CubieMeshes[(int)type].VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }
};
*/

class Cubies {
public:
    int gridX, gridY;
    Mino type = Mino::Empty;
    matrix4 modelMatrix;
    float radius = 12.0f;
    float size   = 1.0f;

    Cubies(int x, int y, Mino t, float r = 12.0f, float s = 1.0f)
        : gridX(x), gridY(y), type(t), radius(r), size(s) {
        UpdateModelMatrix();
    }

    // --- Fix for Game.h: Cubies::UpdateModelMatrix() ---
void UpdateModelMatrix() {
    // 1. Calculate cylindrical coordinates
    float angle = (float(gridX) / BOARD_WIDTH) * 2.0f * PI;
    float x = radius * cosf(angle);
    float z = radius * sinf(angle);
    float y = gridY * size;
    
    // 2. Use existing, faulty functions to create new, temporary matrices
    matrix4 translationMatrix;
    translationMatrix.Translate(x, y, z); // translationMatrix is now ONLY the translation

    //matrix4 scaleMatrix;
    //scaleMatrix.Scale(size * 0.98f, size * 0.98f, size * 0.98f); // scaleMatrix is now ONLY the scaling

    // 3. Combine them using matrix multiplication (Scale must happen before Translate)
    // M_model = M_Translation * M_Scale
	modelMatrix = translationMatrix;
    //modelMatrix = translationMatrix * scaleMatrix;
    // Note: If you add rotation later, it would be:
    // modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;
}

    void Draw(const Shader& shader, bool ghost = false) const {
        if (type == Mino::Empty) return;

        shader.setMat4("model", modelMatrix);
        shader.setInt("mode", ghost ? 1 : 0);  // 0 = filled, 1 = yellow wireframe

        glBindVertexArray(MinoMeshes[(int)type].VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }
};



class TetrisRenderer {
private:
    std::vector<Cubies> boardCubies;
    std::vector<Cubies> currentPieceCubies;
    std::vector<Cubies> ghostCubies;

    float radius = 2.0f; //used to be 12, too much
    float blockSize = 1.0f;

public:
    TetrisGame& game;
    Shader& shader;  // reference to your main shader

    TetrisRenderer(TetrisGame& g, Shader& s) : game(g), shader(s) {
        
		SetRadius(radius);
		//RebuildBoard();
        //UpdateCurrentPiece();
        //UpdateGhost();
    }

    void SetRadius(float r) {
        radius = r;
        RebuildBoard();
        UpdateCurrentPiece();
        UpdateGhost();
    }

    void RebuildBoard() {
        boardCubies.clear();
        for (int y = 0; y < BOARD_HEIGHT; ++y) {
            for (int x = 0; x < BOARD_WIDTH; ++x) {
                Mino m = game.board.getCell(x, y);
                if (m != Mino::Empty) {
                    boardCubies.emplace_back(x, y, m, radius, blockSize);
                }
            }
        }
    }

    void UpdateCurrentPiece() {
        currentPieceCubies.clear();
        for (auto [px, py] : game.current.minoPositions()) {
            currentPieceCubies.emplace_back(px, py, game.current.type, radius, blockSize);
        }
    }

    void UpdateGhost() {
        ghostCubies.clear();
        Pieces ghost = game.current;
        while (game.board.canPlace(ghost)) ghost.y--;
        ghost.y++;  // one step above lock

        for (auto [px, py] : ghost.minoPositions()) {
            ghostCubies.emplace_back(px, py, Mino::Ghost, radius, blockSize);
        }
    }

    void Render(Camera& cam, const matrix4& projection) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tetrisAtlas);
		
		
		//vec3 lightPos(2.0f, 3.0f, 4.0f); 
		//vec3 lightAmbient(0.3f, 0.3f, 0.3f);
		//vec3 lightDiffuse(1.0f, 1.0f, 1.0f);
		//vec3 lightSpecular(1.0f, 1.0f, 1.0f);
		
		
        shader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view", cam.GetViewMatrix());
        shader.setVec3("viewPos", cam.Position);

        // Light (tweak to taste)
        shader.setVec3("light.position", vec3(2.0f, 3.0f, 4.0f));
        shader.setVec3("light.ambient",  vec3(0.6f, 0.6f, 0.6f));
        shader.setVec3("light.diffuse",  vec3(1.0f, 1.0f, 1.0f));
        shader.setVec3("light.specular", vec3(1.0f, 1.0f, 1.2f));
		
		// Set Material
		shader.setVec3("material.specular", vec3(0.8f, 0.8f, 0.8f));
		shader.setFloat("material.shininess", 64.0f);

        // Draw order: board → ghost → current
        for (auto& c : boardCubies)          c.Draw(shader, false);
        for (auto& c : ghostCubies)          c.Draw(shader, true);   // yellow wireframe
        for (auto& c : currentPieceCubies)   c.Draw(shader, false);
    }
};