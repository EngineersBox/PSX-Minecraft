import math

def main() -> None:
    positions: list[str] = []
    for a in range(0, 90):
        r = math.radians(a)
        x = int(math.cos(r) * 4096.0)
        y = int(math.sin(r) * 4096.0)
        positions.append(f"    [{a}]=(DVECTOR){{ .vx={x}, .vy={y} }}")
    print(",\n".join(positions))

if __name__ == "__main__":
    main()
