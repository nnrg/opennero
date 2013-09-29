SET(Z_SOURCES
adler32.c
compress.c
crc32.c
deflate.c
infback.c
inffast.c
inflate.c
inftrees.c
trees.c
uncompr.c
zutil.c
)

SET_SOURCE_FILES_PROPERTIES(${Z_SOURCES} PROPERTIES COMPILE_FLAGS -fPIC)
ADD_LIBRARY(z ${Z_SOURCES})

