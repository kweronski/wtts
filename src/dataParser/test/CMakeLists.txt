add_executable(testXMLParser testXMLParser.cpp)

target_link_libraries(testXMLParser PRIVATE dataparser)

add_test(NAME xmlParserTest001
	COMMAND testXMLParser ${CMAKE_SOURCE_DIR}/data/xml/dataStorage.xml
	${CMAKE_SOURCE_DIR}/data/txt/dataStorage.txt)
