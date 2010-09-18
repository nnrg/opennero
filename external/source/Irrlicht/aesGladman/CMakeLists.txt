SET(AES_SOURCES
aescrypt.cpp
aeskey.cpp
aestab.cpp
fileenc.cpp
hmac.cpp
prng.cpp
pwd2key.cpp
sha1.cpp
sha2.cpp
aes.h
aesopt.h
fileenc.h
hmac.h
prng.h
pwd2key.h
sha1.h
sha2.h
)

SET_SOURCE_FILES_PROPERTIES(${AES_SOURCES} PROPERTIES COMPILE_FLAGS -fPIC)
ADD_LIBRARY(aes ${AES_SOURCES})
