local changeset = {}

if slc.is_provided("ot_ncp")
   and (slc.is_provided("ot_stack_ftd") or slc.is_provided("ot_stack_mtd"))
   and not slc.is_provided("ot_cli") then

  table.insert(changeset, {
    component = "ot_cli_source",
    action = "add"
  })
end

return changeset
