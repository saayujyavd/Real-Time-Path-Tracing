cls
del *.exe

nvcc -arch=sm_86 --use_fast_math .\src\main.cpp .\src\kernel.cu ^
.\src\Imgui\imgui.cpp ^
.\src\Imgui\imgui_draw.cpp ^
.\src\Imgui\imgui_widgets.cpp ^
.\src\Imgui\imgui_tables.cpp ^
.\src\Imgui\imgui_demo.cpp ^
.\src\Imgui\imgui_impl_glfw.cpp ^
.\src\Imgui\imgui_impl_opengl3.cpp ^
-allow-unsupported-compiler ^
-lopengl32 -luser32 -lgdi32 -lshell32 ^
-o RealTimePathTracing.exe -Xlinker /SUBSYSTEM:WINDOWS -Xlinker /ENTRY:mainCRTStartup

del *.obj
del *.exp
del *.lib

RealTimePathTracing.exe
