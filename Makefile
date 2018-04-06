# Specify compiler
CC=cl
  
# Specify linker
LINK=link

# Name of program
NAME=KRbCam

# Specify libraries
LIBS=user32.lib atmcd64m.lib gdi32.lib

  
# Link the object files into a binary
KRbCam : main.obj krbcam.obj
    $(LINK) /OUT:$(NAME).exe main.obj krbcam.obj $(LIBS)
  
# Compile the source files into object files
main.obj : main.cpp
    $(CC) /c main.cpp /EHsc

# Compile the source files into object files
krbcam.obj : krbcam.cpp
    $(CC) /c krbcam.cpp /EHsc
  
# Clean target

.PHONY : clean
clean :
    del main.obj krbcam.obj $(NAME).exe