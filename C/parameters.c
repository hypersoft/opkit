#include "parameters.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*
 * a legal parameter name may not contain this RE class [:=]; because those 
 * bytes are for specifying data joined indirectly or directly (respectively) to
 * the parameter name given.
 * 
 * legal examples (with data):
 * 
 *  --file: FILE_NAME
 *  --file=FILE_NAME
 *  -f: FILE_NAME
 *  +f: FILE_NAME
 *  -f=FILE_NAME
 *  +f=FILE_NAME
 *	-xyzf: FILE_NAME
 *	+xyzf: FILE_NAME
 *  -xyzf=FILE_NAME
 *  +xyzf=FILE_NAME
 *
 * legal examples (no data/manual-data)
 * 
 *	--word-*
 *  [the asterisk in the following examples represents any character]
 *	-*...
 *  +*...
 * 
 * if a the data can't be parsed, it will be returned to you in the source field
 * of the ParameterData, and the atomSelector of the ParserState will be updated
 * anyway. So, if you have options which take data, you can parse the data yourself,
 * if the RE class [:=] is not specified, by simply parsing the next field.
 * in any case, every parameter will be parsed, but not every parameter will
 * successfully parsed by the parser branch codes.
 * 
 * NOTE: if the parameter type is long, you MUST free the longParameter string,
 * IF you are finished using it, otherwise, you can keep the data and never free
 * it, but any memory leak detection you may be using will report a false
 * positive on exit. You should make a habit of freeing the data when it is no
 * longer needed.
 * 
 * NOTE: this code does not perform any validation of what is parsed. It simply
 * tokenizes command line parameters according to this
 * specification[ you just read]. If you need further tokenization or validation,
 * you will have to provide the methods, and this code will provide you with all 
 * of the parameter-data.
*/

// example usage
/*
#include <stdio.h>
#include <stdlib.h>

#include "parameters.h"

int main(int argc, char * argv[]) {
	ParameterParserState state;
	param_begin_parameter_parsing(&state, argc, argv);
	ParameterData parameter;
	param_parse_next_parameter(&state, &parameter);
	param_debug_print_parameter(&parameter);
	if (parameter.atomPart) {
		while (param_parse_next_parameter(&state, &parameter)) {
			param_debug_print_parameter(&parameter);
			puts("");
		}
	}
  exit(0);
}
*/

static bool string_starts_with(char * a, const char * b)
{
  int val = memcmp(a, b, strlen(b));
  return val == 0;
}

static char * string_get_first_character_match(char * a, char b)
{
  return strchr(a, b);
}

static const char * get_parameter_type_string(ParameterType type)
{
	switch(type) {
		case PRMTR_TYPE_VOID: return "void";
		case PRMTR_TYPE_LONG: return "long";
		case PRMTR_TYPE_SHORT_DASH: return "short-dash";
		case PRMTR_TYPE_SHORT_PLUS: return "short-plus";
	}
}

void param_debug_print_parameter(ParameterData * result)
{
	if (result == NULL) {
		fprintf(stderr, "critical: parameter-debug-fault: no parameter data was provided by the caller\n");
		abort();
	}
	
	printf("type: %s\n", get_parameter_type_string(result->type));
	if (result->branch) printf("branch: %i\n", result->branch);
	printf("atom: %i\n", result->atom);
	if(result->atomPart) 
		printf("atom-part: %i\n", result->atomPart);
	if(result->atomSpan) printf("atom-span: %i\n", result->atomSpan);
	printf("source: %s\n", result->source);
	if (result->type == PRMTR_TYPE_LONG) {
		printf("long-parameter: %s\n", result->longParameter);
	} else if (result->type != PRMTR_TYPE_VOID) printf("short-parameter: %c\n", result->shortParameter);
	if (result->value) 
		printf("value: %s\n", result->value);

}

void param_begin_parameter_parsing(ParameterParserState * state, int count, char * parameter[])
{
  memset(state, 0, sizeof(ParameterParserState));
  state->atomCount = count;
  state->atomTable = parameter;
  state->atomSelector = 1; // because we don't parse parameter[0] == PROGRAM_PATH
}

