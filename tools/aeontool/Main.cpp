/*
Copyright (C) 2016,2018 Rodrigo Jose Hernandez Cordoba

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include <iostream>
#include <unordered_map>
#include <functional>
#include "aeongames/Memory.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include <google/protobuf/text_format.h>
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include <cassert>
#include <cstdint>
#include "Convert.h"
#include "PipelineTool.h"

int main ( int argc, char *argv[] )
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    std::unordered_map<std::string, std::function<std::unique_ptr<AeonGames::Tool>() > > ToolFactories
    {
        { "convert", [] { return std::make_unique<AeonGames::Convert>(); } },
        { "pipeline", [] { return std::make_unique<AeonGames::PipelineTool>(); } },
    };
#ifdef _MSC_VER
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#if 0
    // Send all reports to STDOUT
    _CrtSetReportMode ( _CRT_WARN, _CRTDBG_MODE_FILE );
    _CrtSetReportFile ( _CRT_WARN, _CRTDBG_FILE_STDOUT );
    _CrtSetReportMode ( _CRT_ERROR, _CRTDBG_MODE_FILE );
    _CrtSetReportFile ( _CRT_ERROR, _CRTDBG_FILE_STDOUT );
    _CrtSetReportMode ( _CRT_ASSERT, _CRTDBG_MODE_FILE );
    _CrtSetReportFile ( _CRT_ASSERT, _CRTDBG_FILE_STDOUT );
#endif
    // Use _CrtSetBreakAlloc( ) to set breakpoints on allocations.
#endif
    try
    {
        int retval = 0;
        if ( argc >= 2 || ToolFactories.find ( argv[1] ) != ToolFactories.end() )
        {
            ToolFactories[argv[1]]()->operator() ( argc, argv );
        }
        else
        {
            std::cout << "Usage: " << argv[0] << " <tool> [-help | ...]" << std::endl;
        }
        google::protobuf::ShutdownProtobufLibrary();
        return retval;
    }
    catch ( std::runtime_error &e )
    {
        std::cout << e.what() << std::endl;
    }
    catch ( ... )
    {
        std::cout << "Error: Unknown Exception caught." << std::endl;
    }
    google::protobuf::ShutdownProtobufLibrary();
    return -1;
}
