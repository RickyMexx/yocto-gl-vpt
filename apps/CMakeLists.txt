add_subdirectory(yscenetrace)

if(YOCTO_OPENGL)
add_subdirectory(ysceneitraces)
endif(YOCTO_OPENGL)

# Added by Emanuele Giacomini and Riccardo Caprari
add_subdirectory(yovdbload)
add_subdirectory(ytestbed)
