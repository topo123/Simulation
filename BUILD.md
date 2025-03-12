#Building The Simulation

##Prequisites
To build on linux you need cmake and make installed.
To build on windows you need cmake and ninja installed. 

##Building on Windows:

1. In the root directory of the project make a build directory 
2. In the root directory of the project use the command: cmake -B build -G "Ninja
2. Then run the command ninja -C build
3. These should create the build files 


##Building On Linux:
1. On linux you can either use ninja or unix makefiles
2. Make the build directory in the root directory
3. Use either the command cmake -B build for Unix Makefiles or use the windows command 
   if you want to build with ninja
4. Run make -C build if you use Unix Makefiles otherwise use the ninja build command for windows
3. This should create the build files


##Running the Project
To run the project navigate to the root directory and type into console "./build/main"
