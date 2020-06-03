#./bin/yscenetrace tests/volume/volume.json -o out/lowres/volume_720_256.jpg -t path -s 256 -r 720
#./bin/yscenetrace tests/smoke/out.json -o out/lowres/smoke_720_256.jpg -t path -s 256 -r 720
#./bin/yscenetrace tests/03_volumetric_lights/volume_testscene.json -o out/lowres/volumetric_light_720_256.jpg -t path -s 256 -r 720
#./bin/yscenetrace tests/03_volumetric_lights/01.json -o out/lowres/01_720_256.jpg -t path -s 256 -r 720

# Delta tracking - Spectral MIS comparison
#./bin/yscenetrace tests/03_volumetric_lights/02.json -o out/lowres/03_spectral_720_256.jpg -t path -s 256 -r 720 -v spectral
#./bin/yscenetrace tests/03_volumetric_lights/02.json -o out/lowres/03_delta_720_256.jpg -t path -s 256 -r 720 -v delta
./bin/yscenetrace tests/04_lightroom/01.json -o out/lowres/04_delta_720_256_01.jpg -t path -s 256 -r 720 -v delta
./bin/yscenetrace tests/04_lightroom/01.json -o out/lowres/04_spectral_720_256_01.jpg -t path -s 256 -r 720 -v spectral

