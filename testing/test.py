
def main() -> None:
    values = []
    for y in range(4):
        for z in range(4):
            for x in range(4):
                # idx = x + y + z
                # adjusted_idx = -idx >> 2 if (idx % 2 == 0) else idx >> 2
                if (((x + y + z) & 0b1) != 0):
                    values.append((
                        4 - x,
                        4 - y,
                        4 - z,
                    ))
                else:
                    values.append((x, y, z))
    print(f"Length: {len(values)} All: {4 * 4 * 4}")
    print(values)
    unique = list(set(values))
    print(list(set(values)))
    print(f"Unique: {len(unique)}")

if __name__ == "__main__":
    main()
