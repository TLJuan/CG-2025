#pragma once
#include "Tetris.h"


unsigned int tetrisAtlas;
int atlasW, atlasH;

void LoadTetrisAtlas() {
    glGenTextures(1, &tetrisAtlas);
    glBindTexture(GL_TEXTURE_2D, tetrisAtlas);

    unsigned char* data = stbi_load("textures/tetris_atlas.png", &atlasW, &atlasH, nullptr, 4);
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

const float U_SIZE = 1.0f / 7.0f;
const float V_SIZE = 1.0f ;

// Assuming an 8-tile wide (1 row high) texture atlas: 1.0 / 8.0 = 0.125
float U_TILE_SIZE = 1.0f/7.0f;//0.125f; 

const std::array<MinoUV, 9> MinoUVs{{
    { {0.0f, 0.0f, 0.0f, 0.0f} },   // 0. Empty (no texture)
    
    // I Piece (U: 0.000 to 0.125, V: 0.00 to 1.00)
    { {0.000f, 0.00f, U_TILE_SIZE * 1.0f, 1.00f} },  // 1. I 
    
    // O Piece (U: 0.125 to 0.250, V: 0.00 to 1.00)
    { {U_TILE_SIZE * 1.0f, 0.00f, U_TILE_SIZE * 2.0f, 1.00f} },  // 2. O
    
    // T Piece (U: 0.250 to 0.375, V: 0.00 to 1.00)
    { {U_TILE_SIZE * 2.0f, 0.00f, U_TILE_SIZE * 3.0f, 1.00f} },  // 3. T
    
    // S Piece (U: 0.375 to 0.500, V: 0.00 to 1.00)
    { {U_TILE_SIZE * 3.0f, 0.00f, U_TILE_SIZE * 4.0f, 1.00f} },  // 4. S
    
    // Z Piece (U: 0.500 to 0.625, V: 0.00 to 1.00)
    { {U_TILE_SIZE * 4.0f, 0.00f, U_TILE_SIZE * 5.0f, 1.00f} },  // 5. Z
    
    // J Piece (U: 0.625 to 0.750, V: 0.00 to 1.00)
    { {U_TILE_SIZE * 5.0f, 0.00f, U_TILE_SIZE * 6.0f, 1.00f} },  // 6. J
    
    // L Piece (U: 0.750 to 0.875, V: 0.00 to 1.00)
    { {U_TILE_SIZE * 6.0f, 0.00f, U_TILE_SIZE * 7.0f, 1.00f} },  // 7. L
    
    // Ghost Piece (U: 0.875 to 1.000, V: 0.00 to 1.00)
    { {U_TILE_SIZE * 7.0f, 0.00f, U_TILE_SIZE * 8.0f, 1.00f} }   // 8. Ghost
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



class Cubies {
public:
    int gridX, gridY;
    Mino type = Mino::Empty;
    matrix4 modelMatrix;
    float radius = 2.0f;
    float size   = 1.0f;

    Cubies(int x, int y, Mino t, float r = 2.0f, float s = 1.0f)
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
	
    matrix4 translationMatrix;
    translationMatrix.Translate(x, y, z); 

    //matrix4 scaleMatrix;
    //scaleMatrix.Scale(size * 0.98f, size * 0.98f, size * 0.98f);
    // M_model = M_Translation * M_Scale
	modelMatrix = translationMatrix;
    //modelMatrix = translationMatrix * scaleMatrix;
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
    Shader& shader; 
	Mesh floorMesh;

    TetrisRenderer(TetrisGame& g, Shader& s) : game(g), shader(s) {
        
		SetRadius(radius);
		floorMesh = CreateCircleMesh(radius + 0.6f, 64, -0.5f);
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
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, tetrisAtlas);
		
		shader.use();
		shader.setMat4("projection", projection);
		shader.setMat4("view", cam.GetViewMatrix());
		shader.setInt("mode", 0); // solid

		// ---- Draw circular floor base ----
		{
			matrix4 floorModel;
			floorModel.Identity();
			shader.setMat4("model", floorModel);

			glBindVertexArray(floorMesh.VAO);
			glDrawElements(GL_TRIANGLES, floorMesh.indices.size(), GL_UNSIGNED_INT, 0);
		}
		// ---------------------------------

		
		
		
		vec3 lightPos(0.0f, 0.0f, 0.0f); 
		vec3 lightAmbient(0.4f, 0.4f, 0.4f);
		vec3 lightDiffuse(0.70f, 0.70f, 0.70f);
		vec3 lightSpecular(1.0f, 1.0f, 1.0f);
		
		vec3 materialSpecular(0.5f, 0.5f, 0.5f);
		float materialShininess = 32.0f;
		
        shader.use();
		shader.setVec3("viewPos", cam.Position);

        // Light (tweak to taste)
        shader.setVec3("light.position", lightPos);
        shader.setVec3("light.ambient",  lightAmbient);
        shader.setVec3("light.diffuse",  lightDiffuse);
        shader.setVec3("light.specular", lightSpecular);
		
		shader.setFloat("light.constant", 1.0f);
		shader.setFloat("light.linear", 0.09f);
		shader.setFloat("light.quadratic", 0.032f);
		// Set Material
		shader.setVec3("material.specular", materialSpecular);
		shader.setFloat("material.shininess", materialShininess);
		
		shader.setMat4("projection", projection);
        shader.setMat4("view", cam.GetViewMatrix());

        // Draw order: board → ghost → current
        for (auto& c : boardCubies)          c.Draw(shader, false);
        for (auto& c : ghostCubies)          c.Draw(shader, true);   // yellow wireframe
        for (auto& c : currentPieceCubies)   c.Draw(shader, false);
    }
};