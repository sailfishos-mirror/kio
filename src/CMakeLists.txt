add_subdirectory(core)

# KIOCore-only executables
if (NOT ANDROID)
    add_subdirectory(kioworkers)
endif()

if (HAVE_QTDBUS)
    add_subdirectory(kiod)
    add_subdirectory(kssld)
endif()
add_subdirectory(kioworker)

if (NOT KIOCORE_ONLY)
    add_subdirectory(gui)
    if (NOT ANDROID)
        add_subdirectory(schemehandlers)
    endif()

    if (NOT KIOGUI_ONLY)
        add_subdirectory(widgets)

        if (HAVE_QTDBUS)
            add_subdirectory(kpasswdserver)
            add_subdirectory(kioexec)
        endif()

        if (NOT ANDROID)
            add_subdirectory(filewidgets)
            add_subdirectory(urifilters)
        endif()

        if(BUILD_DESIGNERPLUGIN AND HAVE_QTDBUS)
            add_subdirectory(designer)
        endif()
    endif()
endif()

ecm_qt_install_logging_categories(
    EXPORT KIO
    FILE kio.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

# Temporary experimental: testing CI
add_test(
    NAME installed_library_check
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --parallel 12 --target all_installed_library_check
)
