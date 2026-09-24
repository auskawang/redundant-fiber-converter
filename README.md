# Ethernet-to-Fiber Media Converter with Proactive Fault Detection and Redundant Failover
### Summary
The rapid growth of artificial intelligence is driving the construction of larger, faster, and more densely interconnected data centers. As network bandwidth and cable distance increase, copper interconnects experience greater electrical loss and require additional power for signal transmission and cooling. These demands increase operating costs and contribute to the environmental impact of modern computing infrastructure. Optical fiber is therefore becoming increasingly prevalent because it supports high data rates over long distances with lower transmission loss and immunity to electromagnetic interference.
Although optical fiber improves network speed and efficiency, an optical link can gradually lose operating margin because of connector contamination, fiber bending, temperature variation, component aging, or declining transceiver performance. Conventional media converters generally indicate whether a link is active but provide little ability to act on degradation before errors or complete loss of connectivity occur. This project addresses that limitation with an intelligent Ethernet-to-fiber converter that continuously monitors optical diagnostics and uses a proactive fiber-maintenance algorithm to transfer traffic to a redundant path before the active link fails.

### Clone and Build Instructions
1. Clone to whatever location and open STM32CubeIDE. Open a workspace when prompted.
2. Open the Project Explorer tab (Window -> Show View -> Project Explorer)
3. File -> Import -> General -> Existing Projects into Workspace. Select the "Select root directory" option and input the root directory where the cloned repo is stored, check the box next to the name of the repo in the Projects results box, then click Finish. The project will appear in the Project Explorer.
4. Select the project in Project Explorer on left hand side.
5. Build
