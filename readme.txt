A small Windows / Mac app to test GLSL shader loading speed.

TL;DR: even simple offline GLSL "pre-optimization" improves shader loading
speed, often 2x faster to load on Mac! Loading D3D9 assembly shaders is several
times faster than loading GLSL ones.


Measuring process:
1. load shaders from files
2. compile super small empty shader (to load/"warmup" the compiler)
3. set 1x1 pixel viewport
4. start timer
5. compile & link the shaders
6. set the program, draw two triangles (some drivers defer final compilation until first use)
7. delete shader
8. glFinish()
9. stop the timer

Several shaders included:
* Tree Leaf shader with per pixel lighting and some translucency calculations
* FXAA 3.11 PC-39 preset
* Deferred Lighting light application shader
* Parallax normal mapping with self-illumination
* Naive "port" of part of Mental Ray's architectural shader (not optimized)
* SSAO


For each shader, it's "raw" form is there as produced by hlsl2glslfork[1] and the
same shader, still in GLSL, but pre-optimized offline by glsl-optimizer[2]. For some shaders,
D3D9 shader assembly is provided and performance of loading that is measured when on Windows.

See results.txt. Even loading "still a GLSL" shaders, but pre-optimized a bit, is faster.
On Mac it's faster quite a lot!

A big game might use 1000 to 10000 shaders. According to these results, even simple offline
pre-optimization can save 3-10ms from each shader loading time. That is 3 to 100 seconds saved
from loading time. Comparing loading speed to D3D9 bytecode loading speed, the savings are even
larger!
 

[1]: https://github.com/aras-p/hlsl2glslfork
[2]: https://github.com/aras-p/glsl-optimizer
