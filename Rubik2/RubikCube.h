#pragma once
#include <vector>
#include <map>
#include <cmath>
#include "Shader.h"
//For random movements(and solver movements, possibly)
#include <queue>
#include <random>
#include <string>

// Map cube face letters to ANSI color codes
std::string GetColor(char c) {
    switch (c) {
        case 'U': return "\033[33m"; // White switched
        case 'D': return "\033[37m"; // Yellow switched
        case 'L': return "\033[35m"; // Magenta (closest to orange)
        case 'R': return "\033[31m"; // Red
        case 'F': return "\033[32m"; // Green
        case 'B': return "\033[34m"; // Blue
        default:  return "\033[0m";  // Reset
    }
}

// Print a single colored character
void PrintColoredChar(char c) {
    std::cout << GetColor(c) << c << "\033[0m";
}
// Texture Atlas UV Ranges
const float U_SIZE = 0.25f;
const float V_SIZE = 1.0f / 3.0f;

const UVRange UV_BLACK  = {0.0f, 0.0f, 0.0f, 0.0f}; // Inner/uncolored faces
const UVRange UV_ORANGE = {0.0f, V_SIZE, U_SIZE, 2.0f * V_SIZE};    // (0, 1) -> Left
const UVRange UV_BLUE   = {U_SIZE, V_SIZE, 2.0f * U_SIZE, 2.0f * V_SIZE};  // (1, 1) -> Back
const UVRange UV_RED    = {2.0f * U_SIZE, V_SIZE, 3.0f * U_SIZE, 2.0f * V_SIZE}; // (2, 1) -> Right
const UVRange UV_GREEN  = {3.0f * U_SIZE, V_SIZE, 4.0f * U_SIZE, 2.0f * V_SIZE}; // (3, 1) -> Front

const UVRange UV_WHITE  = {U_SIZE, 2.0f * V_SIZE, 2.0f * U_SIZE, 3.0f * V_SIZE}; // (1, 0) -> Top
const UVRange UV_YELLOW = {U_SIZE, 0.0f, 2.0f * U_SIZE, V_SIZE};                // (1, 2) -> Bottom

const float CUBIE_GAP = 0.05f; //small distance between cubes(cubies), so they do not overlap.
const float CUBIE_OFFSET = 1.0f + CUBIE_GAP;
// Colors list
//enum class FaceColor { WHITE, YELLOW, GREEN, BLUE, ORANGE, RED, BLACK }; 
// BLACK for inward faces.

class Cubie {
	public:
    int gridPos[3]; 
    matrix4 modelMatrix;
    //std::map<vec3, FaceColor> faceColors; 
	Mesh mesh;
	
    Cubie(int x, int y, int z, const Mesh& cubieMeshInstance): mesh(cubieMeshInstance) {
        gridPos[0] = x; gridPos[1] = y; gridPos[2] = z;
        modelMatrix = mesh.modelMatrix;
    }
    // pivot position (center of the cube)
    vec3 getCenter() const {
        return vec3(gridPos[0] * CUBIE_OFFSET,
                    gridPos[1] * CUBIE_OFFSET,
                    gridPos[2] * CUBIE_OFFSET);
    }
};
enum class Axis { X, Y, Z };
class Move {
/*
Every rotation has a Layer, Axis(X or Y) and a direction(+/- 90 degrees(PI/2 in radians))
We use this structure to:
 - Track the movements. Both the used for Scramble and those needed to Solve.
 - Feed those movements to our Rotate or RotateSnap
Basically, to any automatic section
*/
public:
    Axis axis;
    int layer;
    int direction;
};

