#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#pragma once
#include <vector>
#include <array>
#include "MatrixOperations.h"
const float PI = 3.14159265359f;

class Mesh {
public:
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO, VBO, EBO;
	vec3 center; 
	matrix4 modelMatrix;
	
	Mesh() : VAO(0), VBO(0), EBO(0), center(0,0,0) {
        modelMatrix.Identity();
    }
    Mesh(const std::vector<float>& verts, const std::vector<unsigned int>& inds, vec3 c = vec3(0.0f))
        : vertices(verts), indices(inds), center(c) {
			modelMatrix.Identity();
		}
	void Initialize() {
		modelMatrix.Translate(center.x, center.y, center.z);
	}
	void Setup(){
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
		
		const int stride = 8*sizeof(float);
			//Adding Textures. Position = location 0 (3 floats), texCoord = location 1 (2 floats). Stride = 5 floats.
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0); // Position
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float))); // TexCoord
			glEnableVertexAttribArray(1);	
			//Normal Vector = location 2 (3 floats)
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float))); 
			glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }
    void UpdateVertices(const matrix4& model) {
		std::vector<float> transformed = vertices;
		matrix4 final = model;
		final.TransformVertices(transformed);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, transformed.size() * sizeof(float), transformed.data());
	}
	
	void Draw(unsigned int shaderProgram){
        glBindVertexArray(VAO);
        glUseProgram(shaderProgram);

        // Filled
        glUniform1i(glGetUniformLocation(shaderProgram, "mode"), 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        // Wireframe
        glUniform1i(glGetUniformLocation(shaderProgram, "mode"), 1);
        glLineWidth(3.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        // Points
        glUniform1i(glGetUniformLocation(shaderProgram, "mode"), 2);
        glPointSize(5.0f);
        glDrawArrays(GL_POINTS, 0, vertices.size() / 3);
    }
	
	matrix4 GetModelMatrix() const {
		matrix4 model;
		model.Translate(center);
		return model;
	}
	matrix4 GetCenteredTransform(const matrix4& transform) const {
		matrix4 T1, T2;
		T1.InverseTranslate(center.x, center.y, center.z); // Move to origin
		T2.Translate(center.x, center.y, center.z);        // Move back
		return T2 * transform * T1;
	}

};

//Primitives - (Spawn position/local axis) as parameter
static Mesh CreateCube(float cx = 0.0f, float cy = 0.0f, float cz = 0.0f) {

    // Normal vectors for each face
    std::vector<float> faceNormals = {
         0.0f,  0.0f,  1.0f,  // Front
         0.0f,  0.0f, -1.0f,  // Back
        -1.0f,  0.0f,  0.0f,  // Left
         1.0f,  0.0f,  0.0f,  // Right
         0.0f, -1.0f,  0.0f,  // Bottom
         0.0f,  1.0f,  0.0f   // Top
    };

    // Position + UV data (same as your original)
    std::vector<float> baseVerts = {
        // Front
        -0.5f,-0.5f, 0.5f,  0.0f,0.0f,
         0.5f,-0.5f, 0.5f,  1.0f,0.0f,
         0.5f, 0.5f, 0.5f,  1.0f,1.0f,
        -0.5f, 0.5f, 0.5f,  0.0f,1.0f,

        // Back
        -0.5f,-0.5f,-0.5f,  1.0f,0.0f,
         0.5f,-0.5f,-0.5f,  0.0f,0.0f,
         0.5f, 0.5f,-0.5f,  0.0f,1.0f,
        -0.5f, 0.5f,-0.5f,  1.0f,1.0f,

        // Left
        -0.5f,-0.5f,-0.5f,  0.0f,0.0f,
        -0.5f,-0.5f, 0.5f,  1.0f,0.0f,
        -0.5f, 0.5f, 0.5f,  1.0f,1.0f,
        -0.5f, 0.5f,-0.5f,  0.0f,1.0f,

        // Right
         0.5f,-0.5f, 0.5f,  0.0f,0.0f,
         0.5f,-0.5f,-0.5f,  1.0f,0.0f,
         0.5f, 0.5f,-0.5f,  1.0f,1.0f,
         0.5f, 0.5f, 0.5f,  0.0f,1.0f,

        // Bottom
        -0.5f,-0.5f,-0.5f,  0.0f,0.0f,
         0.5f,-0.5f,-0.5f,  1.0f,0.0f,
         0.5f,-0.5f, 0.5f,  1.0f,1.0f,
        -0.5f,-0.5f, 0.5f,  0.0f,1.0f,

        // Top
        -0.5f, 0.5f, 0.5f,  0.0f,0.0f,
         0.5f, 0.5f, 0.5f,  1.0f,0.0f,
         0.5f, 0.5f,-0.5f,  1.0f,1.0f,
        -0.5f, 0.5f,-0.5f,  0.0f,1.0f
    };

    // Final vertex list with normals added
    std::vector<float> vertices;
    vertices.reserve(24 * 8);

    for (int face = 0; face < 6; face++) {
        float nx = faceNormals[face * 3 + 0];
        float ny = faceNormals[face * 3 + 1];
        float nz = faceNormals[face * 3 + 2];

        for (int v = 0; v < 4; v++) {
            int idx = (face * 20) + v * 5;

            // Position
            vertices.push_back(baseVerts[idx + 0]);
            vertices.push_back(baseVerts[idx + 1]);
            vertices.push_back(baseVerts[idx + 2]);

            // UV
            vertices.push_back(baseVerts[idx + 3]);
            vertices.push_back(baseVerts[idx + 4]);

            // Normal
            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);
        }
    }

    std::vector<unsigned int> indices = {
        0,1,2, 2,3,0,
        4,5,6, 6,7,4,
        8,9,10, 10,11,8,
        12,13,14, 14,15,12,
        16,17,18, 18,19,16,
        20,21,22, 22,23,20
    };

    Mesh cube(vertices, indices, vec3(cx, cy, cz));
    cube.Setup();
    return cube;
}

