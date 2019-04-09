package opkit;

import java.io.PrintStream;

/**
 * Parameter Parsing Suite
 *
 * Supports many formats:
 *
 *   Branch  Description
 *      (1)  Unix Long Option: --switch
 *      (2)  Unix Long Option with inline data: --switch:value or --switch=value
 *      (3)  Unix Long Option with data: --switch: value
 *      (4)  Unix Short Option: -s or +s
 *      (5)  Unix Short Option with inline data: -s:value, +s:value, -s=value or +s=value
 *      (6)  Unix Short option with data: -s: value or +s: value
 *      (7)  Multiple Unix Short Options: -abc or +abc
 *      (8)  Multiple Unix Short Options with inline data: -abc:value, +abc:value, -abc=value or +abc=value
 *      (9)  Multiple Unix Short options with data: -abc: value or +abc: value
 *     (10)  JavaScript Property Style: switch: value
 *
 *  [NOTE: values apply only to the last short option in a multiple-short-option-list]
 *  [NOTE: windows forward slash is not supported because of unix-root-file-compatibility]
 *  [NOTE: single-dash-long-options are not supported because word-lists are not implemented]
 */

public class Parameters {

    public enum ParameterType {
        PRMTR_TYPE_VOID,
        PRMTR_TYPE_LONG,
        PRMTR_TYPE_SHORT_DASH,
        PRMTR_TYPE_SHORT_PLUS,
        PRMTR_TYPE_PROPERTY
    }

    public enum ParameterParserFaultCode {
        PRMTR_NO_FAULT,
        PRMTR_NO_PARAMETER_DATA,
        PRMTR_NO_PARAMETERS_AVAILABLE,
        PRMTR_NO_PARAMETER_VALUE_AVAILABLE,
        PRMTR_NO_PARAMETER_MATCH
    }

    public static class ParameterParserState {
        public String[] atomTable;
        public int atomCount, atomSelector, subatomSelector;
        public ParameterParserFaultCode faultCode;
        private void clear() {
            atomTable = null;
            atomCount = atomSelector = -1;
            subatomSelector = 0;
            faultCode = ParameterParserFaultCode.PRMTR_NO_FAULT;
        }

        public void printTrace(PrintStream pw) {
            pw.println("\nParser State:\n");
            pw.print("\tatomCount: "+atomCount);
            pw.print(", atomSelector: " + atomSelector);
            pw.println(", subatomSelector: "+ subatomSelector);
            pw.println("\tfaultCode: "+ faultCode.toString());
        }
    }

    public static class ParameterData {
        public ParameterType type;
        public int branch, atom, subatom, atomSpan;
        public String longParameter;
        public char shortParameter;
        public String source;
        Object value;
        private void clear() {
            type = ParameterType.PRMTR_TYPE_VOID;
            branch = subatom = atomSpan = 0;
            atom = -1;
            longParameter = source = null;
            value = null;
            shortParameter = 0;
        }
        public void printTrace(PrintStream pw) {
            pw.println("\nParameter Data:\n");
            pw.print("\ttype: " + type.toString());
            pw.print(", branch: " + branch);
            pw.print(", atom: "+atom);
            pw.print(", atomSpan: "+atomSpan);
            pw.println(", subatom: "+subatom);
            if (type == ParameterType.PRMTR_TYPE_LONG) pw.print("\tlongParameter: "+longParameter);
            else pw.print("\tshortParameter: "+shortParameter);
            pw.println(", source: "+source);
            pw.println("\tvalue: "+value);
        }

    }

    private int getFirstCharacterMatch(String search, char find) {
        for (int i = 0; i < search.length(); i++) {
            if (search.charAt(i) == find) return i;
        }
        return -1;
    }

    public void beginParameterParsing(ParameterParserState state, int start, String[] parameters) {
        state.clear();
        state.atomCount = parameters.length;
        state.atomTable = parameters;
        state.atomSelector = start;
    }

