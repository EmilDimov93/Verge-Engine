# ⚠️ Warning: file is not complete!

# For readers

#### **VIS** stands for Vehicle Input State and it is a struct that contains the input information from the user for throttle, brake and more.

#### An **Orphan** is a Renderer object that has lost its upkeeper from the Scene in the current frame and is therefore scheduled for removal

## Scene

#### A **Handle** is a per-type ID that can be used in the Scene class. Example: scene.vehicle(vehicle1Handle).someVehicleMethod()

## Input

#### The term **controller** encompasses all devices that may be connected and used to control a Vehicle. In the case that it is a gamepad, the hardcoded keybinds may be used(VE::CONTROLLER_*).

#### Axes may be used in 4 ways: full range, full range inverted, positive half and negative half. The reason is because GLFW returns -1 to 1 for all axes, which is good for a steering wheel or a joystick, but bad for a pedal or a trigger.

# For writers

#### **Mat** shall be short for matrix and shall be used as its substitute in all cases.

#### Include order shall be:
1. Associated .hpp file
2. Internal .hpp files
3. External libraries

#### In classes, the **public** section shall go first and the **private** second.

#### In physics calculating functions **Mag** shall refer to Magnitude

#### When referring to vehicles, **front** shall be used for the forward-facing side and **back** shall be used for the rearward-facing side. The term *rear* must not be used due to abbreviation collisions with *right*.

# For users

#### **.wav** is highly advised as audio format