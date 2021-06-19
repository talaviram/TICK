import subprocess
import datetime
import pathlib
import sys
def get_git_revision_short_hash():
    short_hash = subprocess.check_output(['git', 'rev-parse', '--short', 'HEAD'])
    short_hash = str(short_hash, "utf-8").strip()
    return short_hash

input_file = str(sys.argv[1])
current_time = datetime.datetime.now().strftime('%I:%M:%S %p')

f = open(pathlib.Path(input_file).absolute(), "w")
f.write(f"// generated {current_time}\n")
f.write("#include \"git_version.h\"\n")
f.write(f"const char* GIT_COMMIT =\"{get_git_revision_short_hash()}\";\n")
f.close()
