/*

Copyright (C) 2018, Triston-Jerard: Taylor

All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 

*/

#include <stdbool.h>

typedef enum {
  PRMTR_TYPE_VOID,
  PRMTR_TYPE_LONG, 
  PRMTR_TYPE_SHORT_DASH, 
  PRMTR_TYPE_SHORT_PLUS,
  PRMTR_TYPE_BREAK
} ParameterType;

typedef enum {
  PRMTR_NO_PARAMETER_DATA,
  PRMTR_NO_PARAMETERS_AVAILABLE,
  PRMTR_NO_PARAMETER_VALUE_AVAILABLE,
  PRMTR_NO_PARAMETER_MATCH
} ParameterParserFaultCode;

typedef struct {
  char ** atomTable;
  int atomCount;
  int atomSelector, subatomSelector;
  ParameterParserFaultCode faultCode;
} ParameterParserState;

typedef struct {
  ParameterType type;
  int branch, atom, subatom, atomSpan;
  char * longParameter;
  char shortParameter;
  char * source, * value;
} ParameterData;

/**
 * Prints Information on standard error about parameter data.
*/
void param_debug_print_parameter(ParameterData * result);
const char * param_get_parameter_type_string (ParameterType type);
void param_begin_parameter_parsing(ParameterParserState * state, int count, char * parameter[]);
bool param_parse_next_parameter(ParameterParserState * state, ParameterData * result);
