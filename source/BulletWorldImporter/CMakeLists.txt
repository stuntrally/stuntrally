add_library(BulletWorldImporter STATIC
	btBulletWorldImporter.cpp
	btBulletWorldImporter.h
	btWorldImporter.cpp
	btWorldImporter.h)

target_include_directories(BulletWorldImporter PRIVATE ${BULLET_INCLUDE_DIRS})
target_link_libraries(BulletWorldImporter PRIVATE BulletFileLoader ${BULLET_LIBRARIES})
target_include_directories(BulletWorldImporter PUBLIC ./)
