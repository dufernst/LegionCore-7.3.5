include(ExternalProject)

ExternalProject_Add(http
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib
    CONFIGURE_COMMAND ""
    UPDATE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

ExternalProject_Add(cereal
    GIT_REPOSITORY https://github.com/USCiLab/cereal
    CONFIGURE_COMMAND ""
    UPDATE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

add_library(captcha STATIC dep/libcaptcha.c)

SET (SRCS
    src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/server/game/Accounts/AccountMgr.cpp
    dep/tweetnacl/tweetnacl.c
    dep/smtp-client/src/smtp.c
    dep/smtp-client/src/SMTPMail.cpp
)

# we use the filesystem API of C++17
set(CMAKE_CXX_STANDARD 17)
add_executable(webapi ${SRCS})
add_dependencies(webapi http captcha cereal)

if( UNIX AND NOT NOJEM AND NOT APPLE AND NOT WIN32 )
    set_target_properties(webapi PROPERTIES LINK_FLAGS "-pthread")
endif()

target_compile_definitions(webapi PUBLIC _WEB_API)

ExternalProject_Get_Property(http SOURCE_DIR)
set(HTTP_SOURCE_DIR ${SOURCE_DIR})

ExternalProject_Get_Property(cereal SOURCE_DIR)
set(CEREAL_SOURCE_DIR ${SOURCE_DIR})

target_include_directories(webapi PUBLIC
    ${HTTP_SOURCE_DIR}
    ${CEREAL_SOURCE_DIR}/include
    dep/tweetnacl
    dep/smtp-client/src
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/src/common/Logging
    ${CMAKE_SOURCE_DIR}/src/common/Utilities
    ${CMAKE_SOURCE_DIR}/src/common/Threading
    ${CMAKE_SOURCE_DIR}/src/common/Debugging
    ${CMAKE_SOURCE_DIR}/src/common/Database
    ${CMAKE_SOURCE_DIR}/src/common/Cryptography
    ${CMAKE_SOURCE_DIR}/src/server/game/Accounts

    ${ACE_INCLUDE_DIR}
    ${MYSQL_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
)
target_link_libraries(webapi PUBLIC
    shared
    captcha
    ${ACE_LIBRARY}
    ${MYSQL_LIBRARY}
    ${OPENSSL_LIBRARIES}
)

set_target_properties(webapi PROPERTIES FOLDER "tools")

if( WIN32 )
  if ( MSVC )
    add_custom_command(TARGET webapi
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/webapi.conf.dist ${CMAKE_BINARY_DIR}/bin/$(ConfigurationName)/
    )
    add_custom_command(TARGET webapi
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/confirmation.mail.dist ${CMAKE_BINARY_DIR}/bin/$(ConfigurationName)/
    )
    add_custom_command(TARGET webapi
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/registration.mail.dist ${CMAKE_BINARY_DIR}/bin/$(ConfigurationName)/
    )
    add_custom_command(TARGET webapi
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/reset.mail.dist ${CMAKE_BINARY_DIR}/bin/$(ConfigurationName)/
    )
    add_custom_command(TARGET webapi
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/validation.mail.dist ${CMAKE_BINARY_DIR}/bin/$(ConfigurationName)/
    )
  elseif ( MINGW )
    add_custom_command(TARGET webapi
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/webapi.conf.dist ${CMAKE_BINARY_DIR}/bin/
    )
    add_custom_command(TARGET webapi
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/confirmation.mail.dist ${CMAKE_BINARY_DIR}/bin/
    )
    add_custom_command(TARGET webapi
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/registration.mail.dist ${CMAKE_BINARY_DIR}/bin/
    )
    add_custom_command(TARGET webapi
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/reset.mail.dist ${CMAKE_BINARY_DIR}/bin/
    )
    add_custom_command(TARGET webapi
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/validation.mail.dist ${CMAKE_BINARY_DIR}/bin/
    )
  endif()
endif()


if( UNIX )
  install(TARGETS webapi DESTINATION bin)
  install(FILES  webapi.conf.dist DESTINATION ${CONF_DIR})
  install(FILES confirmation.mail.dist DESTINATION ${CONF_DIR})
  install(FILES registration.mail.dist DESTINATION ${CONF_DIR})
  install(FILES reset.mail.dist DESTINATION ${CONF_DIR})
  install(FILES validation.mail.dist DESTINATION ${CONF_DIR})
elseif( WIN32 )
  install(TARGETS webapi DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install(FILES webapi.conf.dist DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install(FILES confirmation.mail.dist DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install(FILES registration.mail.dist DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install(FILES reset.mail.dist DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install(FILES validation.mail.dist DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif()
