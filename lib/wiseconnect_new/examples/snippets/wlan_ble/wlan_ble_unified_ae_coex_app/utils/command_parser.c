#include <stdio.h>
#include "generic_command_handling.h"
#include "rsi_common_apis.h"
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void command_parser(uint8_t argc, uint8_t **argv);

/*******************************************************************************
 * extern functions 
 ******************************************************************************/
extern module_lut_entry_t root_module_lut[];
extern command_lut_entry_t root_commands_lut[];

/*==============================================*/
/**
 * @fn          void command_parser(uint8_t argc, uint8_t **argv)
 * @brief       function will find the command handler routine for the given command
 * @param[in]   uint8_t, argc
 * @param[in]   uint8_t, argv
 * @return      None
 *
 * @section description
 * function will find and invoke the respective command handler routine for the given command from UI task
 *
 */
void command_parser(uint8_t argc, uint8_t **argv)
{
  int current_cmd_index = 0, max_no_of_args = 0;
  module_lut_entry_t *current_module_lut = root_module_lut;
  module_lut_entry_t *module_lut_entry;
  command_lut_entry_t *current_command_lut = root_commands_lut;
  command_lut_entry_t *cmd_lut_entry;
  while (1) {
    //! call get_command_entry to get the commands from current_command_lut
    cmd_lut_entry = get_command_entry(&argv[current_cmd_index], current_command_lut);

    if (cmd_lut_entry != NULL) {
      //! check command handler
      if (cmd_lut_entry->handler == NULL) {
        printf("\r\n call generic command handler\r\n");
        break;
      } else {
        cmd_lut_entry->handler((argc - (max_no_of_args + 1)), &argv[current_cmd_index + 1]);
        break;
      }
    } else {
      //! call get_module_entry to get the sub_modules from current_module_lut
      module_lut_entry = get_module_entry(&argv[current_cmd_index], current_module_lut);
    }

    if (module_lut_entry != NULL) {
      //! check custom_handler
      if (module_lut_entry->custom_handler == NULL) {
        //! check if given args is sufficient to loop completely
        if (max_no_of_args < argc) {
          //! update the current_module_lut if submodules != NULL
          if (module_lut_entry->sub_modules != NULL) {
            current_module_lut = module_lut_entry->sub_modules;
          } else {
            break;
          }
          //! update the current_command_lut if commands != NULL
          if (module_lut_entry->commands != NULL) {
            current_command_lut = module_lut_entry->commands;
          } else {
            break;
          }
          //! increment the current_cmd_index
          current_cmd_index++;
          //! increment the max_no_of_args
          max_no_of_args++;
        } else {
          //! check if modules_lut_entry->submodules != NULL then update the current_module_lut and call display fucntion
          if (module_lut_entry->sub_modules != NULL) {
            current_module_lut = module_lut_entry->sub_modules;
            //! call display sub_module function
            display_submodule(&argv[current_cmd_index], current_module_lut);
          } else {
            printf("\r\n sub_modules LUT not present\r\n");
            break;
          }
          //! check if modules_lut_entry->commands != NULL then update the current_command_lut and call display fucntion
          if (module_lut_entry->commands != NULL) {
            current_command_lut = module_lut_entry->commands;
            //! call display command function
            display_command(&argv[current_cmd_index], current_command_lut);
          } else {
            printf("\r\n commands LUT not present\r\n");
            break;
          }
          break;
        }
      } else {
        printf("\r\n call current_module_lut custom_handler\r\n");
        break;
      }
    } else { //! if user input wrong sub_module and commands then call display fucntion
      printf("\r\n Command parser: Enter correct module_name \r\n");
      //! call display sub_module function
      display_submodule(&argv[current_cmd_index], current_module_lut);
      printf("\r\n Command parser: Enter correct command_name \r\n");
      //! call display command function
      display_command(&argv[current_cmd_index], current_command_lut);
      break;
    }
  }
}
/**
 * @brief  This function gets the commands with respect to current module
 * @param  current argument
 * @param  pointer to the current_command_lut
 * @return pointer to current_command if command_name is not NULL else return NULL
 */
command_lut_entry_t *get_command_entry(uint8_t **argv, command_lut_entry_t *current_command)
{
  int j = 0;
  while (current_command[j].command_name != NULL) {
    //! compare given command_name with current_command->command_name
    if (strcmp((char *)(*argv), (char *)current_command[j].command_name) == 0) {
      return &current_command[j];
    }
    j++;
  }
  return NULL;
}

/**
 * @brief  This function gets the module with respect to current module
 * @param  current argument
 * @param  pointer to the current_module_lut
 * @return pointer to current_module if module_name is not NULL else return NULL
 */
module_lut_entry_t *get_module_entry(uint8_t **argv, module_lut_entry_t *current_module)
{
  int i = 0;
  while (current_module[i].module_name != NULL) {
    //! compare gievn module_name with current_module->module_name
    if (strcmp((char *)(*argv), (char *)current_module[i].module_name) == 0) {
      return &current_module[i];
    }
    i++;
  }
  return NULL;
}

/**
 * @brief  This function displays the name of next sub_module if input is wrong
 * @param  current argument
 * @param  pointer to the current_module_lut
 * @return none
 */
void display_submodule(uint8_t __attribute__((unused)) * *argv, module_lut_entry_t *current_module)
{
  int i                      = 0;
  char buff[MAX_BUFF_LENGTH] = { 0 };
  while (current_module[i].module_name != NULL) {
    //! storing the list of sub_modules present wrt present current_module
    strncat(buff, (char *)current_module[i].module_name, strlen((char *)current_module[i].module_name));
    strcat(buff, "\n");
    i++;
  }
  if (current_module->module_name != NULL) {
    //! if module_name != NULL then print the list of module_name
    printf("sub_module present : \n%s", buff);
  }
}

/**
 * @brief  This function displays the name of next command
 * @param  current argument
 * @param  pointer to the current_module_lut
 * @return none
 */
void display_command(uint8_t __attribute__((unused)) * *argv, command_lut_entry_t *current_command)
{
  int j                      = 0;
  char buff[MAX_BUFF_LENGTH] = { 0 };
  while (current_command[j].command_name != NULL) {
    //! storing the list of commands present wrt present current_command
    strncat(buff, (char *)current_command[j].command_name, strlen((char *)current_command[j].command_name));
    strcat(buff, "\n");
    j++;
  }
  if (current_command->command_name != NULL) {
    //! if command_name != NULL then print the list of commands
    printf("command present : \n%s", buff);
  }
}
