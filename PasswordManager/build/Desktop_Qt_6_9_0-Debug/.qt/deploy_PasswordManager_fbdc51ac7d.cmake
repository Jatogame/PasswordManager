include("/home/jakali/Documents/programmieren/PasswortManager/PasswordManager/build/Desktop_Qt_6_9_0-Debug/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/PasswordManager-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")

qt6_deploy_runtime_dependencies(
    EXECUTABLE /home/jakali/Documents/programmieren/PasswortManager/PasswordManager/build/Desktop_Qt_6_9_0-Debug/PasswordManager
    GENERATE_QT_CONF
)