    public boolean parseNextParameter(ParameterParserState state, ParameterData result) {

        if (state == null) throw new IllegalArgumentException("state parameter is null");
        state.faultCode = ParameterParserFaultCode.PRMTR_NO_FAULT;

        if (result == null) {
            state.faultCode = ParameterParserFaultCode.PRMTR_NO_PARAMETERS_AVAILABLE;
            throw new IllegalArgumentException("result parameter is null");
        }

        result.clear();

        if (state.atomSelector == state.atomCount) {
            state.faultCode = ParameterParserFaultCode.PRMTR_NO_PARAMETER_VALUE_AVAILABLE;
            return false;
        }

        result.atom = state.atomSelector;
        result.source = state.atomTable[state.atomSelector];

        int length = result.source.length();
        int finalSubPoint = length - 1;

        int equals = getFirstCharacterMatch(result.source, '=');
        int fullColon = getFirstCharacterMatch(result.source, ':');

        boolean longDash = result.source.startsWith("--");
        boolean shortDash = (longDash)? false: result.source.charAt(0) == '-';
        boolean shortPlus = result.source.charAt(0) == '+';

        /* branch 1: test for long option */
        if (longDash && fullColon < 0 && equals < 0 && length > 2) {
            result.type = ParameterType.PRMTR_TYPE_LONG;
            result.branch = 1;
            result.atomSpan = 1;
            result.longParameter = result.source.substring(2);
            state.atomSelector++;
            return true;
        }

        /* branch 2: test for long option with setting */
        if (longDash && fullColon > 2 && length > 3) {
            result.type = ParameterType.PRMTR_TYPE_LONG;
            result.branch = 2;
            result.longParameter = result.source.substring(2, fullColon);
            if (length > fullColon) {
                result.atomSpan = 1;
                result.value = result.source.substring(fullColon + 1);
            } else {
                result.atomSpan = 2;
                state.atomSelector++;
                if (state.atomSelector == state.atomCount) {
                    state.faultCode = ParameterParserFaultCode.PRMTR_NO_PARAMETER_VALUE_AVAILABLE;
                    return false;
                }
                result.value = state.atomTable[state.atomSelector];
            }
            state.atomSelector++;
            return true;
        }

        /* branch 3: test for long option with data */
        if (longDash && equals > 2) {
            result.type = ParameterType.PRMTR_TYPE_LONG;
            result.branch = 3;
            result.atomSpan = 1;
            result.longParameter = result.source.substring(2, equals);
            result.value = result.source.substring(equals + 1);
            state.atomSelector++;
            return true;
        }

        /* branch 4: test for short option */
        if ((shortDash || shortPlus) && fullColon == -1 && equals == -1 && length == 2) {
            result.type = (shortDash) ? ParameterType.PRMTR_TYPE_SHORT_DASH : ParameterType.PRMTR_TYPE_SHORT_PLUS;
            result.branch = 4;
            result.atomSpan = 1;
            // if parameter is '-' then zero will fall through as character
            result.shortParameter = result.source.charAt(1);
            state.atomSelector++;
            return true;
        }

        /* branch 5: test for short option with setting */
        if ((shortDash || shortPlus) && fullColon == 2) {
            result.type = (shortDash) ? ParameterType.PRMTR_TYPE_SHORT_DASH : ParameterType.PRMTR_TYPE_SHORT_PLUS;
            result.branch = 5;
            result.shortParameter = result.source.charAt(1);
            if (length > fullColon) {
                result.atomSpan = 1;
                result.value = result.source.substring(fullColon+1);
            } else {
                result.atomSpan = 2;
                state.atomSelector++;
                if (state.atomSelector == state.atomCount) {
                    state.faultCode = ParameterParserFaultCode.PRMTR_NO_PARAMETER_VALUE_AVAILABLE;
                    return false;
                }
                result.value = state.atomTable[state.atomSelector];
            }
            state.atomSelector++;
            return true;
        }

        /* branch 6: test for short option with data */
        if ((shortDash || shortPlus) && equals == 2) {
            result.type = (shortDash) ? ParameterType.PRMTR_TYPE_SHORT_DASH : ParameterType.PRMTR_TYPE_SHORT_PLUS;
            result.branch = 6;
            result.atomSpan = 1;
            result.shortParameter = result.source.charAt(1);
            result.value = result.source.substring(equals + 1);
            state.atomSelector++;
            return true;
        }

        /* branch 7: test for multiple short options */
        if ((shortDash || shortPlus) && equals < 0 && fullColon < 0 && length > 2) {
            result.type = (shortDash) ? ParameterType.PRMTR_TYPE_SHORT_DASH : ParameterType.PRMTR_TYPE_SHORT_PLUS;
            result.branch = 7;
            result.subatom = ++state.subatomSelector;
            result.shortParameter = result.source.charAt(result.subatom);
            if (result.subatom == finalSubPoint) {
                result.atomSpan = 1;
                state.atomSelector++;
                state.subatomSelector = 0;
                return true;
            } else {
                return true;
            }
        }

        /* branch 8: test for multiple short options with setting */
        if ((shortDash || shortPlus) && fullColon > 2) {
            finalSubPoint = fullColon - 1;
            result.type = (shortDash) ? ParameterType.PRMTR_TYPE_SHORT_DASH : ParameterType.PRMTR_TYPE_SHORT_PLUS;
            result.branch = 8;
            result.subatom = ++state.subatomSelector;
            result.shortParameter = result.source.charAt(result.subatom);
            if (result.subatom == finalSubPoint) {
                if (length > fullColon + 1) {
                    result.atomSpan = 1;
                    result.value = result.source.substring(fullColon + 1);
                } else {
                    state.atomSelector++;
                    if (state.atomSelector == state.atomCount) {
                        result.atomSpan = 1;
                        state.faultCode = ParameterParserFaultCode.PRMTR_NO_PARAMETER_VALUE_AVAILABLE;
                        return false;
                    }
                    result.atomSpan = 2;
                    result.value = state.atomTable[state.atomSelector];
                }
                state.atomSelector++;
                state.subatomSelector = 0;
                return true;
            } else {
                return true;
            }
        }

        /* branch 9: test for multiple short options with data */
        if ((shortDash || shortPlus) && equals > 3 && length > 3) {
            finalSubPoint = equals - 1;
            result.type = (shortDash) ? ParameterType.PRMTR_TYPE_SHORT_DASH : ParameterType.PRMTR_TYPE_SHORT_PLUS;
            result.branch = 9;
            result.subatom = ++state.subatomSelector;
            result.shortParameter = result.source.charAt(result.subatom);
            if (result.subatom == finalSubPoint) {
                result.atomSpan = 1;
                state.atomSelector++;
                result.value = result.source.substring(equals + 1);
                state.subatomSelector = 0;
                return true;
            } else {
                return true;
            }
        }

        /* branch 10: test for javascript style property */
        if (fullColon == (length -1)) {
            result.type = ParameterType.PRMTR_TYPE_PROPERTY;
            result.branch = 10;
            result.longParameter = result.source.substring(0, fullColon);
            result.atomSpan = 2;
            state.atomSelector++;
            if (state.atomSelector == state.atomCount) {
                state.faultCode = ParameterParserFaultCode.PRMTR_NO_PARAMETER_VALUE_AVAILABLE;
                return false;
            }
            result.value = state.atomTable[state.atomSelector];
            state.atomSelector++;
            return true;
        }

        result.atom = state.atomSelector++;
        state.faultCode = ParameterParserFaultCode.PRMTR_NO_PARAMETER_MATCH;
        return false;
        
    }

}
