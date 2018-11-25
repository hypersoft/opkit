/*

Copyright (C) Triston-Jerard: Taylor; November: 25th; 2018

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

#include "parameters.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static const char * DASH_DASH = "--";
static const char * DASH = "-";
static const char * PLUS = "+";

static bool string_starts_with (char * a, const char * b)
{
  int val = memcmp(a, b, strlen(b));
  return val == 0;
}

static char * string_get_first_character_match (char * a, char b)
{
  return strchr(a, b);
}

const char * param_get_parameter_type_string (ParameterType type)
{
  switch (type) {
    case PRMTR_TYPE_VOID: return "void";
    case PRMTR_TYPE_LONG: return "long";
    case PRMTR_TYPE_SHORT_DASH: return "short-dash";
    case PRMTR_TYPE_SHORT_PLUS: return "short-plus";
  }
}

void param_debug_print_parameter (ParameterData * result)
{
  if (result == NULL) {
    fprintf(stderr, "critical: parameter-debug-fault: no parameter data was provided by the caller\n");
    abort();
  }

  printf("type: %s\n", param_get_parameter_type_string(result->type));
  if (result->branch) printf("branch: %i\n", result->branch);
  printf("atom: %i\n", result->atom);
  if (result->atomPart)
    printf("atom-part: %i\n", result->atomPart);
  if (result->atomSpan) printf("atom-span: %i\n", result->atomSpan);
  printf("source: %s\n", result->source);
  if (result->type == PRMTR_TYPE_LONG) {
    printf("long-parameter: %s\n", result->longParameter);
  } else if (result->type != PRMTR_TYPE_VOID) printf("short-parameter: %c\n", result->shortParameter);
  if (result->value)
    printf("value: %s\n", result->value);

}

void param_begin_parameter_parsing (ParameterParserState * state, int count, char * parameter[])
{
  memset(state, 0, sizeof (ParameterParserState));
  state->atomCount = count;
  state->atomTable = parameter;
  state->atomSelector = 1; // because we don't parse parameter[0] == PROGRAM_PATH
}

bool param_parse_next_parameter (ParameterParserState * state, ParameterData * result)
{

  if (state == NULL) {
    fprintf(stderr, "critical: parameter-parsing-fault: no parser state was provided by the caller\n");
    abort();
  }

  state->faultCode = 0;
  memset(result, 0, sizeof (ParameterData));
  result->atom = state->atomSelector;

  if (result == NULL) {
    state->faultCode = PRMTR_NO_PARAMETER_DATA;
    return false;
  }

  if (state->atomSelector == state->atomCount) {
    state->faultCode = PRMTR_NO_PARAMETERS_AVAILABLE;
    return false;
  }

  result->source = state->atomTable[state->atomSelector];

  size_t length = strlen(result->source);
  size_t finalSubPoint = length - 1;

  char * equals = string_get_first_character_match(result->source, '=');
  char * fullColon = (equals) ? NULL : string_get_first_character_match(result->source, ':');

  bool longDash = string_starts_with(result->source, DASH_DASH);
  bool shortDash = (longDash) ? false : result->source[0] == '-';
  bool shortPlus = result->source[0] == '+';

  /* branch 1: test for long option */
  if (longDash && ! fullColon && ! equals && length > 2) {
    result->type = PRMTR_TYPE_LONG;
    result->branch = 1;
    result->atomSpan = 1;
    result->longParameter = strndup(result->source + 2, length - 2);
    state->atomSelector++;
    return true;
  }

  /* branch 2: test for long option with setting */
  if (longDash && fullColon && length > 3) {
    result->type = PRMTR_TYPE_LONG;
    result->branch = 2;
    result->atomSpan = 2;
    result->longParameter = strndup(result->source + 2, length - 3);
    state->atomSelector++;
    if (state->atomSelector == state->atomCount) {
      state->faultCode = PRMTR_NO_PARAMETER_VALUE_AVAILABLE;
      return false;
    }
    result->value = state->atomTable[state->atomSelector];
    state->atomSelector++;
    return true;
  }

  /* branch 3: test for long option with data */
  if (longDash && equals && equals - result->source > 2) {
    result->type = PRMTR_TYPE_LONG;
    result->branch = 3;
    result->atomSpan = 1;
    result->longParameter = strndup(result->source + 2, equals - (result->source + 2));
    result->value = ++equals;
    state->atomSelector++;
    return true;
  }

  /* branch 4: test for short option */
  if ((shortDash || shortPlus) && !fullColon && !equals && length == 2) {
    result->type = (shortDash) ? PRMTR_TYPE_SHORT_DASH : PRMTR_TYPE_SHORT_PLUS;
    result->branch = 4;
    result->atomSpan = 1;
    // if parameter is '-' then zero will fall through as character
    result->shortParameter = result->source[1];
    state->atomSelector++;
    return true;
  }

  /* branch 5: test for short option with setting */
  if ((shortDash || shortPlus) && fullColon && length == 3) {
    result->type = (shortDash) ? PRMTR_TYPE_SHORT_DASH : PRMTR_TYPE_SHORT_PLUS;
    result->branch = 5;
    result->atomSpan = 2;
    result->shortParameter = result->source[1];
    state->atomSelector++;
    if (state->atomSelector == state->atomCount) {
      state->faultCode = PRMTR_NO_PARAMETER_VALUE_AVAILABLE;
      return false;
    }
    result->value = state->atomTable[state->atomSelector];
    state->atomSelector++;
    return true;
  }

  /* branch 6: test for short option with data */
  if ((shortDash || shortPlus) && equals == result->source + 2) {
    result->type = (shortDash) ? PRMTR_TYPE_SHORT_DASH : PRMTR_TYPE_SHORT_PLUS;
    result->branch = 6;
    result->atomSpan = 1;
    result->shortParameter = result->source[1];
    result->value = ++equals;
    state->atomSelector++;
    return true;
  }

  /* branch 7: test for multiple short options */
  if ((shortDash || shortPlus) && ! equals && ! fullColon && length > 2) {
    result->type = (shortDash) ? PRMTR_TYPE_SHORT_DASH : PRMTR_TYPE_SHORT_PLUS;
    result->branch = 7;
    result->atomPart = ++state->subatomSelector;
    result->shortParameter = result->source[result->atomPart];
    if (result->atomPart == finalSubPoint) {
      result->atomSpan = 1;
      state->atomSelector++;
      state->subatomSelector = 0;
      return true;
    } else {
      return true;
    }
  }

  /* branch 8: test for multiple short options with setting */
  if ((shortDash || shortPlus) && length > 3 && result->source + finalSubPoint == fullColon) {
    --finalSubPoint;
    result->type = (shortDash) ? PRMTR_TYPE_SHORT_DASH : PRMTR_TYPE_SHORT_PLUS;
    result->branch = 8;
    result->atomPart = ++state->subatomSelector;
    result->shortParameter = result->source[result->atomPart];
    if (result->atomPart == finalSubPoint) {
      state->atomSelector++;
      if (state->atomSelector == state->atomCount) {
        result->atomSpan = 1;
        state->faultCode = PRMTR_NO_PARAMETER_VALUE_AVAILABLE;
        return false;
      }
      result->atomSpan = 2;
      result->value = state->atomTable[state->atomSelector];
      state->atomSelector++;
      state->subatomSelector = 0;
      return true;
    } else {
      return true;
    }
  }

  /* branch 9: test for multiple short options with data */
  if ((shortDash || shortPlus) && equals && length > 3) {
    finalSubPoint = (size_t) (equals - result->source) - 1;
    result->type = (shortDash) ? PRMTR_TYPE_SHORT_DASH : PRMTR_TYPE_SHORT_PLUS;
    result->branch = 9;
    result->atomPart = ++state->subatomSelector;
    result->shortParameter = result->source[result->atomPart];
    if (result->atomPart == finalSubPoint) {
      result->atomSpan = 1;
      state->atomSelector++;
      result->value = equals + 1;
      state->subatomSelector = 0;
      return true;
    } else {
      return true;
    }
  }

  result->atom = state->atomSelector++;
  state->faultCode = PRMTR_NO_PARAMETER_MATCH;
  return false;

}
