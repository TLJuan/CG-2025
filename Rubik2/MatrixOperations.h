#pragma once
#include <cmath>
#include <vector>
#include <iostream>
class vec3 {
	public:
    float x, y, z;
    vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
	float length() const { return std::sqrt(x * x + y * y + z * z); }
    //Helper functions
	vec3 normalize() const {
        float len = length();
        if (len > 1e-6f) return vec3(x / len, y / len, z / len);
        return vec3(0, 0, 0);
    }
    vec3 cross(const vec3& other) const {
        return vec3(y * other.z - z * other.y,
                    z * other.x - x * other.z,
                    x * other.y - y * other.x);
    }
    vec3 operator-(const vec3& other) const {
        return vec3(x - other.x, y - other.y, z - other.z);
    }
	vec3 operator+(const vec3& other) const {
        return vec3(x + other.x, y + other.y, z + other.z);
    }
    vec3 operator-() const {
        return vec3(-x, -y, -z);
    }
	bool operator<(const vec3& other) const {
		if (x != other.x) return x < other.x;
		if (y != other.y) return y < other.y;
		return z < other.z;
	}
};
class matrix4 {
	public:
    float m[16];

    matrix4() {
        Identity();
    }

    void Identity() {
        for (int i = 0; i < 16; ++i)
            m[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
//--- Transformations ---
    void RotateX(float angle) {
        float cosA = cos(angle);
        float sinA = sin(angle);
        Identity();
        m[5] = cosA; m[6] = sinA;
        m[9] = -sinA; m[10] = cosA;
    }
	
	void RotateY(float angle) {
		float cosA = cos(angle);
		float sinA = sin(angle);
		Identity();
		m[0] = cosA;  m[2] = -sinA;
		m[8] = sinA; m[10] = cosA;
	}
	
	void RotateZ(float angle) {
		float cosA = cos(angle);
		float sinA = sin(angle);
		Identity();
		m[0] = cosA;  m[1] = sinA;
		m[4] = -sinA;  m[5] = cosA;
	}
	
	void Rotate(char axis, float angle) {
		switch (axis) {
			case 'x': case 'X': RotateX(angle); break;
			case 'y': case 'Y': RotateY(angle); break;
			case 'z': case 'Z': RotateZ(angle); break;
			default: Identity(); break;
		}
	}
	
    void Scale(float sx, float sy, float sz) {
        Identity();
        m[0] = sx; m[5] = sy; m[10] = sz;
    }

    void Translate(float tx, float ty, float tz) {
        Identity();
        m[12] = tx; m[13] = ty; m[14] = tz;
    }
	void Translate(const vec3& v) {
		Translate(v.x, v.y, v.z);
		}

	void InverseTranslate(float tx, float ty, float tz) {
		Identity();
		m[12] = -tx;
		m[13] = -ty;
		m[14] = -tz;
	}

	void InverseScale(float sx, float sy, float sz) {
		Identity();
		m[0] = 1.0f / sx;
		m[5] = 1.0f / sy;
		m[10] = 1.0f / sz;
	}

	void InverseRotateX(float angle) { RotateX(-angle); }
	void InverseRotateY(float angle) { RotateY(-angle); }
	void InverseRotateZ(float angle) { RotateZ(-angle); }

    matrix4 operator*(const matrix4& other) const {
        matrix4 result;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j) {
                result.m[i + j * 4] = 0.0f;
                for (int k = 0; k < 4; ++k)
                    result.m[i + j * 4] += m[i + k * 4] * other.m[k + j * 4];
            }
        return result;
    }
	// Matrix-vector multiplication (column-major order)
	void multVect(const float m[16], float& x, float& y, float& z)
	{
		float tx = m[0]*x + m[4]*y + m[8]*z + m[12];  // x
		float ty = m[1]*x + m[5]*y + m[9]*z + m[13];  // y
		float tz = m[2]*x + m[6]*y + m[10]*z + m[14]; // z
		float tw = m[3]*x + m[7]*y + m[11]*z + m[15]; // w

		//Homogenize if w != 1
		if (tw != 0.0f && tw != 1.0f) {
			tx /= tw;
			ty /= tw;
			tz /= tw;
		}

		x = tx;
		y = ty;
		z = tz;
	}
	bool Invert()
	{
		matrix4 inv;
		// Upper 3x3 matrix
		float r00 = m[0], r01 = m[4], r02 = m[8];
		float r10 = m[1], r11 = m[5], r12 = m[9];
		float r20 = m[2], r21 = m[6], r22 = m[10];

		// Determinant of the 3x3 matrix
		float det = r00 * (r11 * r22 - r12 * r21) -
					r01 * (r10 * r22 - r12 * r20) +
					r02 * (r10 * r21 - r11 * r20);
		if (fabs(det) < 1e-6f) {
			std::cerr << "Matrix not invertible (det ~ 0)\n";
			return false;
		}
		float invDet = 1.0f / det;
		// Invert 3x3
		inv.m[0] =  (r11 * r22 - r12 * r21) * invDet;
		inv.m[1] = -(r10 * r22 - r12 * r20) * invDet;
		inv.m[2] =  (r10 * r21 - r11 * r20) * invDet;
		inv.m[3] = 0.0f;

		inv.m[4] = -(r01 * r22 - r02 * r21) * invDet;
		inv.m[5] =  (r00 * r22 - r02 * r20) * invDet;
		inv.m[6] = -(r00 * r21 - r01 * r20) * invDet;
		inv.m[7] = 0.0f;

		inv.m[8]  =  (r01 * r12 - r02 * r11) * invDet;
		inv.m[9]  = -(r00 * r12 - r02 * r10) * invDet;
		inv.m[10] =  (r00 * r11 - r01 * r10) * invDet;
		inv.m[11] = 0.0f;

		// Invert translation
		float tx = m[12], ty = m[13], tz = m[14];

		inv.m[12] = -(inv.m[0] * tx + inv.m[4] * ty + inv.m[8] * tz);
		inv.m[13] = -(inv.m[1] * tx + inv.m[5] * ty + inv.m[9] * tz);
		inv.m[14] = -(inv.m[2] * tx + inv.m[6] * ty + inv.m[10] * tz);
		inv.m[15] = 1.0f;

		*this = inv;
		return true;
	}
	
	void TransformVertices(std::vector<float>& vert) {
    
		for (size_t i = 0; i < vert.size(); i += 5) {
			float& x = vert[i];
			float& y = vert[i + 1];
			float& z = vert[i + 2];
			multVect(m, x, y, z);
		}
	}

    void PrintMatrix() const {
        for (int i = 0; i < 4; ++i) {
            std::cout << m[i] << " " << m[i + 4] << " " << m[i + 8] << " " << m[i + 12] << "\n";
        }
    }
	/*
	LookAT(): 
		- transforms world coordinates into camera coordinates
		- places the camera in the desired position and orientation
		- EYE: Camera coordinates
		- TARGET: Point camera is "lookingAt"(for orientation)
		- UP: vector indicating the "up" direction for the camera 
			((0,1,0) for world-space UP)
		- Steps.
			First calculate camera local axis(z/x/y cam)
		----
		View Matrix result? 
			Combination of rotation+translation
			this translates the world so the camera is the origin
			aligning the camera axis with the world axis
		M View =
		[x cam.x][x cam.y][x cam.z][0]    [1][0][0][- eye.x]
		[y cam.x][y cam.y][y cam.z][0] *  [0][1][0][- eye.y]
		[z cam.x][z cam.y][z cam.z][0]    [0][0][1][- eye.z]
		[0]      [0]      [0]      [1]    [0][0][0][1]
	*/
	
	void LookAt(const vec3& eye, const vec3& target, const vec3& up) {
		// 1. Compute the camera basis vectors (right-handed)
		vec3 z_cam = (eye - target).normalize(); // Points away from target
		vec3 x_cam = up.cross(z_cam).normalize(); // Right vector
		vec3 y_cam = z_cam.cross(x_cam); // Up vector

		Identity();

		// 2. Set Rotation/Basis vectors (COLUMN-MAJOR)
		m[0] = x_cam.x; m[1] = y_cam.x; m[2] = z_cam.x;  // Column 0 (x_cam)
		m[4] = x_cam.y; m[5] = y_cam.y; m[6] = z_cam.y;  // Column 1 (y_cam)
		m[8] = x_cam.z; m[9] = y_cam.z; m[10] = z_cam.z; // Column 2 (z_cam)
		
		// 3. Set Translation part (COLUMN-MAJOR)
		m[12] = -(x_cam.x * eye.x + x_cam.y * eye.y + x_cam.z * eye.z); // -dot(x_cam, eye)
		m[13] = -(y_cam.x * eye.x + y_cam.y * eye.y + y_cam.z * eye.z); // -dot(y_cam, eye)
		m[14] = -(z_cam.x * eye.x + z_cam.y * eye.y + z_cam.z * eye.z); // -dot(z_cam, eye)
		m[15] = 1.0f;
	}
	
	/*
ProjectionMatrix transforms 3D view coordinates into 2D homogeneous
Perspective makes farther objects appear smaller
Perspective():
	- FieldOfView(FOV): Angle - extent of the visible world space.
	- AspectRatio(A): ratio of the viewport(width/height)
	- NearPlane(near): Distance to Near "Clipping" plane
	- FarPlane(far): Distance to Far "Clipping" plane
	---
	ProjMat
	[1/(A*tan(FOV/2))][0]             [0]                     [0]
	[0]               [1/(tan(FOV/2))][0]                     [0]
	[0]               [0]             [-(Far+Near)/(Far-Near)][-(2*Far*Near)/(Far-Near)]
	[0]               [0]             [-1]                    [0]
	*/
	void Perspective(float fovRadians, float aspectRatio, float nearPlane, float farPlane) {
		float tanHalfFov = tan(fovRadians / 2.0f);
		float f = 1.0f / tanHalfFov; // Cotangent(FOV/2)
		float n = nearPlane;
		float ff = farPlane;

		Identity(); // Start with an identity matrix to clear any previous data

		// Column 0
		m[0] = f / aspectRatio;
		m[1] = 0.0f;
		m[2] = 0.0f;
		m[3] = 0.0f;

		// Column 1
		m[4] = 0.0f;
		m[5] = f;
		m[6] = 0.0f;
		m[7] = 0.0f;

		// Column 2
		m[8] = 0.0f;
		m[9] = 0.0f;
		m[10] = -(ff + n) / (ff - n);
		m[11] = -1.0f; // Necessary for homogeneous clip space division

		// Column 3
		m[12] = 0.0f;
		m[13] = 0.0f;
		m[14] = -(2.0f * ff * n) / (ff - n);
		m[15] = 0.0f;
	}
	/*
	Orthographic requires 6 parametes
	Left, Right(X axis bounds)
	Top, Bottom(Y axis bounds)
	Near, Far(Z axis bounds)
	
	And involves two main steps
	Translation: Center of viewing volume moved to origin(0,0,0,)
	Scaling: Scales the volume to match the NDC cube(has a lenght of 2)
	M Orthographic=
	[2/(R-L)][0]      [0]        [-(R+L)/(R-L)]
	[0]      [2/(T-B)][0]        [-(T+B)/(T-B)]
	[0]      [0]      [2/(F/N)]  [-(F+N)/(F-N)]
	[0]      [0]      [0]        [1]
	*/
	void Orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
		Identity();

		// Check for valid bounds to prevent division by zero
		if (left == right || bottom == top || nearPlane == farPlane) {
			std::cerr << "Orthographic: Invalid bounds (R-L, T-B, or F-N is zero).\n";
			return;
		}

		float width = right - left;
		float height = top - bottom;
		float depth = farPlane - nearPlane;

		// Column 0
		m[0] = 2.0f / width;

		// Column 1
		m[5] = 2.0f / height;

		// Column 2
		m[10] = -2.0f / depth; // Note the negative sign for Z-mapping

		// Column 3 (Translation)
		m[12] = -(right + left) / width;
		m[13] = -(top + bottom) / height;
		m[14] = -(farPlane + nearPlane) / depth;
		m[15] = 1.0f;
	}
	
};