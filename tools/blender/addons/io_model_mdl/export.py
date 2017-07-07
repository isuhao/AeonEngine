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

import bpy
import os
import struct
import addon_utils
import model_pb2
import google.protobuf.text_format


class MDLExporter(bpy.types.Operator):

    '''Exports an armature to an AeonGames Model (MDL) file'''
    bl_idname = "export_model.mdl"
    bl_label = "Export AeonGames Model"

    directory = bpy.props.StringProperty(subtype='DIR_PATH')

    @classmethod
    def poll(cls, context):
        is_msh_enabled, is_msh_loaded = addon_utils.check('io_mesh_msh')
        return is_msh_loaded

    def execute(self, context):
        if not os.path.exists(self.directory + "/meshes"):
            os.makedirs(self.directory + "/meshes")
        bpy.ops.export_mesh.all_msh(
            'EXEC_DEFAULT',
            directory=self.directory +
            "/meshes")
        model_buffer = model_pb2.ModelBuffer()
        print(
            "Writting",
            self.directory +
            "/" +
            context.scene.name +
            ".mdl",
            ".")
        out = open(self.directory + "/" + context.scene.name + ".mdl", "wb")
        magick_struct = struct.Struct('8s')
        out.write(magick_struct.pack(b'AEONMDL\x00'))
        out.write(model_buffer.SerializeToString())
        out.close()
        print("Done.")
        print(
            "Writting",
            self.directory +
            "/" +
            context.scene.name +
            ".anm.txt",
            ".")
        out = open(
            self.directory +
            "/" +
            context.scene.name +
            ".mdl.txt",
            "wt")
        out.write("AEONMDL\n")
        out.write(
            google.protobuf.text_format.MessageToString(model_buffer))
        out.close()
        return {'FINISHED'}

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}
