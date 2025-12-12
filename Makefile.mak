CC = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\cl.exe"

# /Wp64 옵션 제거
CFLAGS = /nologo /W3 /MD /D_CRT_SECURE_NO_WARNINGS /Z7

INCLUDE_PATH = /I"C:\vcpkg\installed\x64-windows\include"
LIB_PATH = /link /LIBPATH:"C:\vcpkg\installed\x64-windows\lib"

SRCS = main.c server.c httpParser.c router.c user.c video.c dbManager.c threadPool.c
OBJS = $(SRCS:.c=.obj)
TARGET = OTTstream.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) /Fe$@ $** $(LIB_PATH) sqlite3.lib libcrypto.lib libssl.lib cjson.lib ws2_32.lib legacy_stdio_definitions.lib /MACHINE:X64

main.obj: main.c
	$(CC) $(CFLAGS) /c $(INCLUDE_PATH) main.c

server.obj: server.c
	$(CC) $(CFLAGS) /c $(INCLUDE_PATH) server.c

httpParser.obj: httpParser.c
	$(CC) $(CFLAGS) /c $(INCLUDE_PATH) httpParser.c

router.obj: router.c
	$(CC) $(CFLAGS) /c $(INCLUDE_PATH) router.c

user.obj: user.c
	$(CC) $(CFLAGS) /c $(INCLUDE_PATH) user.c

video.obj: video.c
	$(CC) $(CFLAGS) /c $(INCLUDE_PATH) video.c

dbManager.obj: dbManager.c
	$(CC) $(CFLAGS) /c $(INCLUDE_PATH) dbManager.c

threadPool.obj: threadPool.c
	$(CC) $(CFLAGS) /c $(INCLUDE_PATH) threadPool.c

clean:
	del $(OBJS) $(TARGET)