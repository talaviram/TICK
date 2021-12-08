import patch_installer_ver as p
import os, pathlib, sys, shutil, subprocess


def main():
    root_path = pathlib.Path(str(sys.argv[1])).absolute()
    version_string = p.get_version_from_jucer(
        pathlib.Path(root_path).joinpath("TICK.jucer")
    )
    print("Making debian package")
    deb_make_path = root_path.joinpath("deb_temp")
    if deb_make_path.exists():
        shutil.rmtree(deb_make_path)
        print("Deleting old temp dir...")
    deb_make_path.mkdir()
    deb_make_path.joinpath("DEBIAN").mkdir()
    arch = "amd64"
    print("Creating debain package control file...")
    with open(deb_make_path.joinpath("DEBIAN/control"), "w") as f:
        f.write("Package: tick-metronome\n")
        f.write(f"Version: {version_string}\n")
        f.write("Maintainer: Tal Aviram <me@talaviram.com>\n")
        f.write("Description: Professional Cross Platform Metronome\n")
        f.write("Homepage: https://tick.talaviram.com\n")
        f.write(f"Architecture: {arch}\n")
        f.write("License: MIT\n")
        f.write("Section: Multimedia\n")
        f.write(
            "Depends: libasound2-dev|libjack-jackd2-dev|ladspa-sdk|libcurl4-openssl-dev|libfreetype6-dev|libx11-dev|libxcomposite-dev|libxcursor-dev|libxext-dev|libxinerama-dev|libxrandr-dev|libxrender-dev|libwebkit2gtk-4.0-dev|libglu1-mesa-dev|mesa-common-dev\n"
        )
        f.write("\n")
        f.close()

    desktopFilePath = deb_make_path.joinpath("usr/share/applications")
    desktopFilePath.mkdir(parents=True)
    print("Create Desktop File")
    with open(desktopFilePath.joinpath("TICK.desktop"), "w") as f:
        f.write("[Desktop Entry]\n")
        f.write(f"Version={version_string}\n")
        f.write("Name=TICK\n")
        f.write("GenericName=TICK Metronome\n")
        f.write("Comment=Professional Open-Source Metronome\n")
        f.write("Icon=/usr/share/applications/TICK_icon.svg\n")
        f.write("Exec=/usr/local/bin/TICK\n")
        f.write("Type=Application\n")
        f.write("Terminal=false\n")
        f.write("Categories=AudioVideo;Music;Audio;Sequencer\n")
        f.write("Keywords=metronome;tick;audio\n")
        f.write("\n")
        f.close()
    shutil.copyfile(root_path.joinpath("Media/Logo/tick_icon.svg"), desktopFilePath.joinpath("TICK_icon.svg"))
    print("Copy files to be packed by deb.")
    linux_build_path = root_path.joinpath("Builds/LinuxMakefile/build")
    standalone_dst = deb_make_path.joinpath("usr/local/bin")
    print("Copy binary (standalone)...")
    os.makedirs(standalone_dst)
    shutil.copyfile(linux_build_path.joinpath("TICK"), standalone_dst.joinpath("TICK"))
    os.chmod(standalone_dst.joinpath("TICK"), 0x777)
    print("Copy VST3...")
    shutil.copytree(
        linux_build_path.joinpath("TICK.vst3"),
        deb_make_path.joinpath("usr/local/lib/vst3"),
    )
    factory_path = deb_make_path.joinpath("opt/TICK")
    factory_path.mkdir(parents=True)
    factory_src = root_path.joinpath("Installer/Factory")
    print("Copy factory presets to be packed..")
    shutil.copytree(factory_src, factory_path.joinpath("Factory"))
    print("Copy license")
    license_path = deb_make_path.joinpath("usr/share/doc/tick-metronome")
    license_path.mkdir(parents=True)
    shutil.copyfile(root_path.joinpath("LICENSE.md"), license_path.joinpath("copyright"))

    print("Package...")
    os.makedirs(root_path.joinpath("Installer/build"), exist_ok=True)
    output_deb = root_path.joinpath(f"Installer/build/TICK_{version_string}_{arch}.deb")
    subprocess.run(["dpkg", "-b", deb_make_path, output_deb])
    shutil.rmtree(deb_make_path)


if __name__ == "__main__":
    main()
