Instalation
===========

In order to use the Blender addon exporters for AeonEngine, you need to install the python protobuffer library on your Blender setup because it is a pain to have Blender use the local python instalation and either way you need to build and install the PB library somewhere.
Specifically, you will need to install pip and then build and install the PB library from source.

I may expand some more on this, but right now this is mostly something I can look up to set up an environment since the process is quite involved and I couldn't or haven't been able to easily automate.

1- Run as administrator or root a command line window or terminal.
2- Add the directory containing the python executable from Blender into the PATH enviroment variable.
3- Install pip using get-pip.py (https://pip.pypa.io/en/stable/installing/)
4- After building and/or installing protocol buffer, add the path to protoc to the PATH enviroment variable.
5- From the python folder under the protobuffer source run "python setup.py build" then "python setup.py test" and "python setup.py install"

Thats it. To actually run the exporters:

1- Make or build the generate-python-protobuf-source target of this project.
2- Run Blender and go to File->User Preferences and click on the "Files" Tab.
3- Type or browse for this folder on the "Scripts" field.
4- Save preferences and restart Blender.
5- I will be making changes so the exporters show up as actual addons but for now...
6- Open the text editor window and open the exporter you need under the addons folder.
7- Click on "Run Script" no error should pop up and nothing really should happen.
8- Go to File->Export or File->Import and find the type you want to export to.