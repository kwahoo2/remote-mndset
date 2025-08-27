# remote-mndset

This is an alternative GUI for [Monado's](https://monado.dev/) remote driver. 

It has been created for testing VR applications without wearing a HMD or using motion controllers.

Both keyboard and mouse or gamepad are supported.

![remote-mndset][rmds]

[rmds]: https://github.com/kwahoo2/remote-mndset/blob/main/images/main_window.png "Main Window"

## How to use

* Install Monado [from git](https://gitlab.freedesktop.org/monado/monado/-/tree/main)
* Install SDL3 development packages
* Build _remote_mndset_:

```
git clone https://github.com/kwahoo2/remote-mndset.git
cd remote-mndset
mkdir build && cd build
cmake ..
make
```

* Run Monado with the remote driver enabled:

`P_OVERRIDE_ACTIVE_CONFIG=remote monado-service`

* Start _remote-mndset_ and your XR application, then click "Connect" in _remote-mndset_

## Bindings

Note: a keyboard + mouse and a controller cannot be used simultaneously (the controller input is disabled when kb + m is grabbed).

### Keyboard and mouse

| Input             | No modifier         | LShift                      | LAlt                         |
| ------------------| --------------------|-----------------------------|------------------------------|
| W/S               | HMD Forward/Backward| Left Contr. Forward/Backward| Right Contr. Forward/Backward|
| A/D               | HMD Left/Right      | Left Contr. Left/Right      | Right Contr. Left/Right      |
| Ctrl/Space        | HMD Up/Down         | Left Contr. Up/Down         | Right Contr. Up/Down         |
| Q/E               | HMD Roll            | Left Contr. Roll            | Right Contr. Roll            |
| Mouse X axis      | HMD Yaw             | Left Contr. Yaw             | Right Contr. Yaw             |
| Mouse Y axis      | HMD Pitch           | Left Contr. Pitch           | Right Contr. Pitch           |
| Mouse left button | Left Contr. Trigger | Left Contr. Trigger         | Left Contr. Trigger          |
| Mouse right button| Right Contr. Trigger| Right Contr. Trigger        | Right Contr. Trigger         |
| Mouse side btn 1  |                     | Left Contr. A               | Right Contr. A               |
| Mouse side btn 2  |                     | Left Contr. B               | Right Contr. B               |

### Gamepad (tested with PS4 controller)

| Input             | No modifier         | Left Shoulder               | Right Shoulder               |
| ------------------| --------------------|-----------------------------|------------------------------|
| Left stick Y axis | HMD Forward/Backward| Left Contr. Forward/Backward| Right Contr. Forward/Backward|
| Left stick X axis | HMD Left/Right      | Left Contr. Left/Right      | Right Contr. Left/Right      |
| D-pad up/down     | HMD Up/Down         | Left Contr. Up/Down         | Right Contr. Up/Down         |
| D-pad left/right  | HMD Roll            | Left Contr. Roll            | Right Contr. Roll            |
| Right stick X axis| HMD Yaw             | Left Contr. Yaw             | Right Contr. Yaw             |
| Right stick Y axis| HMD Pitch           | Left Contr. Pitch           | Right Contr. Pitch           |
| Left trigger      | Left Contr. Trigger | Left Contr. Trigger         | Left Contr. Trigger          |
| Right trigger     | Right Contr. Trigger| Right Contr. Trigger        | Right Contr. Trigger         |
| South (triangle/A)|                     | Left Contr. A               | Right Contr. A               |
| East  (circle/B)  |                     | Left Contr. B               | Right Contr. B               |


## License

Check [LICENSE](LICENSE) for details.

[remote_mndset](https://github.com/kwahoo2/remote-mndset) repository containts parts of [OpenGL Mathematics (GLM)](https://github.com/g-truc/glm) library and [Dear ImGui](https://github.com/ocornut/imgui) library.

