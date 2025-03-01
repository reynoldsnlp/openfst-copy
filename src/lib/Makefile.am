AM_CPPFLAGS = -I$(srcdir)/../include $(ICU_CPPFLAGS)


libfst_la_SOURCES = compat.cc encode.cc flags.cc fst.cc fst-types.cc \
                    mapped-file.cc properties.cc symbol-table.cc \
                    symbol-table-ops.cc weight.cc util.cc
libfst_la_LDFLAGS = -version-info 26:0:0

if EMSCRIPTEN

libfst_la_SOURCES += EmscriptenBindings.cc
libfst_la_LDFLAGS += $(WASM_LDFLAGS)
# Replace libfst.la with libfst.a
lib_LIBRARIES = libfst.a
noinst_LTLIBRARIES = libfst.la
libfst_a_SOURCES = $(libfst_la_SOURCES)
libfst_a_CPPFLAGS = $(AM_CPPFLAGS)

else

lib_LTLIBRARIES = libfst.la
libfst_la_LIBADD = $(DL_LIBS)

endif
