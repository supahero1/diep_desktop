#   Copyright 2025 Franciszek Balcerak
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

import os
import subprocess



env = Environment(tools = ["mingw"] if os.name == "nt" else ["default"])

flags = Split("-std=gnu23 -Wall -Iinclude/ -D_GNU_SOURCE")

freetype2_flags = subprocess.check_output(["pkg-config", "--cflags", "freetype2"], text=True)
flags.extend(Split(freetype2_flags))

release = int(ARGUMENTS["RELEASE"] if "RELEASE" in ARGUMENTS else os.environ.get("RELEASE", "0"))
if release <= 0:
	flags.extend(Split("-O0 -g3 -D_FORTIFY_SOURCE=3"))
	if os.name != "nt":
		env.Append(LINKFLAGS=Split("-rdynamic"))
else:
	if release >= 1:
		flags.extend(Split("-O3 -DNDEBUG -flto"))
	if release >= 2:
		flags.extend(Split("-march=native"))

env.Append(CPPFLAGS=flags)

libs = Split("m zstd SDL3 harfbuzz utf8proc freetype png")
if os.name == "nt":
	libs.extend(Split("vulkan-1 ws2_32"))
else:
	libs.extend(Split("vulkan"))
env.Append(LIBS=libs)



def help(target, source, env):
	print("""
Specify one (or more) of the following:

client      generates the client, requires dds and all sources
server      generates the server, no prerequisites

Specify RELEASE=1 for a production build.
Specify RELEASE=2 for a native build (faster than production but not portable).
	""")

env.AlwaysBuild(env.Alias("help", [], help))

Default("help")



deps = {}

def prop_headers(dest, headers):
	for header in headers:
		if header not in dest:
			dest.append(header)
			if header in deps:
				prop_headers(dest, deps[header])

def add_file(path):
	if path in deps or "client/tex/" in path or "client/font/" in path:
		return False
	with open(path, "r") as f:
		if f.readline().startswith("/* skip */"):
			return False
		include_lines = [line for line in f if line.startswith("#include <DiepDesktop/")]
		headers = ["include/" + line[10:-2] for line in include_lines]
		deps[path] = headers
		for header in headers:
			add_file(header)
		for header in headers:
			if header in deps:
				prop_headers(headers, deps[header])
		if path.startswith("tests/"):
			sources = []
			for header in headers:
				source = "src/" + header[20:-1] + "c"
				if source in deps:
					sources.append(source)
			headers.extend(sources)
	return True

def add_files(path):
	output = []
	for root, _, files in os.walk(path):
		for file in files:
			if file.endswith(".c"):
				filepath = os.path.join(root, file)
				if add_file(filepath):
					output.append(filepath)
	return output

client_src_files = add_files("src/client")
server_src_files = add_files("src/server")
shared_src_files = add_files("src/shared")

tex_src_files = add_files("tex")

libtest_file = "tests/libtest.c"
add_file(libtest_file)

client_test_files = add_files("tests/client")
server_test_files = add_files("tests/server")
shared_test_files = add_files("tests/shared")



objects = {}

def add_object(file):
	files = deps[file]
	obj = env.Object("bin/" + file[:-1] + "o", file)[0]
	objects[file] = obj
	for file in files:
		if file.endswith(".c"):
			if file not in objects:
				add_object(file)
			env.Depends(obj, objects[file])
		else:
			env.Depends(obj, file)
	return obj

def add_objects(files):
	return [add_object(file) for file in files]

client_src_objects = add_objects(client_src_files)
server_src_objects = add_objects(server_src_files)
shared_src_objects = add_objects(shared_src_files)

tex_src_objects = add_objects(tex_src_files)

libtest_object = add_object(libtest_file)

client_test_objects = add_objects(client_test_files)
server_test_objects = add_objects(server_test_files)
shared_test_objects = add_objects(shared_test_files)

client = env.Program("bin/client", shared_src_objects + client_src_objects)
server = env.Program("bin/server", shared_src_objects + server_src_objects)

env.Alias("client", client)
env.Alias("server", server)

def add_recur(obj_deps, obj):
	if obj in obj_deps:
		return
	obj_deps.append(obj)
	for header in deps[str(obj)[4:-1] + "c"]:
		if header.endswith(".c"):
			add_recur(obj_deps, objects[header])
		else:
			source = "src/" + header[20:-1] + "c"
			if source in objects:
				add_recur(obj_deps, objects[source])

libtest_obj_deps = []
add_recur(libtest_obj_deps, libtest_object)
libtest = env.Library("bin/tests/libtest", libtest_obj_deps)
env.Alias("libtest", libtest)

def add_program(object, use_libtest=False):
	obj_deps = []
	add_recur(obj_deps, object)
	program = env.Program(str(object)[:-2], obj_deps,
		LIBPATH = Split("bin/tests/") if use_libtest else [],
		LIBS = libs + Split("libtest elf") if use_libtest else libs)
	env.Alias(str(object)[4:-2], program)
	return program

def add_test(object):
	program = add_program(object, True)
	output = str(program[0]) + ".out"
	test = env.Command(output, program, "$SOURCE --file > $TARGET 2>&1")
	valgrind_output = output + ".val"
	valgrind_test = env.Command(valgrind_output, program,
		"valgrind --leak-check=full --show-leak-kinds=all --suppressions=val_sup.txt -- $SOURCE --file > $TARGET 2>&1")
	return [test, valgrind_test]

def add_tests(objects):
	return [add_test(object) for object in objects]

client_tests = add_tests(client_test_objects)
server_tests = add_tests(server_test_objects)
shared_tests = add_tests(shared_test_objects)

if release <= 0:
	tests = env.Command("bin/tests/status",
		shared_tests + client_tests + server_tests,
		"echo \"pass\" > $TARGET")
	env.Alias("test", tests)
	env.Depends([client, server], tests)
else:
	def test_message(target, source, env):
		print("\033[1m\033[35m[TEST]\033[39m > Tests are not available " +
			"in release mode (because assertions are disabled). <\033[0m")

	env.AlwaysBuild(env.Alias("test", [], test_message))

for object in tex_src_objects:
	env.Alias(str(object)[4:-2], add_program(object))
