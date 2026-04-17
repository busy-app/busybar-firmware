ifndef HOST_OS
$(error Must define HOST_OS to "windows", "linux", or "macos")
endif

$(PROJECT_OUTPUT)/generated/console_variable_database.c: $(PROJECT_OUTPUT)/generated/console_variable_database.yaml  | $(PROJECT_OUTPUT)/generated/.directory
	components/yinja/yinja-$(HOST_OS) -t $(console_DIRECTORY)/console_variable_table.c.template -t $(console_DIRECTORY)/console_variable_group_processing.template -y $(PROJECT_OUTPUT)/generated/console_variable_database.yaml > $@

$(PROJECT_OUTPUT)/generated/console_variable_database.yaml: | $(PROJECT_OUTPUT)/generated/.directory
	components/yinja/yinja-$(HOST_OS) -t $(console_DIRECTORY)/console_variable_table.yaml.template -j $(PROJECT_OUTPUT)/yakka_summary.json > $@

$(PROJECT_OUTPUT)/generated/console_command_database.c: $(console_DIRECTORY)/console_command_database.c.template | $(PROJECT_OUTPUT)/generated/.directory
	components/yinja/yinja-$(HOST_OS) -t $(console_DIRECTORY)/console_command_database.c.template -t $(console_DIRECTORY)/console_command_processing.template -j $(PROJECT_OUTPUT)/yakka_summary.json > $@

$(PROJECT_OUTPUT)/generated/console_argument_types.h: $(console_DIRECTORY)/console_argument_types.h.template | $(PROJECT_OUTPUT)/generated/.directory
	components/yinja/yinja-$(HOST_OS) -t $(console_DIRECTORY)/console_argument_types.h.template -j $(PROJECT_OUTPUT)/yakka_summary.json > $@

$(PROJECT_OUTPUT)/generated/console_argument_types.c: $(console_DIRECTORY)/console_argument_types.c.template | $(PROJECT_OUTPUT)/generated/.directory
	components/yinja/yinja-$(HOST_OS) -t $(console_DIRECTORY)/console_argument_types.c.template -j $(PROJECT_OUTPUT)/yakka_summary.json > $@
