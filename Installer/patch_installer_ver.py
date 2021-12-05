import xml.etree.ElementTree as ET
import plistlib
import pathlib
import sys


def get_lines_for_string(file_name, string_to_search):
    lines_with_string = []
    """ Check if any line in the file contains given string """
    with open(file_name, "r") as read_obj:
        # Read all lines in the file one by one
        count = 0
        for line in read_obj:
            # For each line, check if line contains the string
            if string_to_search in line:
                lines_with_string.append(count)
            count += 1
    return lines_with_string


def patch_file(file_name, lines, patch_string):
    with open(file_name, "r") as file:
        file_to_patch = file.readlines()
        for line in lines:
            file_to_patch[line] = patch_string + "\n"
        with open(file_name, "w") as file:
            file.writelines(file_to_patch)


def patch_packages(file_name, version_string):
    print("Patch Packages Installer version...")
    modified_plist = None
    with open(file_name, "rb") as fp:
        modified_plist = plistlib.load(fp)
        packages = modified_plist["PACKAGES"]
        for package in packages:
            (package["PACKAGE_SETTINGS"])["VERSION"] = f'"{version_string}"'
    with open(file_name, "wb") as fp:
        plistlib.dump(modified_plist, fp)


def get_version_from_jucer(file_name):
    tree = ET.parse(file_name)
    root = tree.getroot()
    return root.attrib["version"]


def main():
    root_path = str(sys.argv[1])
    version_string = get_version_from_jucer(
        pathlib.Path(root_path).joinpath("TICK.jucer")
    )
    innosetup_line_to_patch = get_lines_for_string(
        pathlib.Path(root_path).joinpath("Installer/tick_installer.iss"),
        "#define MyAppVersion",
    )
    patch_packages(
        pathlib.Path(root_path).joinpath("Installer/TICK.pkgproj"), version_string
    )
    print("patch inno script (Windows)")
    patch_file(
        pathlib.Path(root_path).joinpath("Installer/tick_installer.iss"),
        innosetup_line_to_patch,
        f'#define MyAppVersion "{version_string}"',
    )


if __name__ == "__main__":
    main()
