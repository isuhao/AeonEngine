# Copyright (C) 2017 Rodrigo Jose Hernandez Cordoba
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

bl_info = {
    "name": "AeonGames Model Format (.mdl)",
    "author": "Rodrigo Hernandez",
    "version": (1, 0, 0),
    "blender": (2, 7, 0),
    "location": "File > Export > Export AeonGames Model",
    "description": "Exports an armature to an AeonGames Model (MDL) file",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Import-Export"}

import bpy
from . import export


def mdl_menu_func(self, context):
    self.layout.operator(
        export.MDLExporter.bl_idname,
        text="AeonGames Model (.mdl)")


def register():
    bpy.utils.register_class(export.MDLExporter)
    bpy.types.INFO_MT_file_export.append(mdl_menu_func)


if __name__ == "__main__":
    register()
