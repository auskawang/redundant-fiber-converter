### Clone and Build Instructions
1. Clone to whatever location and open STM32CubeIDE. Open a workspace when prompted.
2. Open the Project Explorer tab (Window -> Show View -> Project Explorer)
3. File -> Import -> General -> Existing Projects into Workspace. Select the "Select root directory" option and input the root directory where the cloned repo is stored, check the box next to the name of the repo in the Projects results box, then click Finish. The project will appear in the Project Explorer.
4. Select the project in Project Explorer on left hand side.
5. Build, if error occurs try clean and build again

### Logged Errors
Linker errors (> 100) with HAL functions, seems to happen when switching branches. Try deleting from workspace and importing it again, clean and build, or remove the .cproject and .project files. 