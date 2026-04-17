# COMMON TOKEN MANAGER Bare Metal Application

This example application demonstrates the use of Common Token Manager for series 2 devices. The application uses command line interface to execute operations which are given by user.

Users can give the following commands:
NOTE: write_custom_static_token and read_custom_static_token automatically performs operations by using predefined tokens through the application itself. All other commands are interactive through CLI.

The below commands are just few of the operations. This can be expanded as required.
If a new custom token is introduced (apart from what is already provided), then a corresponding size should also be defined.

* Write Custom Static Token (write_custom_static_token)
Argument 0:
0 - To write the custom static secure tokens
1 - To write the custom static device tokens

* Read Custom Static Token (read_custom_static_token)
Argument 0:
0 - To read the custom static secure tokens
1 - To read the custom static device tokens

* Write Dynamic Token (write_dynamic_token)
Argument 0:
0 - To write a data object token
1 - To write an override token created for static device tokens
2 - To write an override token created for static secure tokens
Argument 1:
This argument is for the key.
If argument 0 is either 1 or 2, the key should be less than 0xFFF.
Argument 2:
This argument is for providing data.

* Read Dynamic Token (read_dynamic_token)
Argument 0:
0 - To read a data object token
1 - To read an override token created for static device tokens
2 - To read an override token created for static secure tokens
Argument 1:
This argument is for providing a key which has already been entered else it will throw an error.

* Delete Dynamic Token (delete_dynamic_token)
Argument 0:
0 - To delete a dynamic token
1 - To delete a static device override token
2 - To delete a static secure override token
Argument 1:
This argument is for the key.

* Write Counter Token (write_counter_token)
Argument 0:
This argument is to specify the value that you want to set the counter to.

* Read Counter Token (read_counter_token)
No Arguments

* Increment Counter Token (increment_counter_token)
No Arguments

## Requirements

Silicon Labs board