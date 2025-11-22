import sys
import magiccube
from magiccube import BasicSolver

if len(sys.argv) < 2:
    print("Usage: script.py '<moves>'")
    sys.exit(1)

moves = sys.argv[1]

cube = magiccube.Cube(3, "YYYYYYYYYRRRRRRRRRGGGGGGGGGOOOOOOOOOBBBBBBBBBWWWWWWWWW")
moves = moves.strip() + " "
cube.rotate(moves)

solver = BasicSolver(cube)
solution = solver.solve()

print(solution)

