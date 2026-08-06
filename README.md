# Doom-style game
this game is made in scratch without an engine, rather it is created with a custom engine in mind
to make rendering more optimized and specialized for this kind of visuals.

This readme will provide a very small information on the engine and game itself.
Also I am not a god developer I am just some mediocre developer who tries their best 
to learn more about game engines... still I am very proud of this project.

<br>

## Engine Capabilities
The engines capability or functions to make the game run.
- Shaders
- 3D Rendering
- Scenes / Map
- Simple AABB Physics system
- Resource hot-reloading
- Raycasting (Kinda bad as of the moment)
- Entities
- Audio

<br>

## Map geometry
the map is divided into `sectors` just like in the old doom game, in where sectors are surrounded by `wall` or `linedef` as 
it's called in doom, then the walls that has two neighboring sectors are called `portal` or in the actual doom terminology is called 
a `two-lined linedef`.

Each map generates each sector's wall normals on load, and recalculates the sectors wall normals if any of the sectors geometry properties are modified
such as floor height and ceil height.

<br>

## Rendering
- Rendering a map primarily uses simple geometries like quads and planes with map partitioning to cull draw calls. Each sector draws.
  it's walls then floor and ceiling before proceeding to the next sector.
- entities are rendered as a billboards (quads that rotate yaw-wise to look directly in the camera).
  Entities are drawn last after all drawable sectors are done.

<br>

### Old Rendering preview
this preview showcases how the map is rendered in order, although the footage is an outdated view
it is still somewhat similar.

https://github.com/user-attachments/assets/011f149e-93de-4348-b146-a97ea59c5bea

## Build
if you want a copy of this project:<br>
use `git clone https://github.com/SarryGeezOwO/DoomStyleGame.git` on the folder you want to clone this repo.

The build system used for this is `makefile` with c++17 as the target cpp version.
more precisely I use GNU MinGW for this project.

if you want to try and build this project for yourself.
run this command:
- `make help` will list down all the available targets.
- `make debug` or `make` will create a debug build of the project.
- `make release` will make a release build (removes all logging and other debug codes).

<div align="center">
  <img width="400" height="400" alt="Hikari" src="https://github.com/user-attachments/assets/04434eb7-581d-43f1-95fb-3285204904dd" />
</div>
<p align="center">Hope you err... like this goofy project of mine!! </p>
