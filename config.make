PROJECT_EXTERNAL_SOURCE_PATHS = ../../../addons/np-patches
USER_CFLAGS = -I../../../addons/np-patches
USER_LIBS = -lexternal_shared_library
PROJECT_LDFLAGS=-Wl,-rpath=../../../addons/np-patches
