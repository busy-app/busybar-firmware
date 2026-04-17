if not slc.is_selected("interrupt_manager") and not slc.is_provided("interrupt_manager") and not slc.is_provided("cortexm0plus") and not slc.is_provided("device_si91x") then
    validation.warning("Interrupts priorities are not automatically configured since interrupt_manager is not selected",
                       validation.target_for_project(),
                       "To prevent corruption caused by concurrent calls to the FreeRTOS API, interrupts priorities need to be configured according to the FreeRTOS configMAX_SYSCALL_INTERRUPT_PRIORITY setting in FreeRTOSConfig.h",
                       validation.target_for_project())
end
