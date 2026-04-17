-- Ncp component to soc application compatibility check

-- local ncp = slc.is_selected('ncp', vendor='silabs', package='bluetooth_mesh_middleware')
for idx,ncpComp in pairs(slc.search_components({id='ncp'})) do
  if (ncpComp.is_selected == false) then
    validation.error(
      'NCP components are not prepared for integration in SOC applications',
      validation.target_for_project(),
      'Remove NCP components',
      nil
    )
  end
end
