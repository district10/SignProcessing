project( MLP )

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

add_library( ${PROJECT_NAME} STATIC MLP.h MLP.cpp )
target_link_libraries( ${PROJECT_NAME} ${QT_LIBRARIES} ${OpenCV_LIBS} Utils )
set_target_properties( ${PROJECT_NAME} PROPERTIES LINK_FLAGS ${LARGEADDRESSAWARE} )
set_target_properties( ${PROJECT_NAME} PROPERTIES FOLDER Src )

install( TARGETS ${PROJECT_NAME} DESTINATION bin )
