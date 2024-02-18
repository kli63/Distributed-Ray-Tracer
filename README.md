YouTube Link to 10 Second Animation:
https://www.youtube.com/watch?v=01X9Vn5kKkY

Effects:
1. Soft Shadows - see shadow of the spheres
2. Glossy Reflection - the reflective sphere uses glossy reflection so its reflections are blurry
3. Depth of Field - the camera focuses in on the subject within the first few seconds of the video
4. Texture - floor texture maps from a marble image; we also texture-mapped a painting
5. Perlin noise - generates the Northern lights

How to build and run:

In project directory, move Eigen library folder into ./previz/include

Make:
cd previz
make && ./previz

Note: to see a subgroup of frames, you can type ./previz start_seq end_seq

This will generate the output frames in ./previz/frames

To generate movie, navigate into the frames folder: ./previz/frames

Run:
./movieMaker
