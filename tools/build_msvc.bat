mkdir bin 2> nul

::set FLAGS=/W4
set FLAGS=

:: cl %FLAGS% /Fe:bin\fttp src\fttp.c ws2_32.lib
:: cl %FLAGS% /Fe:bin\color_picker src\color_picker.c gdi32.lib user32.lib opengl32.lib
cl %FLAGS% /Fe:bin\music_player src\music_player.c ole32.lib
:: cl %FLAGS% /Fe:bin\image_converter src\image_converter.c
:: cl %FLAGS% /Fe:bin\image_viewer src\image_viewer.c gdi32.lib user32.lib opengl32.lib shell32.lib
