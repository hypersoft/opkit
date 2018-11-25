### The Hypersoft-Systems: U.-S.-A.: C Parameter Parsing Suite (C.-P.-P.-S)

The C Parameter Parsing Suite (CPPS) provides applications with a simple parameter
parsing backend which is capable of parsing any command-line-parameter(s) given
by users including void/non-existent parameters.

The parsing suite is a "dumb", parser which means it has no knowledge of the
content that it is parsing. It only transforms the parameters given to tokens,
which can then be lexically validated by the calling application, in a
step-by-step manner. You can compare this to "GNU's Get Opt" which assumes
expert knowledge of all parameters that it will parse, thusly requiring heavy
configuration and expert knowledge on the part of the API users.

CPPS requires no configuration, you just load your vectors and begin parsing,
right away. If the parser parses a parameter your application does not support,
you are responsible for notifying the user and responding accordingly. 

CPPS's method of parameter parsing is suitable for all application profiles.
Especially applications which must support, multiple/different parameter sets,
based on specifications which have been supplied by an application user.

CPPS supports each of the following "command line switch" profiles:

```
  --* switches (known as long options)
   -* switches (known as short/compound-short options)
   +* switches (known as short/compound-short toggles)

    * everything else (known as raw-data)
```

Please note that the asterisk '*' in the above menu means 1 or more non-zero
character values, not-exactly: alpha-numeric-characters.

Each switch may have an optional '=' appended to it with data joined:

```
  switch=raw-data
```
or may have ':' appended to it with data following as the next parameter:

```
  switch: raw-data
```

Of course, this is not what everyone wants for their application, and if your
application does not want to support a feature, all that it must do is tell
the user: "Stop doing that!". And if an application wants switches and data,
without the assignment operators \[:=], its simply a matter of parsing the next
parameter after detecting the correct switch.

CPPS takes care of the hard work involved in the parsing of switches and data.

  1. If a parameter has data associated by assignment operation, that data will be returned to the caller with the switch specification on-demand.

  2. If a compound switch is followed by data, the data only applies to the last switch in the set.

  3. If a switch specifies data, and data is not supplied, the application will be notified though a fault code which is stored within the `ParameterParserState`.

  4. If an application executes an incorrect call, the user will be notified, and the application will be aborted.

CPPS Neatly keeps tabs on all data. While using CPPS your application will always
have the knowledge needed to notify the user that parameter number "X" is not valid.
If the the invalid parameter of "X" is in a compound set, your application will
also have the meta-data to explain the relationship of "X" and "Y" to the user,
such as: "parameter #4 at sub-position #3 is an illegal option \[CAUSE]".

#### Coding With CPPS

Coding with CPPS may seem like a daunting task with its highly-adept-feature-set,
however this assumption could not be further from the truth. An application
requires only two API calls to handle any parameters specified by a user.

The first API call is `param_begin_parameter_parsing`; which initializes a caller
supplied parser state.

The second API call is `param_parse_next_parameter` which is consecutively called
by an application until no further processing is required.

In order to support application coders there are two more API calls which are
for debugging purposes.

The first API call is `param_debug_print_parameter` which will print a complete 
report on what was parsed from a previous call to `param_parse_next_parameter`.

The second API call is `param_get_parameter_type_string`, which will retrieve a
string pointer that uniquely identifies what the type of parameter parsed is. A
parameter type can be one of:

  * void
  * long
  * short-dash
  * short-plus

The void type, is reserved for all raw-data (not parsed thusly: void), and
the other types are here assumed to be self-explanatory.

There are two C structures which an application coder must familiarize themselves
with to be effective at using this suite.

The first is `ParameterParserState` which keeps track of the parameter token
stream meta-data and all required parsing meta-data such as last fault code,
and current position within the token stream (commonly known as "argv").

The second is `ParameterData` which contains all of the relevant data about a
parsed (or unparsed) parameter, such as position, value, source (context) and
sub position within the source for compound parameter items.

To use the API, you simply allocate the foregoing structures and pass the pointers
to those structures through the API.

Here is some sample code written in the C programming language, which is used
to debug the API:

```
#include <stdio.h>
#include <stdlib.h>

#include "parameters.h"

int main(int argc, char * argv[])
{
  ParameterParserState state;
  param_begin_parameter_parsing(&state, argc, argv);
  ParameterData parameter;
  param_parse_next_parameter(&state, &parameter);
  param_debug_print_parameter(&parameter);
  if (parameter.longParameter) free(parameter.longParameter);
  if (parameter.subatom) {
    while (param_parse_next_parameter(&state, &parameter)) {
      param_debug_print_parameter(&parameter);
      puts("");
    }
  }
  exit(0);
}
```

Simply call the application and it will parse the first parameter given along with
any data that has been assigned to it. If the first parameter is a compound switch,
it will iterate through each switch in the set and report what was parsed.

#### API Terminology

atom-table = array of parameter (string) pointers

atom = index/offset in atom-table

subatom = index/offset of character in atom-table (argv\[atom]\[subatom])

fault = error/exception

void = undefined/unknown/non-existent

PRMTR = parameter


#### Closing

That's all for this document. If you need any further help, or direction, try
asking a question on StackOverflow. There are many knowledgeable coders who can
be helpful with debugging you or your code. This code is simple enough for any C
coder who can write a program from scratch; but don't be fooled by its
simplicity. Simple in this context merely means versatile/adaptable and free of
gadgets which are non-essential.

See: [elementary](https://www.google.com/search?q=define+elementary) for an
example of "simple".

"UNIX is very simple, it just needs a genius to understand its simplicity."
-- Dennis Ritchie (founding father of C programming)