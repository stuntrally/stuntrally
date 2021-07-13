add_library(BulletFileLoader STATIC
	bChunk.cpp
	bChunk.h
	bCommon.h
	bDefines.h
	bDNA.cpp
	bDNA.h
	bFile.cpp
	bFile.h
	btBulletFile.cpp
	btBulletFile.h)

target_include_directories(BulletFileLoader PRIVATE ${BULLET_INCLUDE_DIRS})
target_link_libraries(BulletFileLoader PRIVATE ${BULLET_LIBRARIES})
target_include_directories(BulletFileLoader PUBLIC ./)
