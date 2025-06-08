cmake_minimum_required(VERSION 3.20)
project(vulkanLearning-1)

set(CMAKE_CXX_STANDARD 17)

aux_source_directory(. DIRSRCS)

set(VULKAN_SDK_PATH ../thirdParty/vulkan)

include_directories(
SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../thirdParty/GLM/
SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../thirdParty/GLFW/include
SYSTEM ${VULKAN_SDK_PATH}/include
)

link_directories(
	SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../thirdParty/GLFW/lib-vc2022
	SYSTEM ${VULKAN_SDK_PATH}/Lib
)

add_subdirectory(vulkanWrapper)
add_subdirectory(texture)

add_executable(vulkanFrameWork ${DIRSRCS} )

target_link_libraries(vulkanFrameWork vulkan-1.lib textureLib glfw3.lib vulkanLib)