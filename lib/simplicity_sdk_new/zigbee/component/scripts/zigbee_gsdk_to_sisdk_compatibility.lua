local changeset = {}

table.insert(changeset, {
    ["status"] = "user_verification",
    ["description"] = "Zigbee public APIs, enumerations, etc. are renamed to a new format, manually updating project specific source files may be required. \
                       Refer to \"Zigbee API Reference v7 vs. v8 (Zigbee Documentation Version 8.0.0)\" to convert old APIs to the new ones."}
)

return changeset