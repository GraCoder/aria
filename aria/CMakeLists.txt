cmake_minimum_required(VERSION 3.8.0)
set(app_name aria)


find_package(Qt5 COMPONENTS Widgets Gui REQUIRED)
set(moc_h
	ariaUi.h
)


qt5_wrap_cpp(mocfiles ${moc_h})
source_group("moc" FILES ${mocfiles})

qt5_add_resources(mocfiles ariaUi.qrc)

set(hdr
	ariaUi.h
	ariaPanel.h
	uriLink.h
	frameless.h
	ariaSys.h
)

set(src
	main.cpp
	ariaUi.cpp
	ariaPanel.cpp
	uriLink.cpp
	frameless.cpp
	ariaSys.cpp
)


add_executable(${app_name} ${hdr} ${src} ${mocfiles})

target_link_directories(${app_name} PRIVATE ${CMAKE_BINARY_DIR}/lib ${CMAKE_SOURCE_DIR}/lib)

set_target_properties(${app_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${app_name}>"
                                               VS_DEBUGGER_COMMAND           "$<TARGET_FILE:${app_name}>"
                                               VS_DEBUGGER_ENVIRONMENT       "PATH=%PATH%;${CMAKE_PREFIX_PATH}/bin")
target_link_libraries(${app_name} Qt5::Widgets Qt5::Gui)
target_link_libraries(${app_name} aria2)
