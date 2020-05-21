# Yocto/PathExtension: Tiny Path Tracer Extension

In this homework, you will implement a new path tracing feature from scratch
and without assistance. In this assignment, you will have to chose one project 
between the ones suggested below. To make sure the assignment is inclusive we 
propose topics that are either algorithmic improvements, system implementations
or large scene creations. Choose the assignment that better fits your interest
and consider carefully the amount of time you will have to implement the feature.

To complete the assignment, you will have to

- **work either alone or in groups of two**, according to the indications of 
  each project; we advise to work in pairs
- develop an extension to either the Yocto/Trace renderer or the solution of 
  the previous assignment that we include here --- if possible duplicate the 
  code you needed in the Yocto extension of your choosing 
- hand in the new code, scenes and demos as explained for each project
- **write a relatively detailed report in a readme.md file** --- this should 
  include what you did, how well it worked, performance numbers and include 
  commented images 
- **declare group members in the readme file right after the title**
- if your project works really well, we will try to include it in the next Yocto
  release, and give you credit for it
- each of the project will be discussed by the professor in class --- you will
  then interact with the professor during the assignment to understand how to 
  better implement it

## Framework

If you still need details, see the descriptions on previous homeworks.

## Functionality

Pick one of the projects below.

### Algorithmic Projects

