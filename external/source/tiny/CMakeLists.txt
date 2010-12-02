FILE(GLOB tinyxml_SOURCES *.cpp)
FILE(GLOB tinyxml_HEADERS *.h)
ADD_LIBRARY (tinyxml ${tinyxml_SOURCES} ${tinyxml_HEADERS})
