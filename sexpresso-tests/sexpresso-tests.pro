QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle
CONFIG += c++14
TEMPLATE = app

SOURCES +=  tst_sexpressotests.cpp


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../release/ -lsexpresso
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../debug/ -lsexpresso
else:unix: LIBS += -L$$OUT_PWD/../ -lsexpresso

INCLUDEPATH += $$PWD/../
DEPENDPATH += $$PWD/../
message(pwd)
message($$PWD)
message(incpath)
message($$INCLUDEPATH)
message(thelibs)
message($$LIBS)
win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../release/libsexpresso.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../debug/libsexpresso.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../release/sexpresso.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../debug/sexpresso.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../libsexpresso.a
