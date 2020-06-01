# Yocto/GL: Volumetric Path Tracing

This project is designed and implemented as an extension of [Yocto/GL](https://xelatihy.github.io/yocto-gl/), a collection of small C++17 libraries for building physically-based graphics algorithms. All credits of Yocto/GL to [Fabio Pellacini](http://pellacini.di.uniroma1.it/) and his team.

<img src="/out/lowres/smoke_720_256.jpg" width="1000" alt="Volumetric Path Tracing" class="inline"/>

## Introduction
As stated in the project title, we deal with volumetric path tracing ([VPT](https://en.wikipedia.org/wiki/Volumetric_path_tracing)) for heterogenous materials since it's not currently supported by the actual release of Yocto/GL. In particular, we focus our implementation on heterogeneous materials, adding full support for volumetric textures and [OpenVDB](https://www.openvdb.org/download/) models. Next, we proceed with the implementation of delta tracking algorithm presented in [PBRT](http://www.pbr-book.org/3ed-2018/Light_Transport_II_Volume_Rendering/Sampling_Volume_Scattering.html) book and a modern volumetric method proposed by [Miller et al](https://cs.dartmouth.edu/~wjarosz/publications/miller19null.html).

## Our work
### OpenVDB support
The starting point of the project is to find a reliable and, of course, a working method to include OpenVDB data in Yocto. To do this we exploit data grids of the OpenVDB volumes and dump them using the official [repo](https://github.com/AcademySoftwareFoundation/openvdb). A volume is made of voxels generically containing a float value that indicates the level of a property. In particular, we are dealing with density and, if present, temperature volumes.

To perform the conversion from *.vdb* to a Yocto-compatible format we start by looking at the grids the volume has using ```openvdb_print``` application. Usually, as said before, a volume contains a grid for the density and optionally one for the temperature. We dump these grids using a custom function and generating a *.bin* file for each of them. These files contain xyz coordinates of each voxel and the float value of the current property.

The format used for the Yocto-compatible files is *.vol*. We obtain these using ```yovdbload```, which is a new application we implemented for volumes support that exploits ```yocto::image::save_volume()```. In this part we find the bounding voxels coordinates of the volumes and, in case they are negative, translate them to have the lower bound in ```(0, 0, 0)``` and the max bound in ```(|min_x|+max_x, |min_y|+max_y, |min_z|+max_z)```, in which min and max are the smallest and biggest voxel coordinates.

![test](/out/lowres/smoke_720_256.jpg)


### Yocto/GL volumes add-ons

JSON objects add-on | Type | Description
:---: | :---: | -------------
```density_vol``` | .vol | Volume containing density voxels
```emission_vol``` | .vol | Volume containing temperature voxels
```scale_vol``` | vec3f | XYZ scale of the volume
```offset_vol``` | vec3f | XYZ center offset of the volume
```density_mult``` | float | Handles the level of density of the volume
```radiance_mult``` | float | Handles the level of radiance the volume emits

In ```yocto::extension```:
Function  | Description
:---: | -------------
```check_bounds()``` | Checks if XYZ indices are inside the bounds
```has_vpt_volume()``` | Checks if an object is a heterogenous volume
```has_vpt_density()``` | Checks if a volume has density voxels
```has_vpt_emission()``` | Checks if a volume has temperature voxels
```eval_vpt_density()``` | Evaluates the density value at the given real coordinates
```eval_vpt_emission()``` | Evaluates the temperature value at the given real coordinates
```delta_tracking()``` | Implementation of delta tracking algorithm
```spectral_MIS()``` | Implementation of unidirectional spectral MIS algorithm


### Delta tracking

### Unidirectional Spectral MIS

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





