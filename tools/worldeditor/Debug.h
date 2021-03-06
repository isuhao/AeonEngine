/*
Copyright (C) 2017,2018 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_DEBUG_H
#define AEONGAMES_DEBUG_H

#include "aeongames/Platform.h"

#ifdef _MSC_VER
#include <vector>
#include <tlhelp32.h>
#include <Dbghelp.h>
void InitializeModuleTable();
void FinalizeModuleTable();
LPCSTR WINAPI GetProcName ( _In_ HMODULE hModule, _In_ FARPROC lpAddress );
BOOL WINAPI GetModuleEntryByAddr ( _In_ FARPROC lpAddress, _Out_ PMODULEENTRY32 lpModuleEntry );
FARPROC WINAPI GetCallStackReturnAddress ( _In_ size_t index );
void WINAPI PrintCallStack ( _In_ FILE* file );
#endif
#endif