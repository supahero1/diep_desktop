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

env = Environment()

flags = Split("-std=c2y -Wall -Iinclude -D_GNU_SOURCE")

usr = "mingw64" if os.name == "nt" else "usr"
flags.append(f"-I/{usr}/include/freetype2")

release = int(ARGUMENTS["RELEASE"] if "RELEASE" in ARGUMENTS else os.environ.get("RELEASE", "0"))
if release <= 0:
	flags.extend(Split("-O0 -g3 -ggdb -D_FORTIFY_SOURCE=3"))
	if os.name != "nt":
		flags.extend(Split("-rdynamic"))
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


def collect_sources(root, dir):
	sources = []
	for root, _, files in os.walk(os.path.join(root, dir)):
		for file in files:
			if file.endswith(".c"):
				with open(os.path.join(root, file), "r") as f:
					line = f.readline()
					if not line.startswith("/* skip */"):
						sources.append(os.path.join(root, file))
	return sources

shared_sources = collect_sources("src", "shared")
client_sources = collect_sources("src", "client")
server_sources = collect_sources("src", "server")
tex_sources = collect_sources("tex", "")

sources = {}

def scan_file(file):
	if file not in sources:
		path = "include/DiepDesktop/" + file[4:] if file.endswith(".h") else file
		with open(path, "r") as f:
			include_lines = [line for line in f if line.startswith("#include <DiepDesktop/")]
			headers = ["src/" + line[22:-2] for line in include_lines]
			sources[file] = headers
			for header in headers:
				scan_file(header)

def source_to_object(source):
	scan_file(source)
	return env.Object("bin/" + source[:-1] + "o", source)

def sources_to_objects(sources):
	return [source_to_object(source) for source in sources]

shared_objects = sources_to_objects(shared_sources)
client_objects = sources_to_objects(client_sources)
server_objects = sources_to_objects(server_sources)
tex_objects = sources_to_objects(tex_sources)
objects = shared_objects + client_objects + server_objects + tex_objects
objects_str = { str(obj[0]): obj[0] for obj in objects }

new_sources = {}
for file in sources:
	name =	file[:-1] + "h"
	if name not in new_sources:
		new_sources[name] = sources[file]
	else:
		new_sources[name].extend(sources[file])
sources = new_sources

def expand_sources(name, file):
	if file not in sources[name]:
		sources[name].append(file)
		for header in sources[file]:
			expand_sources(name, header)

for file in sources:
	for dependency in sources[file]:
		for header in sources[dependency]:
			expand_sources(file, header)

deps = {}

def build_deps(name, file):
	added = []
	if name not in deps:
		deps[name] = sources[file]
		added = sources[file]
	else:
		for source in sources[file]:
			if source not in deps[name]:
				deps[name].append(source)
				added.append(source)
	for file in added:
		build_deps(name, file)

for file in sources:
	name = "bin/" + file[:-1] + "o"
	build_deps(name, file)

for object_name in deps:
	if object_name in objects_str:
		env.Depends(objects_str[object_name], ["include/DiepDesktop/" + header[4:] for header in deps[object_name]])

checked = {}
new_deps = {}
for object_name in deps:
	new_deps[object_name] = []
	for header in deps[object_name]:
		checked[header] = True
		object = objects_str.get("bin/" + header[:-1] + "o", None)
		if object is not None:
			new_deps[object_name].append(object)
deps = new_deps

client = env.Program("bin/client", shared_objects + client_objects)
server = env.Program("bin/server", shared_objects + server_objects)

env.Alias("client", client)
env.Alias("server", server)

shared_tests = collect_sources("tests", "shared")
client_tests = collect_sources("tests", "client")
server_tests = collect_sources("tests", "server")

def create_program(source, object):
	program_object = source_to_object(source)
	header_deps = ["include/DiepDesktop/" + header[4:] for header in sources[str(object)[4:-1] + "h"]]
	env.Depends(program_object, deps[str(object)] + header_deps)
	return env.Program("bin/" + source[:-2], [program_object] + deps[str(object)])

def create_test(test_source, source_object):
	test_program = create_program(test_source, source_object)
	output = str(test_program[0]) + ".out"
	test = env.Command(output, test_program, "./$SOURCE > " + output)
	return test

def for_source_pairs(sources, objects, cb):
	for source in sources:
		source_file = source.replace("tests/", "bin/src/").replace("tex/", "bin/tex/")
		matching_source_object = None
		for list in objects:
			for obj in list:
				if str(obj) == source_file[:-1] + "o":
					matching_source_object = obj
					break
		if matching_source_object is None:
			raise Exception("No matching source object for " + source)
		cb(source, matching_source_object)

def create_tests(tests, objects):
	outputs = []
	for_source_pairs(tests, objects, lambda source, object: outputs.append(create_test(source, object)))
	return outputs

shared_test_outputs = create_tests(shared_tests, shared_objects)
client_test_outputs = create_tests(client_tests, client_objects)
server_test_outputs = create_tests(server_tests, server_objects)

tests = env.Command("bin/tests/status",
					shared_test_outputs + client_test_outputs + server_test_outputs,
					"echo \"pass\" > $TARGET")
env.Depends([client, server], tests)

def create_tex_program(source, object):
	env.Alias(source[4:-2], create_program(source, object))

for_source_pairs(tex_sources, tex_objects, create_tex_program)