- **Hair Shading (1-2 people, medium)**:
    - implement a hair BSDF to shade realistic-looking hairs
    - you can follow the algorithm presented in [pbrt](https://www.pbrt.org/hair.pdf)
      that also includes a full code implementation
    - you can get example hair models from [Bitterli](https://benedikt-bitterli.me/resources/)
- **AI Denoising (1 person, easy)**:
    - integrate the [Intel Open Image Denoise](https://openimagedenoise.github.io)
    - modify the renderer to output what is needed for the denoiser
    - write a new app `yimagedenoise` that takes the new images in and output the denoised ones
    - favor HDR image processing but support both HDR and LDR
    - compare this to your own implementation of an non-local-means denoiser; 
      you can grab the code from anywhere you want
- **Bayesian Denoising (2 people, easy)**:
    - integrate the [Bayesian Collaborative Denoising](https://github.com/superboubek/bcd)
    - modify the renderer to output what is needed for the denoiser
    - write a new app `yimagedenoise` that takes the new images in and output the denoised ones
    - favor HDR image processing but support both HDR and LDR
    - compare this to your own implementation of an non-local-means denoiser; 
      you can grab the code from anywhere you want
- **Monte Carlo Geometry Processing (1-2 people, medium)**:
    - this is really beautiful new work
    - implement a few geometry processing functions using Monte Carlo methods
    - you should be familiar with geometry processing ideas already
    - take any examples from [Crane](http://www.cs.cmu.edu/~kmcrane/Projects/MonteCarloGeometryProcessing/paper.pdf)
    - example code in 2D from [here](http://www.cs.cmu.edu/~kmcrane/Projects/MonteCarloGeometryProcessing/WoSLaplace2D.cpp.html)
    - example code in 3D from [here](https://twitter.com/keenanisalive/status/1258152695074033664)
- **Volumetric Path Tracing (2 people, hard)** for heterogenous materials:
    - implement proper volumetric models for heterogenous materials
    - implement volumetric textures, for now just using `image::volume`, 
      for density and emission
    - add these textures to both the SceneIO loader and the path tracer
    - on this data structure, implement delta tracking as presented in [pbrt](http://www.pbr-book.org/3ed-2018/Light_Transport_II_Volume_Rendering/Sampling_Volume_Scattering.html)
    - also implement a modern volumetric method to choose from
        - Spectral Tracking from [Kutz et al](https://s3-us-west-1.amazonaws.com/disneyresearch/wp-content/uploads/20170823124227/Spectral-and-Decomposition-Tracking-for-Rendering-Heterogeneous-Volumes-Paper1.pdf)
        - pseudocode of Algorithm 1 of [Miller et al](https://cs.dartmouth.edu/~wjarosz/publications/miller19null.html) --- ignore most of the math here since it is not that helpful
    - get examples from OpenVDB
- **Texture Synthesis (1 person, easy)**:
    - implement texture synthesis following the [Disney method](http://www.jcgt.org/published/0008/04/02/paper.pdf)
    - [source code](https://benedikt-bitterli.me/histogram-tiling/)
    - for this, just add properties to the `trace::texture` object and change `eval_texture()`
    - you should alsop provide a 2D image generator for it in the spirit of the code above but in C++
- **SDF Shapes (1-2 people, medium)**:
    - implement a high quality SDF shape object that is integrated within the path tracer
    - represent SDFs as sparse hash grids; an [example on GPU](https://nosferalatu.com/SimpleGPUHashTable.html)
    - add a new `sdf` to represent the SDF and have `object` hold a pointer to either `sdf` or `shape`
    - change `eval_xxx()` to work for SDFs too
    - add code to load/save SDFs in `sceneio`; just make up your own file format for now
    - get examples from OpenVDB
- **Adaptive rendering (1 person - easy)**
    - implement a stropping criterion to focus render resources when more needed
    - technique is described [here](https://jo.dreggn.org/home/2009_stopping.pdf)
    - possible implementation [here](https://github.com/mkanada/yocto-gl) 
    - your job here is to integrate really well the code, provide test cases and make it run interactively
- **Better adaptive rendering (1-2 person - medium)**
    - implement adaptive rendering and reconstruction as described [here](https://www.uni-ulm.de/fileadmin/website_uni_ulm/iui.inst.100/institut/Papers/atrousGIfilter.pdf)
    - this will provide better quality than the above method
    - alsop consider the [new variant from NVidia](https://www.highperformancegraphics.org/wp-content/uploads/2017/Papers-Session1/HPG2017_SpatiotemporalVarianceGuidedFiltering.pdf)

### System Projects

- **Yocto/Python (1-2 people, easy-medium)**
    - provide Python binding to the main Yocto/GL functionality so that 
      we can call Yocto from Python
    - generate the binding using [PyBind11](https://github.com/pybind/pybind11)
    - providing full coverage of the entire API is not helpful at this point
    - instead focus on providing enough coverage to be able to
        - write a `yscenetrace.py` that is a Python script that can run a 
          path tracer similar to `yscenetrace.cpp` 
        - similarly write a `ysceneproc.py`, `yimageproc.py` and `yshapeprc.py`
        - you should be able to call Yocto from inside Python Jupyter and see 
          images rendered by Yocto in Jupyter
        - extra credit: run the interactive path tracer inside Python Jupyter
        - make sure we can pip install Yocto
    - non-goals: do not worry about supporting the entire Yocto api
- **Yocto/QBVH (1 person, medium)**
    - implement a QBVH tree by translating our BVH2 to a BVH4 as described [here](http://jcgt.org/published/0004/04/05/)
    - implement QBVH intersection with only one ray at a time
    - optimize QBVH intersection using SIMD instruction
    - for example code for the optimization look for QBVH on 
- **Yocto/BVHBuild (1 person, medium)**
    - implement faster BVH build as described [here](http://jcgt.org/published/0004/03/02/)
- **Yocto/BVHMorton (1 person, medium)**
    - implement faster BVH build as described [here](https://www.highperformancegraphics.org/wp-content/uploads/2017/Papers-Session3/HPG207_ExtendedMortonCodes.pdf)
    - source code is available from the authors
- **NoYocto/PyTrace (2 people, medium)**
    - do not use yocto for this
    - write a path tracer in pure Python code
    - use Intel's embree python wrapper
    - your code should show images similar to hoemwork02, skip volumes
    - to get the speed right, use `numpy` to hold all data and `jax` or `numba` to JIT fast code
    - also write your path tracer in a wavefront fashion, which means that all actions are repeated for the whole image
    - so, instead of runing a loop over pixel and then follow a single path, send all paths at once, ona wavefront, and iterate over the wavefront
- **Yocto/Web (1-2 people, medium-hard)**
    - run the Yocto/Pathtracer in the browser using [emscripten](https://emscripten.org)
    - I know little about this so I cannot help you that much
- **Yocto/GPU (2 people, hard)**
    - Try to run Yocto on the GPU in any way you want, e.g. CUDA
    - For ray-intersection either compile our code or try [NVidia/Optix](https://developer.nvidia.com/optix)
    - I know little about this so I cannot help you that much
- **NoYocto/Optix (2 people, hard)**
    - I know little about this so I cannot help you that much
    - Write a GPU path tracer outside of Yocto using [NVidia/Optix](https://developer.nvidia.com/optix)
    - This library is targeted at high-quality GPU rendering and used by many industry GPU tracers
    - Use as much Yocto as useful
- **NoYocto/DXR (2 people, hard)**
    - I know little about this so I cannot help you that much
    - Write a GPU path tracer outside of Yocto using DXR
    - This is targeted at games more than final quality rendering
    - Use as much Yocto as useful
    - [Tutorial](http://intro-to-dxr.cwyman.org)
    - Maybe try this framework [Falcor](https://github.com/nvidiagameworks/falcor)

### Scene Creation Projects

- **MYOT (1 person, medium)**, make your own tree:
    - write a tree generator that creates 3D trees
    - either implement a space colonization method from 
      [TreeSketch](https://www.researchgate.net/publication/235765743_TreeSketch_Interactive_Procedural_Modeling_of_Trees_on_a_Tablet)
      and [Runions](http://algorithmicbotany.org/papers/colonization.egwnp2007.large.pdf)
    - or implement a parametric generator from [Weber and Penn](https://www2.cs.duke.edu/courses/cps124/fall01/resources/p119-weber.pdf) 
      used in [sapling tree gen](https://github.com/abpy/improved-sapling-tree-generator)
- **MYOC (1 person, medium)**, make your own city:
    - create a city-size scene yo show off Yocto scalability
    - use any asset you want that is CC licensed
    - start from openstreetmaps
    - this will require coding
- **MYOF (1 person, medium)**, make your own forest:
    - create a large, detailed, natural scene like a forest
    - use any asset you want that is CC licensed
    - or use generators like Blender trees and grass
    - write a command line utility that assemble these assets onto a plane 
      randomly to create a realistic environment
    - see tutorials on the web on how to do procedural placement (it is very easy)
    - modify `ysceneproc` for this
- **MYOH (1 person, easy)**, make your own homework:
    - make small to large environment to show off Yocto/GL that we can use for next year homeworks
    - these environments should show off the rendering features we demonstrated
    - we are particularly interested in subdiv examples, displacement maps, etc
    - the scenes should be compelling and of quality similar to the ones proposed this year
    - remember to use CC licensed assets only, and include the license for them
- **MYOS 2.0 (1 person, easy)**, make your own scene 2.0:
    - create additional scenes that are _really compelling to look at_
    - for this assignment, do not worry about license; use anything you want
    - include paid models if really want to, but only if you really want to; 
      you are not required at all for this, but I have to be honest and say that 
      those are the best looking models
    - the scenes have to be really compelling
    - something like [cgtrader](https://www.cgtrader.com/3d-models/interior)
      or [evermotion](https://evermotion.org/shop)
    - or like [blender](https://www.blender.org/download/demo-files/)
    - how you convert them is up to you

## Grading

In this assignment, there is no extra credit. Instead the assignment will 
have a higher point grade than previous assignments. We will grade on both
difficulty and quality of the resulting images/demos and also take into
account the number of people in the group.

## Submission

To submit the homework, you need to pack a ZIP file that contains all the code
you wrote, a readme file, all scenes and images you generated.
**Both people in the group should submit the same material.**
The file should be called `<lastname>_<firstname>_<studentid>.zip` 
(`<cognome>_<nome>_<matricola>.zip`) and you should exclude 
all other directories. Send it on Google Classroom.
