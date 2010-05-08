IF(UNIX)
   FILE(TO_CMAKE_PATH "/usr/local/bin" PATH_DIR)
ENDIF(UNIX)

IF(WIN32)   
    SET(PATH_DIR "[HKEY_LOCAL_MACHINE\\SOFTWARE\\GnuWin32;InstallPath]/bin")       
ENDIF(WIN32)

MESSAGE(STATUS "Looking for lex...")
# FIND_PROGRAM twice using NO_DEFAULT_PATH on first shot
FIND_PROGRAM(LEX_PROGRAM 
  NAMES flex lex
  PATHS ${PATH_DIR}
  NO_DEFAULT_PATH 
  DOC "A lex-compatible lexer generator")
FIND_PROGRAM(LEX_PROGRAM 
  NAMES flex lex
  PATHS ${PATH_DIR}
  DOC "A lex-compatible lexer generator")

IF (LEX_PROGRAM) 
  MESSAGE(STATUS "Looking for lex... - found lex is ${LEX_PROGRAM}")
  SET(LEX_FOUND "YES")
ELSE (LEX_PROGRAM) 
  SET(LEX_FOUND "NO")
  MESSAGE(STATUS "Looking for lex... - NOT found")
ENDIF (LEX_PROGRAM) 

MESSAGE(STATUS "Looking for yacc...")
# FIND_PROGRAM twice using NO_DEFAULT_PATH on first shot
FIND_PROGRAM(YACC_PROGRAM 
  NAMES bison yacc yacc.bison 
  PATHS ${PATH_DIR}
  NO_DEFAULT_PATH
  DOC "A yacc-compatible parser generator")
FIND_PROGRAM(YACC_PROGRAM 
  NAMES bison yacc yacc.bison 
  PATHS ${PATH_DIR}
  DOC "A yacc-compatible parser generator")
  
IF (YACC_PROGRAM) 
  SET(YACC_FOUND "YES")
  MESSAGE(STATUS "Looking for yacc... - found yacc is ${YACC_PROGRAM}")
  IF ("${YACC_PROGRAM}" MATCHES ".*bison.*")
    SET(YACC_COMPAT_ARG -y)
  ELSE ("${YACC_PROGRAM}" MATCHES ".*bison.*")
    SET(YACC_COMPAT_ARG  )
  ENDIF ("${YACC_PROGRAM}" MATCHES ".*bison.*")
ELSE (YACC_PROGRAM)
  SET(YACC_FOUND "NO")
  MESSAGE(STATUS "Looking for yacc... - NOT found")
ENDIF (YACC_PROGRAM)
    
IF(WIN32)
   IF(LEX_FOUND)
	 get_filename_component(PATH_LEX ${LEX_PROGRAM} PATH)
	 get_filename_component(INCLUDE_LEX ${PATH_LEX} PATH)
	 INCLUDE_DIRECTORIES(${INCLUDE_LEX}/include)
   ENDIF(LEX_FOUND)
ENDIF(WIN32)