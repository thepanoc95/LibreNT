CC := x86_64-w64-mingw32-gcc
WINDRES := x86_64-w64-mingw32-windres
CFLAGS := -Wall -Wextra -O2
LDFLAGS := -mwindows -lgdi32 -luser32 -lcomctl32 -lshell32

SOURCES_FBSH := fallback.explorer.c
SOURCES_WELCOME := welcome/welcome.c
SOURCES_WINVER := winver/winver.c winver/osinfo.c

# WinUI 3 Shell (requires dotnet SDK on Windows)
SHELL_DIR := shell
SHELL_PROJECT := $(SHELL_DIR)/LibreNT.Shell.csproj
SHELL_OUTPUT := $(SHELL_DIR)/bin/Release/net8.0-windows10.0.22621.0/win-x64/LibreNT.Shell.exe

.PHONY: all desktop welcome winver shell clean

all: desktop welcome winver shell

desktop: $(SOURCES_FBSH)
	$(CC) $(CFLAGS) -o explorerfallback $(SOURCES_FBSH) $(LDFLAGS)

welcome: $(SOURCES_WELCOME)
	$(CC) $(CFLAGS) -o welcome $(SOURCES_WELCOME) $(LDFLAGS)

winver: winver/winver.rc
	$(WINDRES) -i winver/winver.rc -o winver.res -O coff -I winver
	$(CC) $(CFLAGS) -o winver.exe $(SOURCES_WINVER) winver.res -municode $(LDFLAGS)

shell: $(SHELL_PROJECT)
	cd $(SHELL_DIR) && dotnet publish -c Release -r win-x64 --self-contained false /p:UseWinUI=true

clean:
	rm -f *.exe *.o winver.res
	rm -rf $(SHELL_DIR)/bin $(SHELL_DIR)/obj
