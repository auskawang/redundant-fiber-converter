# Embedded firmware setup

The embedded project lives in [`firmware/`](../firmware/). The Python laptop demo is separate; see the [root quick start](../README.md#run-the-virtual-demo).

## STM32CubeIDE

1. Clone the repository and open STM32CubeIDE with your workspace.
2. Open **File → Import → General → Existing Projects into Workspace**.
3. Select the repository directory as the root and let CubeIDE discover the project under `firmware/`.
4. Select the detected project and finish the import.
5. Build the project. If stale generated artifacts cause errors, clean and rebuild; inspect remaining toolchain/dependency errors individually.

These preserve the repository's existing import instructions. No embedded build or board programming was performed as part of the documentation cleanup; required CubeIDE/toolchain versions have not been independently verified here.
