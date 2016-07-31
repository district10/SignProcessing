project( SignProcessorDemo )

qt4_add_resources( MODELS ${MODELS_QRC} )
source_group( "All Resource Files" FILES ${MODELS} )
add_executable( ${PROJECT_NAME} main.cpp ${MODELS} )
target_link_libraries( ${PROJECT_NAME} ${QT_LIBRARIES} SignProcessor Utils MLP )
set_target_properties( ${PROJECT_NAME} PROPERTIES FOLDER Demo )

install( TARGETS ${PROJECT_NAME} DESTINATION bin )
