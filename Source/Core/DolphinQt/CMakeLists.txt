set(QT_USE_QTNETWORK FALSE)
set(QT_USE_QTOPENGL FALSE)
set(QT_USE_QTSQL FALSE)
set(QT_USE_QTXML FALSE)
set(QT_USE_QTSVG FALSE)
set(QT_USE_QTTEST FALSE)
set(QT_USE_QTDBUS FALSE)
set(QT_USE_QTSCRIPT FALSE)
set(QT_USE_QTWEBKIT FALSE)
set(QT_USE_QTXMLPATTERNS FALSE)
set(QT_USE_PHONON FALSE)

set(SRCS		Src/Config/ConfigAudio.cpp
			Src/Config/ConfigGeneral.cpp
			Src/Config/ConfigGfx.cpp
			Src/Config/ConfigMain.cpp
			Src/Config/ConfigPad.cpp
			Src/Config/ConfigWiimote.cpp
			Src/Util/Util.cpp
			Src/Util/Resources.cpp
			Src/GameList.cpp
			Src/ISOFile.cpp
			Src/IssueReporter.cpp
			Src/LogWindow.cpp
			Src/main.cpp
			Src/MainWindow.cpp
			Src/MainWindowEvents.cpp
			Src/RenderWindow.cpp)

if(USE_EGL)
	set(SRCS ${SRCS} Src/GLInterface/Platform.cpp
		Src/GLInterface/EGL.cpp)
	if(USE_WAYLAND)
		set(SRCS ${SRCS} Src/GLInterface/Wayland_Util.cpp)
	endif()
	if(USE_X11)
		set(SRCS ${SRCS} Src/GLInterface/X11_Util.cpp)
	endif()
else()
	if(WIN32)
		set(SRCS ${SRCS} Src/GLInterface/WGL.cpp)
	elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		set(SRCS ${SRCS} Src/GLInterface/AGL.cpp)
	else()
		set(SRCS ${SRCS} Src/GLInterface/GLX.cpp
			Src/GLInterface/X11_Util.cpp)

	endif()
endif()

set(LIBS		core
			${LZO}
			discio
			bdisasm
			inputcommon
			common
			audiocommon
			z
			sfml-network
			${OPENGL_LIBRARIES})

if(USE_X11)
	set(LIBS  ${LIBS} ${X11_LIBRARIES}
			${XINPUT2_LIBRARIES}
			${XRANDR_LIBRARIES})
endif()

if(SDL2_FOUND)
	# Using shared SDL2
	set(LIBS	${LIBS} ${SDL2_LIBRARY})
else(SDL2_FOUND)
	if(SDL_FOUND)
		# Using shared SDL
		set(LIBS	${LIBS} ${SDL_LIBRARY})
	else(SDL_FOUND)
		# Using static SDL from Externals
		set(LIBS	${LIBS} SDL)
	endif()
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(EXEGUI DolphinQt)
else()
	set(EXEGUI dolphin-emu-qt4)
endif()

qt_wrap_cpp(${EXEGUI} MOC_SRCS	Src/Config/ConfigAudio.h
				Src/Config/ConfigGeneral.h
				Src/Config/ConfigGfx.h
				Src/Config/ConfigMain.h
				Src/Config/ConfigPad.h
				Src/Config/ConfigWiimote.h
				Src/Util/Util.h
				Src/IssueReporter.h
				Src/GameList.h
				Src/LogWindow.h
				Src/MainWindow.h
				Src/RenderWindow.h)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	# Link against OS X system frameworks.
	list(APPEND LIBS
		${APPKIT_LIBRARY}
		${AU_LIBRARY}
		${COREAUDIO_LIBRARY}
		${COREFUND_LIBRARY}
		${CORESERV_LIBRARY}
		${IOB_LIBRARY}
		${IOK_LIBRARY}
		)
	# Add resource files to application bundle.
	set(RESOURCES resources/Dolphin.icns)
	list(APPEND SRCS ${RESOURCES})
	set_source_files_properties(${RESOURCES} PROPERTIES
		MACOSX_PACKAGE_LOCATION Resources)
endif()

add_executable(${EXEGUI} ${SRCS} ${MOC_SRCS})
target_link_libraries(${EXEGUI} ${LIBS} ${QT_LIBRARIES})
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	include(BundleUtilities)
	set(BUNDLE_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${EXEGUI}.app)
	# Ask for an application bundle.
	set_target_properties(${EXEGUI} PROPERTIES
		MACOSX_BUNDLE true
		MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
		)
else()
	install(TARGETS ${EXEGUI} RUNTIME DESTINATION ${bindir})
endif()
