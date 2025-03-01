SUBDIRS = src
ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = BUILD.bazel WORKSPACE.bazel .bazelrc

# Add .wasm files to clean target
CLEANFILES = *.wasm

# Make sure .wasm files are removed with make clean
clean-local:
	rm -f *.wasm
