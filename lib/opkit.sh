
# opkit gives you 9 parameter triggers.

function opkit.match() {
  local match="$1"; shift;
  for param; do [[ "$match" == "$param" ]] && return 0; done;
  return 1;
}

function opkit.keys() { eval echo \${!$1[@]}\;; }

function opkit.set() {
  local destination=$1 assignment; shift;
  for assignment; do
    local key="${assignment/=*/}";
    local value="${assignment/*=/}"
    printf -v ${destination}$key %s "$value"
  done;
}

function opkit.get() {
  local source=$1 name; shift;
  for name; do eval echo \"\${$source[$name]}\"\;; done;
}

function opkit.content() { eval echo \${$1[@]}; }

function opkit.begin() { opkit.set $1 [INDEX]=1 [SUBINDEX]=0; }

function opkit.dump() { 

  local name;
  for name in $(opkit.keys $1); do
    printf '%s: %s\n' $name "$(opkit.get $1 $name)";
  done

}

function opkit.parse() {

  # PARAMETERS: $1 and $2 are names of associative arrays.
  # the output of the function will be placed in $2
  # the state of the parsing is saved in $1.

  # the remaining parameters are parsed. the number of items parsed are
  # returned in 2[SIZE].

  local OK_STATE=$1 RCRD_PARAMETER=$2; shift 2;

  local INDEX=$OK_STATE[INDEX] SUBINDEX=$OK_STATE[SUBINDEX];
  local PARAMETER=BASH_REMATCH[1] VALUE=BASH_REMATCH[2]
    
  opkit.set $RCRD_PARAMETER [BRANCH]=0 [INDEX]=${!INDEX} \
    [INPUT]="$1" [SUBINDEX]=0 [VALUE]='' [SHORT]=0 [SIZE]=1;

  # 1 test for long option
  [[ "$1" =~ ^--([a-zA-Z-]*[a-zA-Z])$ ]] && {
    opkit.set $RCRD_PARAMETER [BRANCH]=1 [PARAMETER]="${!PARAMETER}";
    opkit.match "${!PARAMETER}" $(opkit.get $OK_STATE LONG) || return 11;
    let $INDEX++;
    return 0;
  }

  # 2 test for long option with setting specification
  [[ "$1" =~ ^--([a-zA-Z-]*[a-zA-Z]):$ ]] && {
    opkit.set $RCRD_PARAMETER [BRANCH]=2 [PARAMETER]="${!PARAMETER}" \
      [VALUE]="$2" [SIZE]=2;
    opkit.match "${!PARAMETER}" $(opkit.get $OK_STATE LONG) || return 21;
    opkit.match "${!PARAMETER}" $(opkit.get $OK_STATE SETTINGS) || \
      return 22;
    let $INDEX+=2;
    return 0;
  }

  # 3 test for long option with setting specification and data
  [[ "$1" =~ ^--([a-zA-Z-]*[a-zA-Z])[:=](.*)$ ]] && {
    opkit.set $RCRD_PARAMETER [BRANCH]=3 [PARAMETER]="${!PARAMETER}" \
      [VALUE]="${!VALUE}";
    opkit.match "${!PARAMETER}" $(opkit.get $OK_STATE LONG) || return 31;
    opkit.match "${!PARAMETER}" $(opkit.get $OK_STATE SETTINGS) || \
      return 32;
    let $INDEX++;
    return 0;
  }

  # 4 test for short option
  [[ "$1" =~ ^-([a-zA-Z])$ ]] && {
    opkit.set $RCRD_PARAMETER [BRANCH]=4 [PARAMETER]="${!PARAMETER}" \
      [SHORT]=1;
    opkit.match "${!PARAMETER}" $(opkit.get $OK_STATE SHORT) || return 41;
    let $INDEX++;
    return 0;
  }

  # 5 test for short option with setting
  [[ "$1" =~ ^-([a-zA-Z]):$ ]] && {
    opkit.set $RCRD_PARAMETER [BRANCH]=5 [PARAMETER]="${!PARAMETER}" \
      [VALUE]="$2" [SHORT]=1 [SIZE]=2;
    opkit.match "${!PARAMETER}" $(opkit.get $OK_STATE SHORT) || return 51;
    opkit.match "${!PARAMETER}" $(opkit.get $OK_STATE SETTINGS) || \
      return 52;
    let $INDEX+=2;
    return 0;
  }

  # 6 test for short option with setting and data
  [[ "$1" =~ ^-([a-zA-Z])[:=](.+)$ ]] && {
    opkit.set $RCRD_PARAMETER [BRANCH]=6 [PARAMETER]="${!PARAMETER}" \
      [VALUE]="${!VALUE}" [SHORT]=1;
    opkit.match "${!PARAMETER}" $(opkit.get $OK_STATE SHORT) || return 61;
    opkit.match "${!PARAMETER}" $(opkit.get $OK_STATE SETTINGS) || \
      return 62;
    let $INDEX++;
    return 0;
  }
  
  # 7 test for multiple short option
  [[ "$1" =~ ^-([a-zA-Z]+)$ ]] && {
    local match=${BASH_REMATCH[1]};
    local -i length=${#match};
    local C=${match:${!SUBINDEX}:1};
    opkit.set $RCRD_PARAMETER [BRANCH]=7 [PARAMETER]="$C" [INDEX]=${!INDEX} \
      [SUBINDEX]=${!SUBINDEX} [SHORT]=1;
    opkit.match "${C}" $(opkit.get $OK_STATE SHORT) || return 71;
    let $SUBINDEX++;
    if (( $SUBINDEX == length )); then
      opkit.set $RCRD_PARAMETER [SIZE]=1;
      let $INDEX++ $SUBINDEX=0;
    else
      opkit.set $RCRD_PARAMETER [SIZE]=0;
      opkit.match "${C}" $(opkit.get $OK_STATE SETTINGS) && {
        opkit.match "$C" $(opkit.get $OK_STATE HAS_DEFAULT) || return 72;
      } 
    fi;
    return 0;
  }

  # 8 test for multiple short option with setting
  [[ "$1" =~ ^-([a-zA-Z]+):$ ]] && {
    local match=${BASH_REMATCH[1]};
    local -i length=${#match};
    local C=${match:${!SUBINDEX}:1};
    opkit.set $RCRD_PARAMETER [BRANCH]=8 [PARAMETER]="$C" [INDEX]=${!INDEX} \
      [SUBINDEX]=${!SUBINDEX} [SHORT]=1;
    opkit.match "${C}" $(opkit.get $OK_STATE SHORT) || return 81;
    let $SUBINDEX++;
    if (( $SUBINDEX == length )); then
      opkit.set $RCRD_PARAMETER [VALUE]="$2";
      opkit.set $RCRD_PARAMETER [SIZE]=2;
      opkit.match "${C}" $(opkit.get $OK_STATE SETTINGS) && {
        opkit.set $RCRD_PARAMETER [VALUE]="$2" [SIZE]=2;
        let $INDEX++;
      } 
      let $INDEX++ $SUBINDEX=0;
    else
      opkit.set $RCRD_PARAMETER [SIZE]=0;
      opkit.match "${C}" $(opkit.get $OK_STATE SETTINGS) && {
        opkit.match "$C" $(opkit.get $OK_STATE HAS_DEFAULT) || return 82;
      } 
    fi;
    return 0;
  }

  # 9 test for multiple short option with setting and data
  [[ "$1" =~ ^-([a-zA-Z]+)[:=](.+)$ ]] && {
    local match=${BASH_REMATCH[1]};
    local -i length=${#match};
    local C=${match:${!SUBINDEX}:1};
    opkit.set $RCRD_PARAMETER [BRANCH]=9 [PARAMETER]="$C" [INDEX]=${!INDEX} \
      [SUBINDEX]=${!SUBINDEX} [SHORT]=1;
    opkit.match "${C}" $(opkit.get $OK_STATE SHORT) || return 91;
    let $SUBINDEX++;
    if (( $SUBINDEX == length )); then
      opkit.set $RCRD_PARAMETER [VALUE]="${BASH_REMATCH[2]}";
      opkit.set $RCRD_PARAMETER [SIZE]=1;
      opkit.match "${C}" $(opkit.get $OK_STATE SETTINGS) && {
        opkit.set $RCRD_PARAMETER [VALUE]="${!VALUE}";
      } 
      let $INDEX++ $SUBINDEX=0;
    else
      opkit.set $RCRD_PARAMETER [SIZE]=0;
      opkit.match "${C}" $(opkit.get $OK_STATE SETTINGS) && {
        opkit.match "$C" $(opkit.get $OK_STATE HAS_DEFAULT) || return 92;
      } 
    fi;
    return 0;
  }
  return 1;
}

# end option parsing utilities section

(( OK_DEBUG )) && {

  # todo: error codes for too few options
  # todo: make branch zero literal match, [taking primary match priority over all processing]
  # and branch 10 regular expression match
  # and branch 11 failed to parse.
  # add the plus/minus status and trigger
  
  declare -A CONFIG;

  opkit.set CONFIG [LONG]="get-theatre help"
  opkit.set CONFIG [SHORT]="H T L Q R d e f h i l l p q z";
  opkit.set CONFIG [SETTINGS]="H Q T e i l q z"
  
  # options that have a default can be specified with or without assignment
  # operators. an option listed here has no effect if it is not listed in
  # CONFIG[SETTINGS]
  opkit.set CONFIG [HAS_DEFAULT]="H T"

  opkit.begin CONFIG;

  while (( $# )); do
    declare -A parse;
    opkit.parse CONFIG parse "$@" || {
      echo "error: parameter #${parse[INDEX]} didn't parse";
      opkit.dump parse;
      exit 1;
    }
    opkit.dump parse;
    shift ${parse[SIZE]}
  done;

  declare -p CONFIG

}

