project( SignTransformer )

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

file( GLOB_RECURSE HDRS_FILES *.h *.hpp )
file( GLOB_RECURSE SRCS_FILES *.cpp )
file( GLOB_RECURSE UI_FILES *.ui )
file( GLOB CD_FILES *.cd )

qt4_wrap_cpp( MOC_SRCS ${HDRS_FILES} )
qt4_add_resources( RSC_SRCS ${RSCS} )
qt4_wrap_ui( UI_HDRS ${UI_FILES} )

set( FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${SRCS_FILES} ${UI_FILES} ${HDRS_FILES} PARENT_SCOPE )

source_group( "UI Files" FILES ${UI_FILES} )
source_group( "Generated Files" FILES ${MOC_SRCS} ${UI_HDRS} ${RSC_SRCS} )
source_group( "Class Diagrams" FILES ${CD_FILES} )
source_group( "Resources" FILES ${RSCS} )

add_library( ${PROJECT_NAME} STATIC ${SRCS_FILES} ${UI_FILES} ${HDRS_FILES} ${MOC_SRCS} ${UI_HDRS} ${RSC_SRCS} ${CD_FILES} )
set_source_files_properties( ${CD_FILES} PROPERTIES HEADER_FILE_ONLY TRUE) #DO NOT COMPILE CS FILES
target_link_libraries( ${PROJECT_NAME} ${QT_LIBRARIES} ${OPENGL_LIBRARY} ${GLUT_LIBRARIES} ${OpenCV_LIBS} Utils SignCutter )
set_target_properties( ${PROJECT_NAME} PROPERTIES LINK_FLAGS ${LARGEADDRESSAWARE} )
set_target_properties( ${PROJECT_NAME} PROPERTIES FOLDER Src )  	
set_target_properties( ${PROJECT_NAME} PROPERTIES VS_KEYWORD Qt4VSv1.0 )

install( TARGETS ${PROJECT_NAME} DESTINATION bin )