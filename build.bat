cls
del *.exe

nvcc -arch=sm_86 --use_fast_math .\src\main.cpp .\src\kernel.cu ^
C:\imgui-master\imgui.cpp ^
C:\imgui-master\imgui_draw.cpp ^
C:\imgui-master\imgui_widgets.cpp ^
C:\imgui-master\imgui_tables.cpp ^
C:\imgui-master\imgui_demo.cpp ^
C:\imgui-master\backends\imgui_impl_glfw.cpp ^
C:\imgui-master\backends\imgui_impl_opengl3.cpp ^
-allow-unsupported-compiler ^
-lopengl32 -luser32 -lgdi32 -lshell32 ^
-o out.exe -Xlinker /SUBSYSTEM:WINDOWS -Xlinker /ENTRY:mainCRTStartup

del *.obj
del *.exp
del *.lib

out.exe
