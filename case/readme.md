# 3D printed case

![case preview](./case.png)

The case of this project is designed to be 3D printed.

You can find ready-to-print STL files in the [stl/](./stl/) folder.
- If you have a 3D printer with a large bed, you can print the case and its support in one piece using the `case_all_nosplit.stl` file.

- Otherwise, you can print the case split in two part and the support separately using the `case_left_part.stl`, `case_right_part.stl` and `case_feet_and_button_lock` files.

The case is designed to be printed with supports, so make sure to enable it in your slicer. Tolerance has been added to the case to make sure the parts fit together nicely, but you'll require a bit of sanding to make it fit perfectly.

To check if the defaut tolerance is good for your printer, you can print the `test_print.stl` file.

### Customizing the case

If you want to customize the case to adjust the tolerance or support angle for example, you need to use [OpenSCAD](https://openscad.org) and install the BOSL2 library. See [instructions here](https://github.com/revarbat/BOSL2) for how to install it.

Open the `case.scad` file in OpenSCAD and adjust the parameters in the `Customizer` tab, and once you're happy with the result, select `Render` and then `Export as STL...` to export the STL file.
