import math, sys
import numpy as np

FRONT = np.array([0,0,-1])
BACK = np.array([0,0,1])
RIGHT = np.array([1,0,0])
LEFT = np.array([-1,0,0])
UP = np.array([0,-1,0])
DOWN = np.array([0,1,0])

ZERO = np.array([0,0,0])

ONE = 4096

def normalize(v):
    return v / np.sqrt(np.sum(v**2))

def printPlane(name, normal, point):
    n0 = int(normal[0] * ONE)
    n1 = int(normal[1] * ONE)
    n2 = int(normal[2] * ONE)
    p0 = int(point[0] * ONE)
    p1 = int(point[1] * ONE)
    p2 = int(point[2] * ONE)
    print(f"        [FRUSTUM_PLANE_{name.upper()}] = (Plane) {{ .normal = vec3_i32({n0}, {n1}, {n2}), .point = vec3_i32({p0}, {p1}, {p2}) }},")

def calculate_frustum(fov_y: float, aspect: float, z_near: float, z_far: float):
    near_centre = FRONT * z_near
    far_centre = FRONT * z_far

    near_height = 2 * math.tan(math.radians(fov_y / 2)) * z_near
    far_height = 2 * math.tan(math.radians(fov_y / 2)) * z_far
    near_width = near_height * aspect
    far_width = far_height * aspect

    far_top_left = far_centre + UP * (far_height * 0.5) + LEFT * (far_width * 0.5)
    far_top_right = far_centre + UP * (far_height * 0.5) + RIGHT * (far_width * 0.5)
    far_bottom_left = far_centre + DOWN * (far_height * 0.5) + LEFT * (far_width * 0.5)
    far_bottom_right = far_centre + DOWN * (far_height * 0.5) + RIGHT * (far_width * 0.5)

    near_top_left = near_centre + UP * (near_height * 0.5) + LEFT * (near_width * 0.5)
    near_top_right = near_centre + UP * (near_height * 0.5) + RIGHT * (near_width * 0.5)
    near_bottom_left = near_centre + DOWN * (near_height * 0.5) + LEFT * (near_width * 0.5)
    near_bottom_right = near_centre + DOWN * (near_height * 0.5) + RIGHT * (near_width * 0.5)

    p0 = None
    p1 = None
    p2 = None

    p0 = near_bottom_left; p1 = far_bottom_left; p2 = far_top_left
    left_plane_normal = normalize(np.cross(p1 - p0, p2 - p1))
    # left_plane_offset = np.dot(left_plane_normal, p0)

    p0 = near_top_left; p1 = far_top_left; p2 = far_top_right
    top_plane_normal = normalize(np.cross(p1 - p0, p2 - p1))
    # top_plane_offset = np.dot(top_plane_normal, p0)

    p0 = near_top_right; p1 = far_top_right; p2 = far_bottom_right
    right_plane_normal = normalize(np.cross(p1 - p0, p2 - p1))
    # right_plane_offset = np.dot(right_plane_normal, p0)

    p0 = near_bottom_right; p1 = far_bottom_right; p2 = far_bottom_left
    bottom_plane_normal = normalize(np.cross(p1 - p0, p2 - p1))
    # bottom_plane_offset = np.dot(bottom_plane_normal, p0)

    print("(Frustum) {")
    print("    .planes = {")
    printPlane("near", FRONT, np.array([0,0,z_near]))
    printPlane("far", BACK, np.array([0,0,z_far]))
    printPlane("left", left_plane_normal, ZERO)
    printPlane("right", right_plane_normal, ZERO)
    printPlane("top", top_plane_normal, ZERO)
    printPlane("bottom", bottom_plane_normal, ZERO)
    print("    }")
    print("}")

def main():
    if (len(sys.argv) != 6):
        print("Usage: frustum_calculator.py <fov y> <aspect width> <aspect height> <z near> <z far>");
        return
    calculate_frustum(
        float(sys.argv[1]),
        float(sys.argv[2]) / float(sys.argv[3]),
        float(sys.argv[4]),
        float(sys.argv[5])
    )

if __name__ == "__main__":
    main()
