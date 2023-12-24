import xml.etree.ElementTree as ET
import os

def bundle(lzp_project):
    for root, dirs, files in os.walk(u"."):
        print(root, dirs, files)
    pass

def main():
    with open("assets.xml", "w+") as file:
        lzp_project = ET.fromstring(file.read())
        bundle(lzp_project)
        #file.write(ET.tostring(lzp_project))

if __name__ == "__main__":
    main()
