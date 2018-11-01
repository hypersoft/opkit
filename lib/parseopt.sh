
# parseopt gives you 9 parameter triggers.

function parseopt.match() {
  local match="$1"; shift;
  for param; do [[ "$match" == "$param" ]] && return 0; done;
  return 1;
}

function parseopt.keys() {
  eval echo \${!$1[@]}\;;
}

function parseopt.set() {
  local destination=$1 assignment; shift;
  for assignment; do
    local key="${assignment/=*/}";
    local value="${assignment/*=/}"
    printf -v ${destination}$key %s "$value"
  done;
}

function parseopt.get() {
  local source=$1 name; shift;
  for name; do
    eval echo \${$source[$name]}\;;
  done;
}

function parseopt.content() {
  eval echo \${$1[@]};
}

function parseopt.begin() {
  parseopt.set $1 [INDEX]=1 [SUBINDEX]=0;
}

function parseopt.dump() { 

  local name;
  for name in $(parseopt.keys $1); do
    printf '%s: %s\n' $name "$(parseopt.get $1 $name)";
  done

}

function parseopt() {

  # PARAMETERS: $1 and $2 are names of associative arrays.
  # the output of the function will be placed in $2
  # the state of the parsing is saved in $1.

  # the remaining parameters are parsed. the number of items parsed are
  # returned in 2[SIZE].

  local PO_STATE=$1 RCRD_PARAMETER=$2; shift 2;

  local INDEX=$PO_STATE[INDEX] SUBINDEX=$PO_STATE[SUBINDEX];
  local PARAMETER=BASH_REMATCH[1] VALUE=BASH_REMATCH[2]
    
  parseopt.set $RCRD_PARAMETER [BRANCH]=0 [INDEX]=${!INDEX};

  # 1 test for long option
  [[ "$1" =~ ^--([a-zA-Z-]*[a-zA-Z])$ ]] && {
    parseopt.set $RCRD_PARAMETER [BRANCH]=1 \
      [PARAMETER]="${!PARAMETER}" [INDEX]=${!INDEX} \
        [SUBINDEX]=0 [VALUE]='' [SHORT]=0 [SIZE]=1;
    parseopt.match "${!PARAMETER}" $(parseopt.get $PO_STATE LONG) || return 1;
    parseopt.match "${!PARAMETER}" $(parseopt.get $PO_STATE SETTINGS) && {
      parseopt.set $RCRD_PARAMETER [VALUE]="$2" [SIZE]=2;
      let $INDEX++;
    };
    let $INDEX++;
    return 0;
  }

  # 2 test for long option with setting specification
  [[ "$1" =~ ^--([a-zA-Z-]*[a-zA-Z]):$ ]] && {
    parseopt.set $RCRD_PARAMETER [BRANCH]=2 \
      [PARAMETER]="${!PARAMETER}" [INDEX]=${!INDEX} \
        [SUBINDEX]=0 [VALUE]="$2" [SHORT]=0 [SIZE]=2;
    parseopt.match "${!PARAMETER}" $(parseopt.get $PO_STATE LONG) || return 1;
    parseopt.match "${!PARAMETER}" $(parseopt.get $PO_STATE SETTINGS) || return 1;
    let $INDEX+=2;
    return 0;
  }

  # 3 test for long option with setting specification and data
  [[ "$1" =~ ^--([a-zA-Z-]*[a-zA-Z])[:=](.*)$ ]] && {
    parseopt.set $RCRD_PARAMETER [BRANCH]=3 \
      [PARAMETER]="${!PARAMETER}" [INDEX]=${!INDEX} \
        [SUBINDEX]=0 [VALUE]="${!VALUE}" [SHORT]=0 [SIZE]=1;
    parseopt.match "${!PARAMETER}" $(parseopt.get $PO_STATE LONG) || return 1;
    parseopt.match "${!PARAMETER}" $(parseopt.get $PO_STATE SETTINGS) || return 1;
    let $INDEX++;
    return 0;
  }

  # 4 test for short option
  [[ "$1" =~ ^-([a-zA-Z])$ ]] && {
    parseopt.set $RCRD_PARAMETER [BRANCH]=4 \
      [PARAMETER]="${!PARAMETER}" [INDEX]=${!INDEX} \
        [SUBINDEX]=0 [VALUE]="" [SHORT]=1 [SIZE]=1;
    parseopt.match "${!PARAMETER}" $(parseopt.get $PO_STATE SHORT) || return 1;
    parseopt.match "${!PARAMETER}" $(parseopt.get $PO_STATE SETTINGS) && {
      parseopt.set $RCRD_PARAMETER [VALUE]="$2" [SIZE]=2;
      let $INDEX++;
    };
    let $INDEX++;
    return 0;
  }

  # 5 test for short option with setting
  [[ "$1" =~ ^-([a-zA-Z]):$ ]] && {
    parseopt.set $RCRD_PARAMETER [BRANCH]=5 \
      [PARAMETER]="${!PARAMETER}" [INDEX]=${!INDEX} \
        [SUBINDEX]=0 [VALUE]="$2" [SHORT]=1 [SIZE]=2;
    parseopt.match "${!PARAMETER}" $(parseopt.get $PO_STATE SHORT) || return 1;
    parseopt.match "${!PARAMETER}" $(parseopt.get $PO_STATE SETTINGS) || return 1;
    let $INDEX+=2;
    return 0;
  }

  # 6 test for short option with setting and data
  [[ "$1" =~ ^-([a-zA-Z])[:=](.+)$ ]] && {
    parseopt.set $RCRD_PARAMETER [BRANCH]=6 \
      [PARAMETER]="${!PARAMETER}" [INDEX]=${!INDEX} \
        [SUBINDEX]=0 [VALUE]="${!VALUE}" [SHORT]=1 [SIZE]=1;
    parseopt.match "${!PARAMETER}" $(parseopt.get $PO_STATE SHORT) || return 1;
    parseopt.match "${!PARAMETER}" $(parseopt.get $PO_STATE SETTINGS) || return 1;
    let $INDEX++;
    return 0;
  }
  
  # 7 test for multiple short option
  [[ "$1" =~ ^-([a-zA-Z]+)$ ]] && {
    local match=${BASH_REMATCH[1]};
    local -i length=${#match};
    local C=${match:${!SUBINDEX}:1};
    parseopt.set $RCRD_PARAMETER [BRANCH]=7 \
      [PARAMETER]="$C" [INDEX]=${!INDEX} \
        [SUBINDEX]=${!SUBINDEX} [VALUE]="" [SHORT]=1;
    parseopt.match "${C}" $(parseopt.get $PO_STATE SHORT) || return 1;
    let $SUBINDEX++;
    if (( $SUBINDEX == length )); then
      eval $RCRD_PARAMETER[SIZE]=1;
      parseopt.match "${C}" $(parseopt.get $PO_STATE SETTINGS) && {
        parseopt.set $RCRD_PARAMETER [VALUE]="$2" [SIZE]=2;
        let $INDEX++;
        true
      } 
      let $INDEX++;
      let $SUBINDEX=0;
    else
      eval $RCRD_PARAMETER[SIZE]=0;
      parseopt.match "${C}" $(parseopt.get $PO_STATE SETTINGS) && {
        echo error: ${BASH_LINENO}
        return 1;
      } 
    fi;
    return 0;
  }

  # 8 test for multiple short option with setting
  [[ "$1" =~ ^-([a-zA-Z]+):$ ]] && {
    local match=${BASH_REMATCH[1]};
    local -i length=${#match};
    local C=${match:${!SUBINDEX}:1};
    parseopt.set $RCRD_PARAMETER [BRANCH]=8 \
      [PARAMETER]="$C" [INDEX]=${!INDEX} \
        [SUBINDEX]=${!SUBINDEX} [VALUE]="" [SHORT]=1;
    parseopt.match "${C}" $(parseopt.get $PO_STATE SHORT) || return 1;
    parseopt.match "${C}" $(parseopt.get $PO_STATE SETTINGS) || return 1;
    let $SUBINDEX++;
    if (( $SUBINDEX == length )); then
      eval $RCRD_PARAMETER[VALUE]="$2";
      eval $RCRD_PARAMETER[SIZE]=2;
      parseopt.match "${C}" $(parseopt.get $PO_STATE SETTINGS) && {
        parseopt.set $RCRD_PARAMETER [VALUE]="$2" [SIZE]=2;
        let $INDEX++;
      } 
      let $INDEX++;
      let $SUBINDEX=0;
    else
      eval $RCRD_PARAMETER[SIZE]=0;
      parseopt.match "${C}" $(parseopt.get $PO_STATE SETTINGS) && {
        echo error: ${BASH_LINENO}
        return 1;
      } 
    fi;
    return 0;
  }

  # 9 test for multiple short option with setting and data
  [[ "$1" =~ ^-([a-zA-Z]+)[:=](.+)$ ]] && {
    local match=${BASH_REMATCH[1]};
    local -i length=${#match};
    local C=${match:${!SUBINDEX}:1};
    parseopt.set $RCRD_PARAMETER [BRANCH]=1 \
      [PARAMETER]="$C" [INDEX]=${!INDEX} \
        [SUBINDEX]=${!SUBINDEX} [VALUE]="" [SHORT]=1;
    parseopt.match "${C}" $(parseopt.get $PO_STATE SHORT) || return 1;
    parseopt.match "${C}" $(parseopt.get $PO_STATE SETTINGS) || return 1;
    let $SUBINDEX++;
    if (( $SUBINDEX == length )); then
      eval $RCRD_PARAMETER[VALUE]="${BASH_REMATCH[2]}";
      eval $RCRD_PARAMETER[SIZE]=1;
      parseopt.match "${C}" $(parseopt.get $PO_STATE SETTINGS) && {
        parseopt.set $RCRD_PARAMETER [VALUE]="${!VALUE}";
      } 
      let $INDEX++;
      let $SUBINDEX=0;
    else
      eval $RCRD_PARAMETER[SIZE]=0;
      parseopt.match "${C}" $(parseopt.get $PO_STATE SETTINGS) && {
        echo error: ${BASH_LINENO}
        return 1;
      } 
    fi;
    return 0;
  }
  
  return 1;

}


# end option parsing utilities section

(( PARSE_OPT_DEBUG )) && {

  declare -A CONFIG;

  parseopt.set CONFIG [LONG]="file"
  parseopt.set CONFIG [SHORT]="f r w x";
  parseopt.set CONFIG [SETTINGS]="file f"

  parseopt.begin CONFIG;

  while (( $# )); do
    declare -A parse;
    parseopt CONFIG parse "$@" || {
      echo "error: parameter #${parse[INDEX]} didn't parse";
      parseopt.dump parse;
      exit 1;
    }
    parseopt.dump parse;
    shift ${parse[SIZE]}
  done;

  declare -p CONFIG

}

