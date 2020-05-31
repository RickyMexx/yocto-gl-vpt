# Yocto/GL: Volumetric Path Tracing

This project is designed and implemented as an extension of [Yocto/GL](https://xelatihy.github.io/yocto-gl/), a collection of small C++17 libraries for building physically-based graphics algorithms. All credits of Yocto/GL to [Fabio Pellacini](http://pellacini.di.uniroma1.it/) and his team.

## Introduction
As stated in the project title, we deal with volumetric path tracing ([VPT](https://en.wikipedia.org/wiki/Volumetric_path_tracing)) for heterogenous materials since it's not currently supported by the actual release of Yocto/GL. In particular, we focus our implementation on heterogeneous materials, adding full support for volumetric textures and [OpenVDB](https://www.openvdb.org/download/) models. Next, we proceed with the implementation of delta tracking algorithm presented in [PBRT](http://www.pbr-book.org/3ed-2018/Light_Transport_II_Volume_Rendering/Sampling_Volume_Scattering.html) book and a modern volumetric method proposed by [Miller et al](https://cs.dartmouth.edu/~wjarosz/publications/miller19null.html).

## Our work
The starting point of the project is to find a reliable and, of course, a working method to include OpenVDB data in Yocto. To do this ...

## Results
how well it worked, performance numbers and include commented images

## Credits
- [Riccardo Caprari](https://github.com/RickyMexx)
- [Emanuele Giacomini](https://github.com/EmanueleGiacomini)

## References
- [Yocto/GL](https://xelatihy.github.io/yocto-gl/)
- [A null-scattering path integral formulation of light transport](https://www.pbrt.org/), Miller, Bailey and Georgiev, Iliyan and Jarosz, Wojciech
- [Physically Based Rendering](https://www.pbrt.org/): from Theory to Implementation
- [Production Volume Rendering](https://graphics.pixar.com/library/ProductionVolumeRendering/paper.pdf), Pixar Animation Studios
- [OpenVDB](https://www.openvdb.org/)
- [Disney clouds](https://www.technology.disneyanimation.com/clouds)





