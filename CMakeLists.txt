
cmake_minimum_required(VERSION 3.28)

project(SmtpClient VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)

# Find OpenSSL package
find_package(OpenSSL REQUIRED)

# Find Boost package with all requirements
find_package(Boost 1.70 REQUIRED COMPONENTS system coroutine)

# Add executable target
add_executable(client)

# Specify source files
target_sources(client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SmtpClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

# Include directories for the client target
target_include_directories(client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
)

# Ad subdirectory for the ErrorHandlers library
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/Handlers)

# Add subdirectory for the Base64 library
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/Base64)

# Add subdirectory for the SmartSocket library
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/SmartSocket)

# Add subdirectory for the SMTPResponse library
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/SMTPResponse)

# Add subdirectory for the MailMessage library
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/MailMessage)

# Add subdirectory for the MessageSender library
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/MessageSender)

# Add subdirectory for the MessageSender library
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/DB)

# Add subdirectory for the tests
add_subdirectory(tests)

# Link libraries to the client target
target_link_libraries(client PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
    SmartSocket
    Base64
    ${Boost_LIBRARIES}
    SMTPResponse
    MailMessage
    MessageSender
)


