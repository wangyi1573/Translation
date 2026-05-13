TEMPLATE = lib
LANGUAGE = C++

CONFIG += qt warn_on Debug
QT += core gui widgets network

HEADERS +=  *.h \
    opencc/Export.hpp \
    opencc/SimpleConverter.hpp \
    opencc/opencc.h
SOURCES +=  *.cpp \
    volcenginetranslate.cpp \
    volcengineapi.cpp \
    volcenginetransconfig.cpp
FORMS +=  *.ui

INCLUDEPATH += ../../include
INCLUDEPATH += ../../qscint/src
INCLUDEPATH += ../../qscint/src/Qsci
INCLUDEPATH += ../../qscint/scintilla/src
INCLUDEPATH += ../../qscint/scintilla/include


win32 {
if (contains(QMAKE_HOST.arch, x86_64)) {
    CONFIG(Debug, Debug | Release) {
        DESTDIR = ../../x64/Debug/plugin
        LIBS += -L../../x64/Debug
            LIBS += -lqmyedit_qt5d
    }
    else {
        DESTDIR = ../../x64/Release/plugin
            LIBS += -L../../x64/Release
            LIBS += -lqmyedit_qt5
    }
}
}

unix {
if(CONFIG(debug, debug|Release)){
          LIBS += -L/home/yzw/build/CCNotepad/lib -lopencc
          LIBS += -L/home/yzw/build/CCNotepad/lib -lmarisa

    }else{
          LIBS += -L/home/yzw/build/CCNotepad/lib -lopencc
          LIBS += -L/home/yzw/build/CCNotepad/lib -lmarisa
    }
   }

unix {
   if(contains(QMAKE_HOST.arch, x86_64)){
    CONFIG(Debug, Debug|Release){
        DESTDIR = ../../x64/Debug/plugin
                LIBS += -L../../x64/Debug
                LIBS += -lqmyedit_qt5d
    }else{
        DESTDIR = ../../x64/Release/plugin
                LIBS += -L../../x64/Release
                LIBS += -lqmyedit_qt5
    }
   }
}

win32{
	if(contains(QMAKE_HOST.arch, x86_64)){
		if(CONFIG(Debug, Debug|Release)){
			LIBS += -Llib -lopencc
		}else{
			LIBS += --Llib -lopencc
		}
   }else{
		if(CONFIG(Debug, Debug|Release)){
			LIBS += -Llib -lopencc
		}else{
			LIBS += -Llib -lopencc
		}
	}
}

unix {
    UI_DIR = .ui
        MOC_DIR = .moc
        OBJECTS_DIR = .obj
}

TRANSLATIONS += ../../realcompare_zh.ts
