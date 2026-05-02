add_executable(testXMLParser testXMLParser.cpp)

target_link_libraries(testXMLParser PRIVATE dataparser)

add_test(NAME xmlParserTest001
	COMMAND testXMLParser ${CMAKE_SOURCE_DIR}/data/xml/dataStorage.xml
	${CMAKE_SOURCE_DIR}/data/txt/dataStorage.txt)

add_test(NAME xmlParserTest002
	COMMAND testXMLParser ${CMAKE_SOURCE_DIR}/data/xml/dataStorage1.xml
	${CMAKE_SOURCE_DIR}/data/txt/dataStorage.txt)

set_tests_properties(xmlParserTest002 PROPERTIES WILL_FAIL true)