class UVRange {
	public:
    float uMin, vMin, uMax, vMax;
	//extra functions, might be helpful to auto-fit textures later...
	float GetWidth() const { return uMax - uMin; }
    float GetHeight() const { return vMax - vMin; }
};

// Helper to generate the 4 UV pairs for a square face from its range
std::vector<float> GetFaceUVs(const UVRange& range) {
    return {
        range.uMin, range.vMin, // Bottom Left
        range.uMax, range.vMin, // Bottom Right
        range.uMax, range.vMax, // Top Right
        range.uMin, range.vMax  // Top Left
    };
}

UVRange ComputeFittedUVRange(
    const UVRange& baseUV,
    float faceWidth,
    float faceHeight,
    float textureWidth,
    float textureHeight)
{
    float uSpan = faceWidth / textureWidth;
    float vSpan = faceHeight / textureHeight;

    return {
        baseUV.uMin,
        baseUV.vMin,
        baseUV.uMin + uSpan,
        baseUV.vMin + vSpan
    };
	//For autoUV?:
	//float faceWidth = 1.0f;   // size in world units
	//float faceHeight = 1.0f;  // size in world units
	//UVRange autoUV = ComputeFittedUVRange(baseUV, faceWidth, faceHeight, texW, texH);
}
UVRange AutoFitUVFromVertices(
    const std::array<vec3, 4>& faceVertices,
    float textureScaleU = 1.0f,
    float textureScaleV = 1.0f)
{
    float minX = faceVertices[0].x, maxX = faceVertices[0].x;
    float minY = faceVertices[0].y, maxY = faceVertices[0].y;

    for (const auto& v : faceVertices) {
        minX = std::min(minX, v.x);
        maxX = std::max(maxX, v.x);
        minY = std::min(minY, v.y);
        maxY = std::max(maxY, v.y);
    }
    // Map world-space to UVs using scale
    return {
        minX * textureScaleU,
        minY * textureScaleV,
        maxX * textureScaleU,
        maxY * textureScaleV
    };
}
// ------
static Mesh CreateRubikCubieMesh(
    const UVRange& frontUV, 
    const UVRange& backUV, 
    const UVRange& leftUV, 
    const UVRange& rightUV, 
    const UVRange& bottomUV, 
    const UVRange& topUV,
    float cx = 0.0f, float cy = 0.0f, float cz = 0.0f) 
{
    // POSITION DATA
    std::vector<float> positions = {
        // Front face (Z = +0.5)
        -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, 
        // Back face (Z = -0.5) 
        -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, 
        // Left face (X = -0.5)
        -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, 
        // Right face (X = +0.5)
         0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f, 
        // Bottom face (Y = -0.5)
        -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, 
        // Top face (Y = +0.5)
        -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f
		// Top face (Y = +0.5)
		//-0.5f,  0.5f, -0.5f, 0.5f,  0.5f, -0.5f, 0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f

    };
	std::vector<float> faceNormals = {
         0.0f,  0.0f,  1.0f, // Front Normal
         0.0f,  0.0f, -1.0f, // Back Normal
        -1.0f,  0.0f,  0.0f, // Left Normal
         1.0f,  0.0f,  0.0f, // Right Normal
         0.0f, -1.0f,  0.0f, // Bottom Normal
         0.0f,  1.0f,  0.0f  // Top Normal
    };
    // COMBINE POSITION, NORMAL AND UV DATA
    std::vector<float> vertices;
    std::vector<UVRange> allFaceUVs = 
	{frontUV, backUV, leftUV, rightUV, bottomUV, topUV};
    
    for (int i = 0; i < 6; ++i) {
        // Get the UVs for this face
        std::vector<float> faceUVs = GetFaceUVs(allFaceUVs[i]);
        // Get the Normal for this face
        float normalX = faceNormals[i * 3 + 0];
        float normalY = faceNormals[i * 3 + 1];
        float normalZ = faceNormals[i * 3 + 2];
        // Combine 4 positions (i*12 to i*12+11) with 4 UVs (i*8 to i*8+7)
        for (int j = 0; j < 4; ++j) {
            // Position 
            vertices.push_back(positions[i * 12 + j * 3 + 0]);
            vertices.push_back(positions[i * 12 + j * 3 + 1]);
            vertices.push_back(positions[i * 12 + j * 3 + 2]);
            // UV
            vertices.push_back(faceUVs[j * 2 + 0]);
            vertices.push_back(faceUVs[j * 2 + 1]);
			// Normal
            vertices.push_back(normalX);
            vertices.push_back(normalY);
            vertices.push_back(normalZ);
        }
    }

    std::vector<unsigned int> indices = {
        0, 1, 2, 2, 3, 0,    // Front (Indices 0-3)
        4, 5, 6, 6, 7, 4,    // Back (Indices 4-7)
        8, 9, 10, 10, 11, 8, // Left (Indices 8-11)
        12, 13, 14, 14, 15, 12, // Right (Indices 12-15)
        16, 17, 18, 18, 19, 16, // Bottom (Indices 16-19)
        20, 21, 22, 22, 23, 20  // Top (Indices 20-23)
    };

    Mesh cube(vertices, indices, vec3(cx, cy, cz));
    cube.Setup();
    return cube;
}

static Mesh CreateCircleMesh(float radius = 5.0f, int segments = 64, float y = 0.0f)
{
    std::vector<float> verts;
    std::vector<unsigned int> inds;

    // Center vertex
    verts.insert(verts.end(), {
        0.0f, y, 0.0f,     // position
        0.5f, 0.5f,        // UV
        0.0f, 1.0f, 0.0f   // normal (up)
    });

    // Ring vertices
    for (int i = 0; i <= segments; i++)
    {
        float a = (float)i / segments * 2.0f * PI;
        float x = cosf(a) * radius;
        float z = sinf(a) * radius;

        verts.insert(verts.end(), {
            x, y, z,
            (x / (radius * 2.0f)) + 0.5f,      // simple UV projection
            (z / (radius * 2.0f)) + 0.5f,
            0.0f, 1.0f, 0.0f                   // normal
        });
    }

    // Triangles
    for (int i = 1; i <= segments; i++)
    {
        inds.push_back(0);
        inds.push_back(i);
        inds.push_back(i + 1);
    }

    Mesh circle(verts, inds);
    circle.Setup();
    return circle;
}


//How to call? 
//Mesh cube = CreateCube(1.0f, 0.0f, -2.0f);