class RubikCube {
private:
    // The 3x3x3 grid of cubies
    std::vector<Cubie> cubies;
	//Rubik State Tracker - Kociemba style - Solved/Initial Cube String.
	std::string RubikState = "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB"; 
	std::string MoveList;
	std::string SolutionList;
	std::queue<Move> MoveQueue;
	std::queue<std::string> moveQueue;
    //Rubik's Cube center
    vec3 cubeCenter = vec3(0.0f, 0.0f, 0.0f);
	// Outline Drawing
	bool isSliceSelected = false;
    Axis selectedAxis = Axis::X; 
    int selectedLayer = 1;
	// Animation State & Rotation Variables
    float currentRotationAngle = 0.0f;
    float targetRotationAngle = 0.0f; // +/- 90 degrees (PI/2) - Need more? Call RotateAgain.
	const float ANIMATION_DURATION = 0.25f; // Target time in seconds
    const float ROTATION_SPEED = (PI / 2.0f) / ANIMATION_DURATION;
public:
	//bool isScrambling = false;
	bool isRotating = false; //Prevent new rotations while one is in progress
	bool executingSolution = false;
	float direction = 1.0f; // plus or minus
	RubikCube() {
        InitializeCubies();
		std::cout << "Cube State: " << GetState() << "\n";
    }
	// Iterate through X, Y, Z centered at (0, 0, 0)
    void InitializeCubies() {
        for (int x = -1; x <= 1; ++x) {
            for (int y = -1; y <= 1; ++y) {
                for (int z = -1; z <= 1; ++z) {
                    if (x == 0 && y == 0 && z == 0) continue; // Skip center
					// Define the UV ranges for this cubie based on its position
					UVRange front = (z == 1) ? UV_GREEN : UV_BLACK;
					UVRange back  = (z == -1) ? UV_BLUE : UV_BLACK;
					UVRange left  = (x == -1) ? UV_ORANGE : UV_BLACK;
					UVRange right = (x == 1) ? UV_RED : UV_BLACK;
					UVRange bottom = (y == -1) ? UV_YELLOW : UV_BLACK;
					UVRange top   = (y == 1) ? UV_WHITE : UV_BLACK;
					// Cubie instance
					Mesh cubieMeshInstance = CreateRubikCubieMesh(
						front, back, left, right, bottom, top,
						x * CUBIE_OFFSET, y * CUBIE_OFFSET, z * CUBIE_OFFSET
					);
                    
					Cubie newCubie(x, y, z, cubieMeshInstance);
					newCubie.modelMatrix.Translate(x * CUBIE_OFFSET, y * CUBIE_OFFSET, z * CUBIE_OFFSET);
                    cubies.push_back(std::move(newCubie));
                }
            }
        }
		
	//	for (auto& c : cubies) {
    //std::cout << "Cubie (" << c.mesh.center.x << ", " << c.mesh.center.y << ", " << c.mesh.center.z << ")\n";}
		
    }
	// --- State Tracker Logic ---
	std::string GetState()
	{
		return RubikState;
	}	
	std::string GetMoveList()
	{
		return MoveList;
	}
	std::string GetSolutionList()
	{
		return SolutionList;
	}
	void SetSolutionList(std::string found)
	{
		if(found.empty() || found == "")
		{
			MoveList.clear();
			return;
		}
		SolutionList = found;
		FillMoveQueue();
	}
	void FillMoveQueue()
	{
		
		return;
	}
	void PrintCubeNet(const std::string& state)
	{
		auto face = [&](int i){ return state.substr(i * 9, 9); };

		std::string U = face(0);
		std::string R = face(1);
		std::string F = face(2);
		std::string D = face(3);
		std::string L = face(4);
		std::string B = face(5);

		auto row = [](const std::string& s, int r){ return s.substr(r * 3, 3); };

		auto printRow = [&](const std::string& s, int r) {
			for (char c : row(s, r)) PrintColoredChar(c);
		};// Print top (U)
		std::cout << "      "; printRow(U,0); std::cout << "\n";
		std::cout << "      "; printRow(U,1); std::cout << "\n";
		std::cout << "      "; printRow(U,2); std::cout << "\n";

		// Print middle: L, F, R, B
		for (int r = 0; r < 3; r++) {
			printRow(L,r); std::cout << " ";
			printRow(F,r); std::cout << " ";
			printRow(R,r); std::cout << " ";
			printRow(B,r); std::cout << "\n";
		}

		// Print bottom (D)
		std::cout << "      "; printRow(D,0); std::cout << "\n";
		std::cout << "      "; printRow(D,1); std::cout << "\n";
		std::cout << "      "; printRow(D,2); std::cout << "\n";
	}
	void PrintCubeNetWithIndex(const std::string& state)
	{
		auto face = [&](int i){ return state.substr(i * 9, 9); };

		std::string U = face(0);
		std::string R = face(1);
		std::string F = face(2);
		std::string D = face(3);
		std::string L = face(4);
		std::string B = face(5);

		auto row = [](const std::string& s, int r){ return s.substr(r * 3, 3); };

		auto printRow = [&](int faceIndex, const std::string& s, int r, int baseOffset = 0) {
			for (int i = 0; i < 3; i++) {
				int idx = faceIndex * 9 + r * 3 + i;
				char c = s[r * 3 + i];
				PrintColoredChar(c);
				std::cout << "(" << idx << ")";  // show global index
			}
		};

		// --- Print top (U) ---
		for (int r = 0; r < 3; r++) {
			std::cout << "      ";
			printRow(0, U, r);
			std::cout << "\n";
		}

		// --- Print middle: L, F, R, B ---
		for (int r = 0; r < 3; r++) {
			printRow(4, L, r); std::cout << " ";
			printRow(2, F, r); std::cout << " ";
			printRow(1, R, r); std::cout << " ";
			printRow(5, B, r);
			std::cout << "\n";
		}

		// --- Print bottom (D) ---
		for (int r = 0; r < 3; r++) {
			std::cout << "      ";
			printRow(3, D, r);
			std::cout << "\n";
		}
	}
	std::string ToKociemba(const std::string state) {
		std::string out;
		out.reserve(54);

		// U
		for (int i = 0; i < 9; i++) out.push_back(state[0 + i]);
		// R
		for (int i = 0; i < 9; i++) out.push_back(state[9 + i]);
		// F
		for (int i = 0; i < 9; i++) out.push_back(state[18 + i]);
		// D
		for (int i = 0; i < 9; i++) out.push_back(state[27 + i]);
		// L
		for (int i = 0; i < 9; i++) out.push_back(state[36 + i]);
		// B (mirrored)
		int backMap[9] = {47,46,45, 50,49,48, 53,52,51};
		for (int i = 0; i < 9; i++) out.push_back(state[backMap[i]]);
		return out;
	}

