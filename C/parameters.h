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
  int branch, atom, atomPart, atomSpan;
  char * longParameter;
  char shortParameter;
  char * source, * value;
} ParameterData;

void param_debug_print_parameter(ParameterData * result);
void param_begin_parameter_parsing(ParameterParserState * state, int count, char * parameter[]);
bool param_parse_next_parameter(ParameterParserState * state, ParameterData * result);
