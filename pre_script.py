Import("env")

build_flags = env.ParseFlags(env['BUILD_FLAGS'])
defines = {k: v for (k, v) in build_flags.get("CPPDEFINES")}
print("CPP defines: ", defines)

# GIT_* defines are quoted strings, use eval to remove the quotes
env.Replace(PROGNAME="mie_hvac-%s" % eval(defines.get("GIT_DESCRIBE")))
