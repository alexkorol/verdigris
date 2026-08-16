# Renderer seam

The first client draws placeholder shapes directly in its platform shell. A
future renderer will consume simulation snapshots/events and own billboard
projection, depth sorting, contact shadows, and procedural effects. It must not
mutate House, Scion, item, or combat state.

The WIZARD orb plates/shaders and Verdigris Splash atmosphere are planned
presentation inputs for this seam. They are not a reason to place WebGL or DOM
dependencies in the core library.
