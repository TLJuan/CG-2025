import sys
import pycuber as pc
import kociemba

color_map = {
    'white': 'U',
    'red': 'R',
    'green': 'F',
    'yellow': 'D',
    'orange': 'L',
    'blue': 'B'
}

def cube_to_facelets(cube):
    # Identify which color belongs to which Kociemba face
    centers = {
        'U': cube.get_face('U')[1][1].colour,
        'R': cube.get_face('R')[1][1].colour,
        'F': cube.get_face('F')[1][1].colour,
        'D': cube.get_face('D')[1][1].colour,
        'L': cube.get_face('L')[1][1].colour,
        'B': cube.get_face('B')[1][1].colour
    }

    # Reverse-map colors → face letters
    color_to_face = {v: k for k, v in centers.items()}

    facelets = ""
    for face in ['U', 'R', 'F', 'D', 'L', 'B']:
        for row in cube.get_face(face):
            for sticker in row:
                facelets += color_to_face[sticker.colour]
    return facelets


def is_solved(cube):
    for face in ['U', 'R', 'F', 'D', 'L', 'B']:
        stickers = [sticker.colour for row in cube.get_face(face) for sticker in row]
        if len(set(stickers)) != 1:
            return False
    return True

scramble = sys.argv[1] if len(sys.argv) > 1 else ""
cube = pc.Cube()
if scramble.strip():
    cube(scramble)

if is_solved(cube):
    print("Cube is already solved!")
else:
    facelets = cube_to_facelets(cube)  # corrected function
    solution = kociemba.solve(facelets)
    print(solution)