	std::string getMoveString(Axis axis, int layer, float direction) {
		//After a movement, return the movement type(F, R, B, etc)
		//Center(layer 0) is a Special Case(Equals two movements)
		if (layer == 0) {
			// Return a special marker for decomposition in FinalizeSliceRotation
			if (axis == Axis::X && direction < 0) return "M"; //Middle Slice      - X axis
			if (axis == Axis::X && direction > 0) return "M'";
			if (axis == Axis::Y && direction < 0) return "S'"; //"Standing" slice  - Y axis
			if (axis == Axis::Y && direction > 0) return "S";
			if (axis == Axis::Z && direction < 0) return "E"; //Equator Slice     - Z axis
			if (axis == Axis::Z && direction > 0) return "E'";
		}
		// FACE moves (layer = -1 or 1)
		std::string face = "";
		int layer_abs = std::abs(layer);
		if (axis == Axis::X) { // X-axis: Right (Layer 1) or Left (Layer -1)
			face = (layer == 1) ? "R" : "L";
		} else if (axis == Axis::Y) { // Y-axis: Up (Layer 1) or Down (Layer -1)
			face = (layer == 1) ? "U" : "D";
		} else if (axis == Axis::Z) { // Z-axis: Front (Layer 1) or Back (Layer -1)
			face = (layer == 1) ? "F" : "B";
		}
		// For 90 degree turns (clockwise or counter-clockwise)
		// R, U, F, D, L, B notation is defined as a Clockwise turn when looking at the face.
		// L and B faces are opposite the positive axes, clockwise becomes counter-intuitive.
		if (face == "R" || face == "U" || face == "F") {
			// Positive faces (R, U, F): Clockwise is Positive Direction (+1.0f)
			if (direction > 0) return face;   
			else return face + "'";           
		} 
		// face == "L" || face == "D" || face == "B"
		else { 
			// Negative faces (L, D, B): Clockwise is Negative Direction (-1.0f)
			if (direction > 0) return face + "'"; 
			else return face;                  
		}
	}
	
	
	
