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
	/*
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
	*/
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
static Mesh CreatePyramid(float cx = 0.0f, float cy = 0.0f, float cz = 0.0f) {
    std::vector<float> vertices = {
        // Base x,y,z + texCoord u,v
        -0.5f, 0.0f, -0.5f, 0.0f, 0.0f,
         0.5f, 0.0f, -0.5f, 1.0f, 0.0f,
         0.5f, 0.0f,  0.5f, 1.0f, 1.0f,
        -0.5f, 0.0f,  0.5f, 0.0f, 1.0f,
        // Apex
         0.0f, 1.0f,  0.0f, 0.5f, 0.5f
    };

    std::vector<unsigned int> indices = {
        // Base
        0, 1, 2, 2, 3, 0,
        // Sides
        0, 1, 4,
        1, 2, 4,
        2, 3, 4,
        3, 0, 4
    };

    Mesh pyramid(vertices, indices, (cx, cy, cz));
    pyramid.Setup();
    return pyramid;
}

static Mesh CreateFivePointedStar(float cx = 0.0f, float cy = 0.0f, float cz = 0.0f)
{
    std::vector<float> vertices;
    const float R = 1.0f;     // Outer radius
    const float r = 0.5f;     // Inner radius
    const int numPoints = 10;
    //const float PI = 3.14159265359f;

    // Center point (position + texture coords)
    vertices.push_back(0.0f);  // x
    vertices.push_back(0.0f);  // y
    vertices.push_back(0.0f);  // z
    vertices.push_back(0.5f);  // u
    vertices.push_back(0.5f);  // v

    // Perimeter points(rotating around a circle outer, then inner)
    for (int i = 0; i < numPoints; ++i) {
        float angle = i * (2 * PI / numPoints) - PI / 2.0f;
        float radius = (i % 2 == 0) ? R : r;
        float x = radius * cos(angle - 30.0f);
        float y = radius * sin(angle - 30.0f);

        // Texture space (0 to 1)
        float u = (x + R) / (2 * R);
        float v = (y + R) / (2 * R);

        vertices.push_back(x);     // x
        vertices.push_back(y);     // y
        vertices.push_back(0.0f);  // z
        vertices.push_back(u);     // u
        vertices.push_back(v);     // v
    }

    std::vector<unsigned int> indices;
    for (int i = 1; i <= 10; ++i) {
        indices.push_back(0);             // center
        indices.push_back(i);
        indices.push_back(i % 10 + 1);    // next point
    }

    Mesh star(vertices, indices, vec3(cx, cy, cz));
    star.Setup();
    return star;
}

static Mesh CreateCube(float cx = 0.0f, float cy = 0.0f, float cz = 0.0f) {
    std::vector<float> vertices = {
        // Position         // Texture Coords
        // Front face (Z = +0.5)
        -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, // 0
         0.5f, -0.5f,  0.5f,   1.0f, 0.0f, // 1
         0.5f,  0.5f,  0.5f,   1.0f, 1.0f, // 2
        -0.5f,  0.5f,  0.5f,   0.0f, 1.0f, // 3
        // Back face (Z = -0.5) (U,V flip for back face)
        -0.5f, -0.5f, -0.5f,   1.0f, 0.0f, // 4 
         0.5f, -0.5f, -0.5f,   0.0f, 0.0f, // 5
         0.5f,  0.5f, -0.5f,   0.0f, 1.0f, // 6
        -0.5f,  0.5f, -0.5f,   1.0f, 1.0f, // 7
        // Left face (X = -0.5)
        -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, // 8 (Same pos as 4)
        -0.5f, -0.5f,  0.5f,   1.0f, 0.0f, // 9 (Same pos as 0)
        -0.5f,  0.5f,  0.5f,   1.0f, 1.0f, // 10 (Same pos as 3)
        -0.5f,  0.5f, -0.5f,   0.0f, 1.0f, // 11 (Same pos as 7)
        // Right face (X = +0.5)
         0.5f, -0.5f,  0.5f,   0.0f, 0.0f, // 12 (Same pos as 1)
         0.5f, -0.5f, -0.5f,   1.0f, 0.0f, // 13 (Same pos as 5)
         0.5f,  0.5f, -0.5f,   1.0f, 1.0f, // 14 (Same pos as 6)
         0.5f,  0.5f,  0.5f,   0.0f, 1.0f, // 15 (Same pos as 2)
        // Bottom face (Y = -0.5)
        -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, // 16 (Same pos as 4)
         0.5f, -0.5f, -0.5f,   1.0f, 0.0f, // 17 (Same pos as 5)
         0.5f, -0.5f,  0.5f,   1.0f, 1.0f, // 18 (Same pos as 1)
        -0.5f, -0.5f,  0.5f,   0.0f, 1.0f, // 19 (Same pos as 0)

        // Top face (Y = +0.5)
        -0.5f,  0.5f,  0.5f,   0.0f, 0.0f, // 20 (Same pos as 3)
         0.5f,  0.5f,  0.5f,   1.0f, 0.0f, // 21 (Same pos as 2)
         0.5f,  0.5f, -0.5f,   1.0f, 1.0f, // 22 (Same pos as 6)
        -0.5f,  0.5f, -0.5f,   0.0f, 1.0f  // 23 (Same pos as 7)
    };

    std::vector<unsigned int> indices = {
        // Front
        0, 1, 2, 2, 3, 0,
        // Back
        4, 5, 6, 6, 7, 4,
        // Left
        8, 9, 10, 10, 11, 8,
        // Right
        12, 13, 14, 14, 15, 12,
        // Bottom
        16, 17, 18, 18, 19, 16,
        // Top
        20, 21, 22, 22, 23, 20
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
//UV Sphere(rings (longitude/slices) and stacks(latitude/segment))
//12*12 by default
static Mesh CreateSphere(float cx = 0.0f, float cy = 0.0f, float cz = 0.0f, float radius = 0.5f, int stacks = 12, int slices = 12) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    // The two poles are special cases
    for (int i = 0; i <= stacks; ++i) {
        float V = (float)i / stacks;             // v (texture coord) from 0 to 1
        float phi = V * PI;                      // Angle phi (latitude) from 0 to PI

        for (int j = 0; j <= slices; ++j) {
            float U = (float)j / slices;         // u (texture coord) from 0 to 1
            float theta = U * 2.0f * PI;         // Angle theta (longitude) from 0 to 2PI
            // Spherical to Cartesian coordinates
            float x = radius * std::sin(phi) * std::cos(theta);
            float y = radius * std::cos(phi);
            float z = radius * std::sin(phi) * std::sin(theta);

            // Position (x, y, z)
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            
            // Texture Coords (u, v)
            vertices.push_back(U);
            vertices.push_back(V);
        }
    }

    // Index calculation for a grid of quads (two triangles)
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            unsigned int p1 = i * (slices + 1) + j;
            unsigned int p2 = p1 + (slices + 1);
            unsigned int p3 = p2 + 1;
            unsigned int p4 = p1 + 1;
			//Triangles order
            // First triangle (quad's bottom-left)
            indices.push_back(p1);
            indices.push_back(p2);
            indices.push_back(p4);

            // Second triangle (quad's top-right)
            indices.push_back(p4);
            indices.push_back(p2);
            indices.push_back(p3);
        }
    }

    Mesh sphere(vertices, indices, vec3(cx, cy, cz));
    sphere.Setup();
    return sphere;
}

//How to call? 
//Mesh cube = CreateCube(1.0f, 0.0f, -2.0f);
//Mesh pyramid = CreatePyramid(0.0f, 0.0f, 0.0f);
