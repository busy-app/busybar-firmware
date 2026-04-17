if slc.is_selected("wisun_rcp") and slc.is_selected("wisun_stack_ffn") then
    validation.error('Wi-SUN rcp project should not include wisun_stack_ffn component', validation.target_for_project())
end