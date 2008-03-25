INCLUDE_DIRECTORIES(
	${CMAKE_PCRE_INCLUDES}
	${CMAKE_LUA_INCLUDES}
	)

ADD_LIBRARY(rex_pcre
	algo.h
	common.h
	common.c
	lpcre.c
	lpcre_f.c
)



