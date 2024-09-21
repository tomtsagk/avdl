file(GLOB cengine_src CONFIGURE_DEPENDS
	"engines/cengine/src/*.c"
	"engines/cengine/src/*.cpp"
)

file(GLOB cengine_headers CONFIGURE_DEPENDS
	"engines/cengine/include/*.h"
)

install(FILES ${cengine_src} DESTINATION share/avdl/cengine)
install(FILES ${cengine_headers} DESTINATION include)
