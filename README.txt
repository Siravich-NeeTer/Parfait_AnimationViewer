Course: CS-460 Fall 2024
Name: Siravich Sereepong [Ping] (0066946)

============================= Student Hardware =============================
Operating System: Windows 11 (x64)
IDE: Microsoft Visual Studio 2022 (version 17.8.4)
Compiler: C++ / MSVC

============================= Implementation Details =============================
- Lerp/Slerp/Elerp functions were implemented in MathUtility.h
- glm::quat was replaced by custom Quaternion (Quaternion.h) which contain basic operators overload Addition, Subtraction, Multiplication, Inverse, ToMatrix
- VQS Implementation contain basic operators overload and ToMatrix with additional function to convert matrix to VQS
- Bone.h/.cpp will contain the integration of VQS and Quaternion regarding bone transformation including interpolation.

*Note: Quaternion and VQS contain the function ToMatrix for sending those data to GPU(Shader) which will be combined with projection and view matrix later on.

============================= How to run program =============================
- Users must have VulkanSDK(1.3.268.0 or newer) for running the program.
- User can run .exe file through siravich.sereepong-CS460-proj-1-exe > Parfait_AnimationViewer.exe
- Users can also run through the visual project but make sure it must be on release mode for better performance.

============================= Resources =============================
- Mixamo by Adobe (Models & Animations)
- Assimp (Models & Animations Loading Library)
- ImGui
- Vulkan Graphics API