	//-----------------------------------
	// --- Rotation Logic ---
	void SelectSlice(Axis axis, int layer) {
		if (isRotating) return;
        isSliceSelected = true;
        selectedAxis = axis;
        selectedLayer = layer;
    }
    void DeselectSlice() {
        isSliceSelected = false;
    }
    // Rotates the slice dependent on the axis and selected layer
	void RotateSlice(Axis axis, int layer) {
		if (isRotating) return;
		isRotating = true;
		currentRotationAngle = 0.0f;
		selectedAxis = axis; //rotationAxis
		selectedLayer = layer;
		// Target is always 90 degrees in the specified direction (+/- PI/2)
		targetRotationAngle = direction * (PI / 2.0f); 	
	}
	//Snap Rotation - Rotate without animation.
    void RotateSliceSnap(Axis axis, int layer, float angleRadians = 90.0f) {
		//Should not rotate unless we are allowed to select a slice.
		//At least not manually...
			if(isSliceSelected = false) return; 
        matrix4 rotationMat; 
        switch (axis) {
            case Axis::X: rotationMat.RotateX(angleRadians*direction); break;
            case Axis::Y: rotationMat.RotateY(angleRadians*direction); break;
            case Axis::Z: rotationMat.RotateZ(angleRadians*direction); break;
        }
		
        int axisIndex = (axis == Axis::X) ? 0 : (axis == Axis::Y) ? 1 : 2;
        // Find and rotate all 9 cubies
        for (auto& cubie : cubies) {
            if (cubie.gridPos[axisIndex] == layer) {
                cubie.modelMatrix = rotationMat * cubie.modelMatrix;
            }
        }
		
		//Update GRID  of CUBIES
		FinalizeSliceRotation(selectedAxis, selectedLayer, direction);
    }
    
