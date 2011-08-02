/*
Copyright (C) 2000-2011 by Toby Jones.

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

#ifndef STRINGUTILS_H
#define STRINGUTILS_H

void pretty_print32(uint32_t value, _Out_z_cap_(size_in_chars) PTSTR output_string, _In_range_(0, INT_MAX) size_t size_in_chars);
void pretty_print64(uint64_t value, _Out_z_cap_(size_in_chars) PTSTR output_string, _In_range_(0, INT_MAX) size_t size_in_chars);

#endif

