cmake_minimum_required(VERSION 3.8.0)
set(app_name aria)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt5 COMPONENTS Widgets Gui Network REQUIRED)
set(moc_h
	ariaUi.h
	frameless.h
	ariaListWgt.h
	ariaHttpServer.h
	taskDatabase.h
	singleapplication.h
	singleapplication_p.h
	taskDetailWgt.h
)

qt5_wrap_cpp(mocfiles ${moc_h})
source_group("moc" FILES ${mocfiles})

qt5_add_resources(mocfiles ariaUi.qrc)

file(GLOB httpserver httpserver/*)
source_group("httpserver" FILES ${httpserver})

set(hdr
	ariaUi.h
	ariaPanel.h
	uriLink.h
	frameless.h
	ariaSys.h
	ariaButton.h
	ariaListWgt.h
	taskInfo.h
	ariaHttpServer.h
	taskDatabase.h
	singleapplication.h
	singleapplication_p.h
	ariaSetting.h
	taskDetailWgt.h
)

set(src
	main.cpp
	ariaUi.cpp
	ariaPanel.cpp
	uriLink.cpp
	frameless.cpp
	ariaSys.cpp
	ariaButton.cpp
	ariaListWgt.cpp
	taskInfo.cpp
	ariaHttpServer.cpp
	taskDatabase.cpp
	singleapplication.cpp
	singleapplication_p.cpp
	ariaSetting.cpp
	taskDetailWgt.cpp
)

set(aria_rc "${CMAKE_CURRENT_SOURCE_DIR}/aria.rc")

add_executable(${app_name} WIN32 ${hdr} ${src} ${mocfiles} ${httpserver} ${aria_rc})

target_include_directories(${app_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../libaria)

target_link_directories(${app_name} PRIVATE ${CMAKE_BINARY_DIR}/lib ${CMAKE_SOURCE_DIR}/lib)

set_target_properties(${app_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${app_name}>"
                                               VS_DEBUGGER_COMMAND           "$<TARGET_FILE:${app_name}>"
                                               VS_DEBUGGER_ENVIRONMENT       "PATH=%PATH%;${CMAKE_PREFIX_PATH}/bin")
target_link_libraries(${app_name} Qt5::Widgets Qt5::Gui Qt5::Network)
target_link_libraries(${app_name} aria2 sqlite3)
target_compile_definitions(${app_name} PUBLIC QAPPLICATION_CLASS=QApplication)
target_link_libraries(${app_name} ws2_32 wsock32 advapi32)

