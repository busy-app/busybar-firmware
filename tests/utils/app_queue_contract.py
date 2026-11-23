"""OpenAPI oracle for application queue requests and responses."""

from __future__ import annotations

from pathlib import Path

from jsonschema import Draft202012Validator
import yaml


class AppQueueContract:
    def __init__(self, repository: Path):
        directory = repository / "applications/services/web_server/openapi"
        root = yaml.safe_load((directory / "openapi.yaml").read_text(encoding="utf-8"))
        apps = yaml.safe_load((directory / "apps.yaml").read_text(encoding="utf-8"))
        self.schemas = {**root["components"]["schemas"], **apps["schemas"]}
        self.paths = apps["paths"]

    def validator(self, schema):
        return Draft202012Validator({**schema, "components": {"schemas": self.schemas}})

    def assert_request(self, path: str, body, *, valid: bool) -> None:
        schema = self.paths[path]["post"]["requestBody"]["content"]["application/json"][
            "schema"
        ]
        errors = list(self.validator(schema).iter_errors(body))
        assert (not errors) == valid, (
            f"Test request validity differs from OpenAPI: {body!r}; "
            f"errors={[error.message for error in errors]!r}"
        )

    def assert_error(self, response) -> None:
        assert (
            400 <= response.status_code < 500
        ), f"Invalid request was not rejected: HTTP {response.status_code}, {response.text[:300]!r}"
        self.validator({"$ref": "#/components/schemas/Error"}).validate(response.json())

    def assert_response(self, path: str, method: str, response) -> None:
        responses = self.paths[path][method]["responses"]
        key = str(response.status_code)
        assert key in responses, (
            f"{method.upper()} {path}: HTTP {key} is absent from OpenAPI responses "
            f"{list(responses)!r}; body={response.text[:300]!r}"
        )
        schema = responses[key]["content"]["application/json"]["schema"]
        self.validator(schema).validate(response.json())