static const char * DASH_DASH = "--";
static const char * DASH = "-";
static const char * PLUS = "+";

bool param_parse_next_parameter(ParameterParserState * state, ParameterData * result)
{ 
	
  if (state == NULL) {
    fprintf(stderr, "critical: parameter-parsing-fault: no parser state was provided by the caller\n");
    abort();
  }
  
	state->faultCode = 0;

  if (result == NULL) {
    state->faultCode = PRMTR_NO_PARAMETER_DATA;
    return false;
  }
  
  if (state->atomSelector == state->atomCount) {
		state->faultCode = PRMTR_NO_PARAMETERS_AVAILABLE;
		return false;
	}
	
	memset(result, 0, sizeof(ParameterData));
	
  result->source = state->atomTable[state->atomSelector];

	size_t length = strlen(result->source);
	size_t finalSubPoint = length - 1;
	
	char * equals = string_get_first_character_match(result->source, '=');
	char * fullColon = (equals)?NULL:string_get_first_character_match(result->source, ':');

	bool longDash = string_starts_with(result->source, DASH_DASH);
	bool shortDash = (longDash)?false:result->source[0] == '-';
	bool shortPlus = result->source[0] == '+';

  /* branch 1: test for long option */
  if (longDash && ! fullColon && ! equals && length > 2) {
    result->type = PRMTR_TYPE_LONG;
    result->branch = 1;
		result->atom = state->atomSelector;
    result->atomSpan = 1;
    result->longParameter = strndup(result->source + 2, length - 2);
    state->atomSelector++;
    return true;
  }
  
  /* branch 2: test for long option with setting */
  if (longDash && fullColon && length > 3) {
    result->type = PRMTR_TYPE_LONG;
    result->branch = 2;
		result->atom = state->atomSelector;
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
		result->atom = state->atomSelector;
    result->atomSpan = 1;
    result->longParameter = strndup(result->source + 2, equals - (result->source + 2));
    result->value = ++equals;
    state->atomSelector++;
		return true;
  }
	
	/* branch 4: test for short option */
	if ((shortDash || shortPlus) && !fullColon && !equals && length == 2) {
    result->type = (shortDash)?PRMTR_TYPE_SHORT_DASH:PRMTR_TYPE_SHORT_PLUS;
    result->branch = 4;
		result->atom = state->atomSelector;
    result->atomSpan = 1;
		// if parameter is '-' then zero will fall through as character
		result->shortParameter = result->source[1];
		state->atomSelector++;
		return true;
	}

	/* branch 5: test for short option with setting */
	if ((shortDash || shortPlus) && fullColon && length == 3) {
    result->type = (shortDash)?PRMTR_TYPE_SHORT_DASH:PRMTR_TYPE_SHORT_PLUS;
    result->branch = 5;
		result->atom = state->atomSelector;
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
    result->type = (shortDash)?PRMTR_TYPE_SHORT_DASH:PRMTR_TYPE_SHORT_PLUS;
    result->branch = 6;
		result->atom = state->atomSelector;
    result->atomSpan = 1;
    result->shortParameter = result->source[1];
    result->value = ++equals;
    state->atomSelector++;
		return true;
	}
	
	/* branch 7: test for multiple short options */
	if ((shortDash || shortPlus) && ! equals && ! fullColon && length > 2) {
    result->type = (shortDash)?PRMTR_TYPE_SHORT_DASH:PRMTR_TYPE_SHORT_PLUS;
    result->branch = 7;
		result->atom = state->atomSelector;
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
    result->type = (shortDash)?PRMTR_TYPE_SHORT_DASH:PRMTR_TYPE_SHORT_PLUS;
    result->branch = 8;
		result->atom = state->atomSelector;
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
		finalSubPoint = (size_t)(equals - result->source) - 1;
    result->type = (shortDash)?PRMTR_TYPE_SHORT_DASH:PRMTR_TYPE_SHORT_PLUS;
    result->branch = 9;
		result->atom = state->atomSelector;
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
