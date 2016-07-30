project( SignCutterDemo )

add_executable( ${PROJECT_NAME} main.cpp )
target_link_libraries( ${PROJECT_NAME} ${QT_LIBRARIES} ${OpenCV_LIBS} Utils SignCutter SignLogger SignTransformer )
set_target_properties( ${PROJECT_NAME} PROPERTIES FOLDER Demo )

install( TARGETS ${PROJECT_NAME} DESTINATION bin )
