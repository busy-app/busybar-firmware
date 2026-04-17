# Console

The console component provides a command line interface and a process for defining commands and argument types

## Argument types

The definition of custom argument types is structured as below:
	console:
	  enums:
	    <argument name>:
	      <option name>: <option value>
	      <option name>: <option value>
	      ...

Generally the types and values of the enumeration for the custom argument is in another header file, to add a #include statement for the relevant file you may add entries under /console/include_headers.
	console:
	  include_headers:
	    - path/to/my_header_file.h

