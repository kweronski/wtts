############################################## XML DATA READER
add_executable(testXMLParser testXMLParser.cpp)
target_link_libraries(testXMLParser PRIVATE dataparser)

# These tests read an xml file and check if the data is correct,
# using the text file as a reference.
add_test(NAME xmlParserTest001
	COMMAND testXMLParser ${CMAKE_SOURCE_DIR}/data/xml/dataStorage.xml
	${CMAKE_SOURCE_DIR}/data/txt/dataStorage.txt)

add_test(NAME xmlParserTest002
	COMMAND testXMLParser ${CMAKE_SOURCE_DIR}/data/xml/dataStorage1.xml
	${CMAKE_SOURCE_DIR}/data/txt/dataStorage.txt)

set_tests_properties(xmlParserTest002 PROPERTIES WILL_FAIL true)

############################################## XML DATA WRITER
add_executable(testXMLWriter xmlWriter.cpp)
target_link_libraries(testXMLWriter PRIVATE dataparser)

# This test writes an xml file using the text file as a source.
# Next, the test uses the xml parser to read the written xml file
# and check if the data is as expected.
add_test(NAME xmlWriterTest001
	COMMAND testXMLWriter ${CMAKE_SOURCE_DIR}/data/txt/dataStorage.txt)
