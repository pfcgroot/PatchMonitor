HEADERS       = window.h
SOURCES       = main.cpp \
                window.cpp
RESOURCES     = patchmonitor.qrc

QT += widgets

# install
# This utility depends on the following DLL's:
# Qt4.8.0: QtCore4.dll, QtGui4.dll
#         + Ming:   libgcc_s_dw2-1.dll, mingwm10.dll
#         + VS2010: msvcr100.dll, msvcp100.dll (< seems to have problems on the scanner)
# Qt5.0.1: icudt49.dll, icuin49.dll, icuuc49.dll, libEGL.dll, libGLESc2.dll, Qt5Core.dll, Qt5Gui.dll, Qt5Widgets.dll (and windows plugin?)
# Use qt.conf to specify these dependencies if not included in the application dir.
target.path = G:/Site/AMC/PatchMmonitor
INSTALLS += target

simulator: warning(This utility might not fully work on Simulator platform)
