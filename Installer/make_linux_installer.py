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
        f.write(
            "Depends: libasound2-dev|libjack-jackd2-dev|ladspa-sdk|libcurl4-openssl-dev|libfreetype6-dev|libx11-dev|libxcomposite-dev|libxcursor-dev|libxext-dev|libxinerama-dev|libxrandr-dev|libxrender-dev|libwebkit2gtk-4.0-dev|libglu1-mesa-dev|mesa-common-dev\n"
        )
        f.write("\n")
        f.close()
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
    print("Package...")
    # os.mkdir(root_path.joinpath("Installer/build"))
    output_deb = root_path.joinpath(f"Installer/build/TICK_{version_string}_{arch}.deb")
    subprocess.run(["dpkg", "-b", deb_make_path, output_deb])
    shutil.rmtree(deb_make_path)


if __name__ == "__main__":
    main()