	// In RubikCube.h, inside the RubikCube class
	void Update(float dt) {
		
		if (executingSolution && !isRotating)
		{
			if (!moveQueue.empty())
			{
				std::string nextMove = moveQueue.front();
				moveQueue.pop();

				AdjustRotationValues(nextMove);
				//std::cout << "Executing move: " << nextMove << "\n";
			}
			else
			{
				// No more moves
				executingSolution = false;
				MoveList.clear();
				std::cout << "Solution complete!\n";
			}
		}
		
		
		if (!isRotating) return;
		float rotationAmount = ROTATION_SPEED * dt;
		// Clockwise or Counter-clockwise?
		float sign = (targetRotationAngle > 0) ? 1.0f : -1.0f;
		// Don't go over the target angle
		// If we do, then SNAP the remaining rotation and STOP
		if (std::abs(currentRotationAngle) + rotationAmount >= std::abs(targetRotationAngle)) {
			rotationAmount = std::abs(targetRotationAngle) - std::abs(currentRotationAngle);
			isRotating = false;
			// FOR THE SOLVER - UPDATE THE TRACKER POSITION
			FinalizeSliceRotation(selectedAxis, selectedLayer, direction);
		}
		ApplyFrameRotation(selectedAxis, selectedLayer, rotationAmount * sign);
		currentRotationAngle += rotationAmount;
	}
	void ApplyFrameRotation(Axis axis, int layer, float angle) {
		matrix4 rotationMat;
		switch (axis) {
			case Axis::X: rotationMat.RotateX(angle); break;
			case Axis::Y: rotationMat.RotateY(angle); break;
			case Axis::Z: rotationMat.RotateZ(angle); break;
		}
		int axisIndex = (axis == Axis::X) ? 0 : (axis == Axis::Y) ? 1 : 2;
		// Apply small rotation to the model matrix
		for (auto& cubie : cubies) {
			if (cubie.gridPos[axisIndex] == layer) {
				cubie.modelMatrix = rotationMat * cubie.modelMatrix;
			}
		}
	}
	// Update the internal gridPos after a full 90-degree 
	void FinalizeSliceRotation(Axis axis, int layer, float fullAngle) {
		int axisIndex = (axis == Axis::X) ? 0 : (axis == Axis::Y) ? 1 : 2;
		for (auto& cubie : cubies) {
			if (cubie.gridPos[axisIndex] == layer) {
				//Update Layer assignment
				int oldX = cubie.gridPos[0];
				int oldY = cubie.gridPos[1];
				int oldZ = cubie.gridPos[2];
				switch (axis) {
					case Axis::X:
						if (fullAngle > 0) {
							cubie.gridPos[1] = -oldZ;
							cubie.gridPos[2] = oldY;
						} else {
							cubie.gridPos[1] = oldZ;
							cubie.gridPos[2] = -oldY;
						}
						break;
					case Axis::Y:
						if (fullAngle > 0) {
							cubie.gridPos[0] = oldZ;
							cubie.gridPos[2] = -oldX;
						} else {
							cubie.gridPos[0] = -oldZ;
							cubie.gridPos[2] = oldX;
						}
						break;
					case Axis::Z:
						if (fullAngle > 0) {
							cubie.gridPos[0] = -oldY;
							cubie.gridPos[1] = oldX;
						} else {
							cubie.gridPos[0] = oldY;
							cubie.gridPos[1] = -oldX;
						}
						break;
				}
				
			}
		}
		//Update RubikState string
				std::string move = getMoveString(axis, layer, fullAngle);
				std::cout << "MoveMark " << move << std::endl;
				if (move == "M") {
					//std::cout << "\n --- MIDDLE MOVED ---  \n";
					MoveList += "L' ";
					MoveList += "R ";
					//Special move X
					// Rotate all cubies around X axis
					/*for (auto& cubie : cubies) {
						int oldY = cubie.gridPos[1];
						int oldZ = cubie.gridPos[2];
						cubie.gridPos[1] = -oldZ;
						cubie.gridPos[2] = oldY;
					}*/
					//MoveList += "x ";
				}
				else if(move == "M'")
				{
					MoveList += "L ";
					MoveList += "R' ";
					
					// same as above but opposite sign
					/*for (auto& cubie : cubies) {
						int oldY = cubie.gridPos[1];
						int oldZ = cubie.gridPos[2];
						cubie.gridPos[1] = oldZ;
						cubie.gridPos[2] = -oldY;
					}*/
					
				}
				else if(move == "S")
				{
					//std::cout << "\n --- STANDING MOVED ---  \n";
					MoveList += "U' ";
					MoveList += "D ";
					
					// Rotate all cubies around Y axis
					/*for (auto& cubie : cubies) {
						int oldY = cubie.gridPos[0];
						int oldZ = cubie.gridPos[2];
						cubie.gridPos[0] = oldZ;
						cubie.gridPos[2] = -oldY;
					}*/
					
				}
				else if(move == "S'")
				{
					MoveList += "U ";
					MoveList += "D' ";
					/*for (auto& cubie : cubies) {
						int oldY = cubie.gridPos[0];
						int oldZ = cubie.gridPos[2];
						cubie.gridPos[0] = -oldZ;
						cubie.gridPos[2] = oldY;
					}*/
				}
				else if(move == "E")
				{
					//std::cout << "\n --- EQUATOR MOVED ---  \n";
					MoveList += "F ";
					MoveList += "B' ";
					/*for (auto& cubie : cubies) {
						int oldY = cubie.gridPos[0];
						int oldZ = cubie.gridPos[1];
						cubie.gridPos[0] = oldZ;
						cubie.gridPos[1] = -oldY;
					}*/
				}
				else if(move == "E'")
				{
					MoveList += "F' ";
					MoveList += "B ";
					
					/*for (auto& cubie : cubies) {
						int oldY = cubie.gridPos[0];
						int oldZ = cubie.gridPos[1];
						cubie.gridPos[0] = -oldZ;
						cubie.gridPos[1] = oldY;
					}*/
				}
				else
				{
					MoveList += move + " ";
				}
	}
	
