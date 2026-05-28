# OpenAPI spec generator
The generator script `scripts/openapi_merge.py` takes per-tag YAML files
(`system.yaml`, `time.yaml`, etc.) and joins them together.

  - Contents of the top-level `paths` field are inserted into the `paths` field
    of the generated file
  - Contents of the top-level `schemas` field are inserted into the
    `components/schemas` field of the generated file

`openapi.yaml` is simply a placeholder file.

## Converting existing definitions
  - Take a number of related endpoint definitions
  - Place them into a separate file under the `paths` field
  - Look at the list of schema references that these endpoints use
  - Place those schema components under the `schemas` field
    - Exceptions: `SuccessResponse`, `Error`, as they're global

## Writing new definitions
  - Place your endpoints under the `paths` field
  - Place your schema components under the `schemas` field (not
    `components/schemas`)
