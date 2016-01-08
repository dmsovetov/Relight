# List of Relight source files
file(GLOB SRCS_CPP "*.cpp")
file(GLOB SRCS_H "*.h")

file(GLOB BAKER_SRCS_CPP "Baker/*.cpp")
file(GLOB BAKER_SRCS_H "Baker/*.h")

file(GLOB RADIOSITY_SRCS_CPP "Radiosity/*.cpp")
file(GLOB RADIOSITY_SRCS_H "Radiosity/*.h")

file(GLOB RT_SRCS_CPP "Rt/*.cpp")
file(GLOB RT_SRCS_H "Rt/*.h")

file(GLOB SCENE_SRCS_CPP "Scene/*.cpp")
file(GLOB SCENE_SRCS_H "Scene/*.h")

# Setup source groups
source_group(Code FILES ${SRCS_CPP} ${SRCS_H})
source_group(Code\\Baker FILES ${BAKER_SRCS_CPP} ${BAKER_SRCS_H})
source_group(Code\\RayTrace FILES ${RT_SRCS_CPP} ${RT_SRCS_H})
source_group(Code\\Radiosity FILES ${RADIOSITY_SRCS_CPP} ${RADIOSITY_SRCS_H})
source_group(Code\\Scene FILES ${SCENE_SRCS_CPP} ${SCENE_SRCS_H})

# Definitions
add_definitions(-DRELIGHT_BUILD_LIBRARY)

# Include paths
include_directories(${EMBREE_INCLUDE_PATH})

# Relight library target
add_library(Relight STATIC
	${SRCS_CPP}
	${SRCS_H}
	${BAKER_SRCS_CPP}
	${BAKER_SRCS_H}
	${RT_SRCS_CPP}
	${RT_SRCS_H}
	${RADIOSITY_SRCS_CPP}
	${RADIOSITY_SRCS_H}
	${SCENE_SRCS_CPP}
	${SCENE_SRCS_H}
	)
	
# Link Embree
target_link_libraries(Relight ${EMBREE_LIBRARY})