	// --- State and Rendering ---
	void Draw(const Shader& shader) {
		for (const auto& cubie : cubies) {
			bool isCubieSelected = false;
			if (isSliceSelected) {
				int axisIndex = (selectedAxis == Axis::X) ? 0 : (selectedAxis == Axis::Y) ? 1 : 2;
				if (cubie.gridPos[axisIndex] == selectedLayer) {
					isCubieSelected = true;
				}
			}
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glLineWidth(1.0f);
			shader.setMat4("model", cubie.modelMatrix);
			shader.setInt("mode", 0);
			glBindVertexArray(cubie.mesh.VAO); 
			glDrawElements(GL_TRIANGLES, cubie.mesh.indices.size(), GL_UNSIGNED_INT, 0);

			//  Draw Outline
			if (isCubieSelected) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glLineWidth(5.0f); 
				shader.setInt("mode", 1);
				
				glDrawElements(GL_TRIANGLES, cubie.mesh.indices.size(), GL_UNSIGNED_INT, 0);
			}
		}
		//glBindVertexArray(0);
	}
	void SwitchDirection(){
		direction *= -1;
	}
	
	void AdjustRotationValues(std::string i) {

		if (i == "U") {
			if (direction < 0) SwitchDirection(); // Debe ser 1.0
			RotateSlice(Axis::Y, 1);
		}
		else if (i == "U'") {
			if (direction > 0) SwitchDirection(); // Debe ser -1.0
			RotateSlice(Axis::Y, 1);
		}
		else if (i == "R") {
			if (direction < 0) SwitchDirection(); // Debe ser 1.0
			RotateSlice(Axis::X, 1);
		}
		else if (i == "R'") {
			if (direction > 0) SwitchDirection(); // Debe ser -1.0
			RotateSlice(Axis::X, 1);
		}
		else if (i == "F") {
			if (direction < 0) SwitchDirection(); // Debe ser 1.0
			RotateSlice(Axis::Z, 1);
		}
		else if (i == "F'") {
			if (direction > 0) SwitchDirection(); // Debe ser -1.0
			RotateSlice(Axis::Z, 1);
		}

		else if (i == "D") {
			if (direction > 0) SwitchDirection(); // Debe ser -1.0
			RotateSlice(Axis::Y, -1);
		}
		else if (i == "D'") {
			if (direction < 0) SwitchDirection(); // Debe ser 1.0
			RotateSlice(Axis::Y, -1);
		}
		else if (i == "L") {
			if (direction > 0) SwitchDirection(); // Debe ser -1.0
			RotateSlice(Axis::X, -1);
		}
		else if (i == "L'") {
			if (direction < 0) SwitchDirection(); // Debe ser 1.0
			RotateSlice(Axis::X, -1);
		}
		else if (i == "B") {
			if (direction > 0) SwitchDirection(); // Debe ser -1.0
			RotateSlice(Axis::Z, -1);
		}
		else if (i == "B'") {
			if (direction < 0) SwitchDirection(); // Debe ser 1.0
			RotateSlice(Axis::Z, -1);
		}
		else {
			std::cout << "--------------- INVALID MOVE: " << i << " --------------- \n";
		}
	}
    
	std::vector<std::string> ParseMoves(const std::string& s)
	{
		std::vector<std::string> moves;
		std::string token;

		for (size_t i = 0; i < s.size(); ++i)
		{
			if (std::isalpha(s[i])) 
			{
				char face = s[i];
				token = face;
				// Check next character
				if (i + 1 < s.size())
				{
					// Prime move (R')
					if (s[i+1] == '\'')
					{
						token = std::string(1, face) + "'";
						moves.push_back(token);
						i++; // skip '
						continue;
					}
					// Double move
					if (s[i+1] == '2')
					{
						// push the face twice
						moves.push_back(std::string(1, face));
						moves.push_back(std::string(1, face));
						i++; // skip 2
						continue;
					}
				}
				// Normal move (R)
				moves.push_back(token);
			}
		}

		return moves;
	}

	/*std::vector<std::string> ParseMoves(std::string& s)
	{
		std::vector<std::string> moves;
		std::string token;
		
		for (size_t i = 0; i < s.size(); ++i) 
		{
			if (std::isalpha(s[i])) { 
				token = s[i];

				if (i + 1 < s.size() && s[i+1] == '\'') {
					token += '\'';
					i++;
				}

				moves.push_back(token);
			}
		}

		return moves;
	}*/
	
	void StartExecutingSolution(const std::vector<std::string>& moves)
	{
		while (!moveQueue.empty()) moveQueue.pop(); // clear old data
		for (const auto& m : moves)
			moveQueue.push(m);

		executingSolution = true;
	}

	
	void ExecuteSolution(const std::vector<std::string>& moves)
	{
		for (const auto& move : moves)
		{
			std::cout << "Executing: " << move << "\n";
			AdjustRotationValues(move);
		}
	}


};