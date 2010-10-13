SET(tinyxml_SOURCES
tinystr.cpp
tinyxml.cpp
tinyxmlerror.cpp
tinyxmlparser.cpp
)

SET(tinyxml_HEADERS
tinystr.h
tinyxml.h
)

ADD_LIBRARY (tinyxml ${tinyxml_SOURCES} ${tinyxml_HEADERS})
