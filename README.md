# Yocto/GL: Volumetric Path Tracing

This project is designed and implemented as an extension of [Yocto/GL](https://xelatihy.github.io/yocto-gl/), a collection of small C++17 libraries for building physically-based graphics algorithms. All credits of Yocto/GL to [Fabio Pellacini](http://pellacini.di.uniroma1.it/) and his team.

![test](/out/lowres/campfire_spectral_1280_4096.jpg)

## Introduction
As stated in the project title, we deal with volumetric path tracing ([VPT](https://en.wikipedia.org/wiki/Volumetric_path_tracing)) for heterogenous materials since it's not currently supported by the actual release of Yocto/GL. In particular, we focus our implementation on heterogeneous materials, adding full support for volumetric textures and [OpenVDB](https://www.openvdb.org/download/) models. Next, we proceed with the implementation of delta tracking algorithm presented in [PBRT](http://www.pbr-book.org/3ed-2018/Light_Transport_II_Volume_Rendering/Sampling_Volume_Scattering.html) book and two modern volumetric methods: spectral tracking from [Kutz et al](https://s3-us-west-1.amazonaws.com/disneyresearch/wp-content/uploads/20170823124227/Spectral-and-Decomposition-Tracking-for-Rendering-Heterogeneous-Volumes-Paper1.pdf) and unidirectional spectral MIS proposed by [Miller et al](https://cs.dartmouth.edu/~wjarosz/publications/miller19null.html).

## Our work
### OpenVDB support
The starting point of the project is to find a reliable and, of course, a working method to include OpenVDB data in Yocto. To do this we exploit data grids of the OpenVDB volumes and dump them using the official [repo](https://github.com/AcademySoftwareFoundation/openvdb). A volume is made of voxels generically containing a float value that indicates the level of a property. In particular, we are dealing with density and, if present, temperature volumes.

To perform the conversion from *.vdb* to a Yocto-compatible format we start by looking at the grids the volume has using ```openvdb_print``` application. Usually, as said before, a volume contains a grid for the density and optionally one for the temperature. We dump these grids using a custom function and generating a *.bin* file for each of them. These files contain xyz coordinates of each voxel and the float value of the current property.

The format used for the Yocto-compatible files is *.vol*. We obtain these using ```yovdbload```, which is a new application we implemented for volumes support that exploits ```yocto::image::save_volume()```. In this part we find the bounding voxels coordinates of the volumes and, in case they are negative, translate them to have the lower bound in ```(0, 0, 0)``` and the max bound in ```(|min_x|+max_x, |min_y|+max_y, |min_z|+max_z)```, in which min and max are the smallest and biggest voxel coordinates.


### Yocto/GL volumes add-ons

We introduce some changes in the original Yocto structure:
- New object properties in JSON files
- Volumes support in ```yocto/yocto_sceneio.{h,cpp}``` and ```yocto_pathtrace/yocto_pathtrace.{h,cpp}```
- New options in ```yscenetrace``` and ```ysceneitraces``` apps
- New ```yovdbload``` app for volumes extraction 
- Insertion of ```yocto_extension/yocto_extension.{h,cpp}``` with the implemented algorithms
- Adapted path tracer routine in ```yocto_pathtrace/yocto_pathtrace.{h,cpp}```

Since we are dealing with a new type of objects, heterogenous volumes, we have to change a little bit the JSON features. We add a dedicated ```/volumes``` folder to store *.vol* files and a bunch of custom settings. Here is a list of the new properties that can be added to objects:


JSON objects add-on | Type | Description
:---: | :---: | -------------
```density_vol``` | .vol | Volume name containing density voxels
```emission_vol``` | .vol | Volume name containing temperature voxels
```scale_vol``` | vec3f | XYZ scale of the volume
```offset_vol``` | vec3f | XYZ center offset of the volume
```density_mult``` | float | Handles the level of density of the volume
```radiance_mult``` | float | Handles the level of radiance the volume emits

All those features are then handled by ```yocto::sceneio``` and ```yocto::pathtrace``` that we change accurately. After adding also volumes support to the rendering apps ```yscenetrace``` and ```ysceneitraces```, we introduce a new option for the execution permitting us to choose one of the volumetric path tracing algorithms: ```--vpt,-v {delta, spectraltracking, spectralMIS}```.

Concerning the algorithms, ```yocto::extension``` contains all the needed functions. Here is a listing:

Function  | Description
:---: | -------------
```check_bounds()``` | Checks if XYZ indices are inside the bounds
```has_vpt_volume()``` | Checks if an object is a heterogenous volume
```has_vpt_density()``` | Checks if a volume has density voxels
```has_vpt_emission()``` | Checks if a volume has temperature voxels
```eval_vpt_density()``` | Evaluates the density value at the given real coordinates
```eval_vpt_emission()``` | Evaluates the temperature value at the given real coordinates
```eval_delta_tracking()``` | Implementation of delta tracking algorithm
```eval_spectral_tracking()``` | Implementation of spectral tracking algorithm
```eval_unidirectional_spectral_mis()``` | Implementation of unidirectional spectral MIS algorithm

We use those methods to compute all the necessary parameters like the density in a ray intersection point or the path tracing in volumes using different techniques. In particular, volumes are implemented using ```yocto::image::volume``` structure and other functions from the same namespace like ```yocto::image::eval_volume()``` which evaluates a voxel value at given coordinates. Further details on the methods implemented [here](/libs/yocto_extension/yocto_extension.h).

The structure for the ```vsdf``` is changed as following:
```
  struct vsdf {
    vec3f density    = {0, 0, 0};
    vec3f scatter    = {0, 0, 0};
    float anisotropy = 0;
    bool  htvolume   = false;
    vec3f scale = {1.0, 1.0, 1.0};
    int event = 0;
    const trace::object* object = nullptr;
  };
```
It now contains heterogeneous volume flag for the path trace, a scale for voxels evaluation, the type of event happening and the object to which it refers to retrieve other information. The events are divided in ```{EVENT_NULL, EVENT_SCATTER, EVENT_ABSORB}``` and permits us to track the current situation of the rays path. The first one stands for the case when a null collision happened and means that a fictitious particle has been hit. We remind that in null-scattering algorithms we can see a heterogeneous volume as a mixture of real and fictitious particles and proceed on a path direction if these latter have been hit. The second event refers to the scattering case when a real particle has been hit and we have to scatter the ray. Finally, *EVENT_ABSORB* is the case when there is an absorption of the ray which leads to its extinction.

The structure of the path tracer in ```yocto::pathtrace``` has undergone some changes to deal with the three different volumetric algorithms. A parameter ```std::string vpt``` is added to the ```trace_params``` structure in order to know the type of tracing. This value is automatically linked with the rendering app. The flow of the trace is a little different from Yocto's one because of a different approach used. In fact, we compute the weights and the distances of the three volumetric algorithms inside the functions themself instead of taking a step-by-step approach in the main ```pathtrace```. The weights are then multiplied by the main weight of the current trace. This new approach, which "forecasts" the total interaction in a single function step (that is O(n) with n the number of sampled distances) and which is also recommended by our references, shows a nice speedup on the computation of the paths. We also decide to compute the transmittance along the path in a simultaneous way instead of performing another cycle which would lead to O(n<sup>2</sup>). This further change scores a remarkable speedup.

### Delta Tracking

The first algorithm, and the classic one if we want to say, we focused on is delta tracking.

### Spectral Tracking

// to-do

### Unidirectional Spectral MIS

// to-do

## Results
how well it worked, performance numbers and include commented images

// to-do

## Credits
- [Riccardo Caprari](https://github.com/RickyMexx)
- [Emanuele Giacomini](https://github.com/EmanueleGiacomini)

## References
- [Yocto/GL](https://xelatihy.github.io/yocto-gl/)
- [Spectral and Decomposition Tracking for Rendering HeterogeneousVolumes](https://s3-us-west-1.amazonaws.com/disneyresearch/wp-content/uploads/20170823124227/Spectral-and-Decomposition-Tracking-for-Rendering-Heterogeneous-Volumes-Paper1.pdf), Kutz, Peter & Habel, Ralf & Li, Yining & Novák, Jan.
- [A null-scattering path integral formulation of light transport](https://cs.dartmouth.edu/~wjarosz/publications/miller19null.html), Miller, Bailey and Georgiev, Iliyan and Jarosz, Wojciech
- [Physically Based Rendering](https://www.pbrt.org/): from Theory to Implementation
- [Production Volume Rendering](https://graphics.pixar.com/library/ProductionVolumeRendering/paper.pdf), Pixar Animation Studios
- [OpenVDB](https://www.openvdb.org/)
- [Disney clouds](https://www.technology.disneyanimation.com/clouds)





