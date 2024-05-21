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

def printPlane(name, normal, pos):
    offset = np.dot(normal, pos)
    print(name)
    print(f"(Plane) {{ .normal = vec3_i32({normal[0] * ONE}, {normal[1] * ONE}, {normal[2] * ONE}), .offset = {offset * ONE} }}")

def calculate_frustum(fov_y: float, aspect: float, z_near: float, z_far: float):
    half_v_side = z_far * math.tan(math.radians(fov_y * 0.5))
    half_h_side = half_v_side * aspect
    front_mul_far = z_far * BACK

    printPlane(
        "near",
        FRONT + z_near,
        FRONT
    )
    printPlane(
        "far",
        front_mul_far,
        BACK
    )
    printPlane(
        "right",
        ZERO,
        np.cross(front_mul_far - RIGHT * half_h_side, UP)
    )
    printPlane(
        "left",
        ZERO,
        np.cross(UP, front_mul_far + RIGHT * half_h_side)
    )
    printPlane(
        "top",
        ZERO,
        np.cross(RIGHT, front_mul_far - UP, half_v_side)
    )
    printPlane(
        "bottom",
        ZERO,
        np.cross(front_mul_far + UP * half_v_side, RIGHT)
    )

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
