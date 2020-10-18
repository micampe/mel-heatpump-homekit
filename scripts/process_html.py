import os.path

Import("env")

src_file = os.path.join(env["PROJECT_DIR"], "web/index.html")
dest_file = os.path.join(env["PROJECT_DIR"], "src/html.cpp")

template = """#include <sys/pgmspace.h>

// generated file, edit web/index.html
const char* index_html PROGMEM = R"====(
__HTML__
)====";
"""

with open(src_file) as src:
    with open(dest_file, 'w') as dest:
        html = src.read()
        result = template.replace('__HTML__', html);
        dest.write(result)

