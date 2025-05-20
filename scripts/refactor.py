import os

def rewriteImport(file, rel_dir):
    with open(f"src/{file}", 'r') as f:
        data = f.read()
    data = data.replace("<stdlib.h>", f"\"{rel_dir}/core/std/stdlib.h\"")
    with open(f"src/{file}", 'w') as f:
        f.write(data)

def main():
    dir = os.path.basename(os.getcwd())
    if (dir != "PSX-Minecraft"):
        print(f"[ERROR] Script must be run from 'PSX-Minecraft' directory, not '{dir}'")
        exit(1)
    for dir_, _, files in os.walk("src"):
        for file_name in files:
            if (file_name == "main.c" or file_name == "stdlib.h"):
                continue
            rel_dir = os.path.relpath(dir_, "src")
            rel_file = os.path.join(rel_dir, file_name)
            rewriteImport(rel_file, os.path.relpath("src", dir_))

if __name__ == "__main__":
    main()
