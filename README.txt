Course: CS-460 Fall 2024
Name: Siravich Sereepong [Ping] (0066946)

============================= Student Hardware =============================
Operating System: Windows 11 (x64)
IDE: Microsoft Visual Studio 2022 (version 17.8.4)
Compiler: C++ / MSVC

============================= Implementation Details =============================
- Render path with C1 continuity by using Catmull-Rom (src/Math/MathUtility.h -> CatmullRom())
- Arc-Length table construction using adaptive approach (src/Core/Curve.h -> BuildTable())
* BuildTable() will get call when we add the new control points also aka. Table concatenation
* BinarySearch was used for finding inverse arc-length function aka. Query the points
- Ease in/out was implemented in UpdateVelocityTable() and UpdateDistanceTable()
* Using ImGui Graph for user-input control the velocity/distance-time graph
* Center of interest was implemented in (src/Core/Animator.h -> MoveAlongPath()) by computing each pitch, yaw and roll
- Animation Blending was calculate in (src/Core/Animator.h -> CalculateBoneTransform()) by interpolating the bone from current animation to next animation

============================= How to run program =============================
- Users must have VulkanSDK(1.3.268.0 or newer) for running the program.
- User can run .exe file through siravich.sereepong-CS460-proj-2-exe > Parfait_AnimationViewer.exe
- Users can also run through the visual project but make sure it must be on release mode for better performance.

============================= Control =============================
- Camera control like Unity, Unreal Engine
-- Hold RMB(Viewport Tab) + W/A/S/D = Moving Camera
-- Hold RMB(Viewport Tab) + Q/E = Moving Camera (Up/Down)
-- Mouse Scroll(Viewport Tab) = Zoom in/out

============================= Resources =============================
- Mixamo by Adobe (Models & Animations)
- Assimp (Models & Animations Loading Library)
- ImGui inc. ImPlot, ImGizmo
- Vulkan Graphics API



