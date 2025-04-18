# kio_help only buildable if doc tools around
if (KF6DocTools_FOUND)

find_package(KF6Archive ${KF_DEP_VERSION} REQUIRED)
find_package(LibXslt)
set_package_properties(LibXslt PROPERTIES
                       URL "http://xmlsoft.org/XSLT"
                       TYPE REQUIRED
                       PURPOSE "Required by the KDE help system to process DocBook XML"
                      )

find_package(LibXml2)
set_package_properties(LibXml2 PROPERTIES
                       URL "http://xmlsoft.org"
                       TYPE REQUIRED
                       PURPOSE "Required by the KDE help system to process DocBook XML"
                      )

include(ConfigureChecks.cmake)
configure_file(config-help.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-help.h )

#macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/checkXML )

find_package(Qt6Core ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)

########### next target ###############
kcoreaddons_add_plugin(kio_help INSTALL_NAMESPACE "kf6/kio")

target_sources(kio_help PRIVATE
    kio_help.cpp
    main.cpp
    xslt_help.cpp
)

target_link_libraries(kio_help
   Qt6::Gui # QTextDocument
   Qt6::Network
   KF6::I18n
   KF6::Archive
   KF6::KIOCore
   KF6::DocTools
   LibXml2::LibXml2
   LibXslt::LibExslt
)

install(TARGETS kio_help  DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kio)


########### next target ###############
kcoreaddons_add_plugin(kio_ghelp INSTALL_NAMESPACE "kf6/kio")

target_sources(kio_ghelp PRIVATE
    kio_help.cpp
    main_ghelp.cpp
    xslt_help.cpp
)

target_link_libraries(kio_ghelp
   Qt6::Gui # QTextDocument
   KF6::I18n
   KF6::Archive
   KF6::KIOCore
   KF6::DocTools
   Qt6::Network
   LibXml2::LibXml2
   LibXslt::LibExslt
)
if(WIN32)
    # libxml/libxslt pulls in windows.h, this breaks QDateTime
    target_compile_definitions(kio_help PRIVATE NOMINMAX)
    target_compile_definitions(kio_ghelp PRIVATE NOMINMAX)
endif()

endif()
