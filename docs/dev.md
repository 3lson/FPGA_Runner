# This will contain the code step by step of my implementation

## Phase 1: Basic Game Setup in Unity

1) Create a New Unity Project:
- Unity Hub -> New Project -> 3D Universal
- Name the project and click Create

Note Unity might take some time to open here

2) Set up the Game Environment
- In the hierarchy:
  - Right Click -> 3D Object -> Plane and rename to Ground and scale to (10, 1, 10) in the inspector
- Under Window -> Rendering -> Ligthing -> Environment -> there should already be a Default Skybox

3) Create the Player Character
- In the hierarchy -> Right Click -> 3D object -> capsule -> rename to player -> set position to (0, 1, 0)
- Attach a rigidbody component:
  - Click Player -> Inspector Panel -> Add Component -> Rigidbody
  - Check the Freeze Rotation X and Z (so it does not fall over)
 
4) Make the Character Move Left/Right

From the starter_code in assets we can create this.

## Phase 2: Integration test for FPGA Control via TCP connection to control the player



## Phase 3: Full Endless Runner Game Developement (1st Draft)

## Phase 4: Replace Keyboard Controls on Endless Runner to FPGA controls
