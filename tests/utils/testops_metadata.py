"""BUSY test taxonomy. Metadata changes labels, never pytest selection or identity.

Use ``pytest.mark.bsb_metadata`` on a module, class or test. The closest
declaration replaces a field, including the entire tuple for multivalue fields.
Product Epic / Feature / Story remain native Allure decorators.
"""

FIELD_LABELS = {
    "components": "component",
    "suite": "suite",
    "level": "test_level",
    "intent": "test_intent",
    "surfaces": "surface",
    "interfaces": "interface",
    "transports": "transport",
    "hardware": "hardware_dependency",
    "release_lanes": "release_lane",
    "criticality": "release_criticality",
}
MULTIPLE = {
    "components",
    "surfaces",
    "interfaces",
    "transports",
    "hardware",
    "release_lanes",
}
CHOICES = {
    "suite": {"Component", "Contract", "Integration", "UI", "Operations"},
    "level": {"Component", "Integration", "E2E"},
    "intent": {
        "Product",
        "Integration",
        "Security",
        "Reliability",
        "UX",
        "Analytics",
        "Operations",
    },
    "release_lanes": {"Smoke", "RC", "Regression", "Nightly", "Long-running"},
    "criticality": {"Critical", "Major", "Normal", "Review"},
}
DEFAULTS = {
    "components": ("BusyBar Firmware",),
    "intent": "Product",
    "surfaces": ("BusyBar",),
    "hardware": ("BusyBar",),
    "release_lanes": ("Regression", "Nightly"),
    "criticality": "Normal",
}


def case_metadata(item):
    """Validate and merge declarations without changing markers or node IDs."""
    markers = list(item.iter_markers("bsb_metadata"))
    if not markers:
        return None
    metadata = dict(DEFAULTS)
    for marker in reversed(markers):
        if marker.args:
            raise ValueError("bsb_metadata accepts named fields only")
        unknown = set(marker.kwargs) - FIELD_LABELS.keys()
        if unknown:
            raise ValueError(
                "Unknown bsb_metadata fields: " + ", ".join(sorted(unknown))
            )
        for field, value in marker.kwargs.items():
            if field in MULTIPLE:
                if not isinstance(value, (tuple, list)) or not value:
                    raise ValueError(field + " must be a nonempty tuple or list")
                values = value
            else:
                values = (value,)
            for text in values:
                if not isinstance(text, str) or not text or text != text.strip():
                    raise ValueError(
                        field
                        + " requires nonempty strings without surrounding whitespace"
                    )
                if field in CHOICES and text not in CHOICES[field]:
                    raise ValueError("Unknown " + field + ": " + text)
            if len(set(values)) != len(values):
                raise ValueError("Duplicate values in " + field)
        metadata.update(marker.kwargs)
    missing = {"suite", "level", "interfaces", "transports"} - metadata.keys()
    if missing:
        raise ValueError("bsb_metadata requires: " + ", ".join(sorted(missing)))
    if item.get_closest_marker("long_running"):
        metadata["release_lanes"] = tuple(
            dict.fromkeys((*metadata["release_lanes"], "Long-running"))
        )
    return metadata


def case_labels(metadata):
    """Emit one label per value; execution has an exact repository source."""
    for field, value in metadata.items():
        for text in value if field in MULTIPLE else (value,):
            yield FIELD_LABELS[field], text
    yield "execution", "Automated"
    yield "automation_source", "bsb-firmware pytest"
