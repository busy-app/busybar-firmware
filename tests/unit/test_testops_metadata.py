"""Offline checks for metadata inheritance and the TestOps label contract."""

import unittest

import pytest

from utils.testops_metadata import DEFAULTS, case_labels, case_metadata


class Item:
    def __init__(self, *markers, long_running=False):
        self.markers = markers
        self.long_running = long_running

    def iter_markers(self, name):
        return iter(self.markers)

    def get_closest_marker(self, name):
        return self.long_running if name == "long_running" else None


def declaration(**fields):
    return pytest.mark.bsb_metadata(**fields).mark


BASE = declaration(
    suite="Contract",
    level="Component",
    interfaces=("Device API",),
    transports=("LAN REST",),
)


class TestOpsMetadataTests(unittest.TestCase):
    def test_nearest_declaration_replaces_fields_and_whole_tuples(self):
        item = Item(
            declaration(transports=("BLE",)),
            declaration(level="Integration", transports=("MQTT", "LAN REST")),
            BASE,
        )
        result = case_metadata(item)
        self.assertEqual(result["level"], "Integration")
        self.assertEqual(result["transports"], ("BLE",))
        self.assertEqual(result["interfaces"], ("Device API",))
        self.assertEqual(DEFAULTS["release_lanes"], ("Regression", "Nightly"))

    def test_unannotated_support_tests_do_not_claim_product_coverage(self):
        self.assertIsNone(case_metadata(Item()))

    def test_invalid_declarations_fail_even_when_overridden(self):
        invalid = [
            declaration(unknown="value"),
            declaration(transports="MQTT"),
            declaration(transports=()),
            declaration(transports=("MQTT", "MQTT")),
            declaration(interfaces=(" Device API",)),
            declaration(level="UI"),
            declaration(intent="Business"),
            declaration(criticality="Core"),
            pytest.mark.bsb_metadata("Component").mark,
        ]
        for marker in invalid:
            with self.subTest(marker=marker), self.assertRaises(ValueError):
                case_metadata(Item(BASE, marker))
        with self.assertRaises(ValueError):
            case_metadata(Item(declaration(suite="Contract")))

    def test_long_running_lane_extends_declared_release_lanes(self):
        result = case_metadata(Item(BASE, long_running=True))
        self.assertEqual(
            result["release_lanes"], ("Regression", "Nightly", "Long-running")
        )

    def test_labels_have_exact_automation_source_and_no_invented_id(self):
        labels = list(case_labels(case_metadata(Item(BASE))))
        self.assertIn(("execution", "Automated"), labels)
        self.assertIn(("automation_source", "bsb-firmware pytest"), labels)
        self.assertEqual(
            [value for key, value in labels if key == "suite"], ["Contract"]
        )
        self.assertNotIn("ALLURE_ID", {key for key, value in labels})


if __name__ == "__main__":
    unittest.main()
