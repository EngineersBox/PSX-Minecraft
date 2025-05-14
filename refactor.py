import os

def rewriteImport(file, rel_dir):
    with open(f"src/{file}", 'w') as f:
        data = f.read()
    # print(f"File: {file} New: {rel_dir}/core/std/stdlib.h")
    data = data.replace("<stdlib.h>", f"\"{rel_dir}/core/std/stdlib.h\"")

def main():
    for dir_, _, files in os.walk("src"):
        for file_name in files:
            rel_dir = os.path.relpath(dir_, "src")
            rel_file = os.path.join(rel_dir, file_name)
            rewriteImport(rel_file, os.path.relpath("src", dir_))

if __name__ == "__main__":
    main()
