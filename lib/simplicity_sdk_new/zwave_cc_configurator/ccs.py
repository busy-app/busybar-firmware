from typing import Any, Optional
import re
import json
import subprocess
from pathlib import Path
from cc_base import cc_validate


class cc_multilevel_sensor(cc_validate):
    """Command Class Multilevel Sensor

    Raises:
        ValueError: Thrown when an invalid sensor name is used
        ValueError: Thrown when an invalid sensor scale is used
    """
    _RELATIVE_PATH_BASE_ZIPSDK = Path(__file__).parent / '../zwave/'
    _RELATIVE_PATH_BASE_MONOREPO = Path(__file__).parent / '../../'
    _HEADER_PATH_IN_ZWAVE = 'ZAF/CommandClasses/MultilevelSensor/src/CC_MultilevelSensor_SensorHandlerTypes.h'
    _ZIPSDK_PATH = _RELATIVE_PATH_BASE_ZIPSDK / _HEADER_PATH_IN_ZWAVE
    _MONOREPO_PATH = _RELATIVE_PATH_BASE_MONOREPO / _HEADER_PATH_IN_ZWAVE
    
    @classmethod
    def _get_header_file_path(cls) -> Optional[Path]:
        """Get the header file path
        
        Returns:
            Optional[Path]: Path to the header file if it exists, None otherwise
        """
        # Package-based SDK
        try:
            result = subprocess.run(
                ['slt', 'locate', '--non-interactive', '--json', 'zwave'],
                capture_output=True,
                text=True,
                check=True,
                timeout=10
            )
            # Parse JSON output
            packages = json.loads(result.stdout)
            if packages and len(packages) > 0:
                package_path = packages[0].get('path')
                if package_path:
                    header_path_pkg = Path(package_path) / 'ZAF/CommandClasses/MultilevelSensor/src/CC_MultilevelSensor_SensorHandlerTypes.h'
                    if header_path_pkg.exists():
                        return header_path_pkg
        except (subprocess.SubprocessError, json.JSONDecodeError, KeyError, TimeoutError, FileNotFoundError):
            # The calling function will raise an error if the header file cannot be found
            pass
        
        # Zip-SDK (25Q4)
        header_path_zipsdk = cls._ZIPSDK_PATH.resolve()
        if header_path_zipsdk.exists():
            return header_path_zipsdk
        
        # monorepo (25Q2 and earlier)
        header_path_monorepo = cls._MONOREPO_PATH.resolve()
        if header_path_monorepo.exists():
            return header_path_monorepo

        return None

    @classmethod
    def _parse_header_file(cls) -> tuple[list[str], list[str]]:
        """Parse the C header file to extract SENSOR_NAME_* and SENSOR_SCALE_* definitions.
        
        Returns:
            tuple: (list of sensor names, list of sensor scales)
            
        Raises:
            FileNotFoundError: If the header file cannot be found
        """
        header_path = cls._get_header_file_path()
        
        if header_path is None:
            raise FileNotFoundError("Header file not found: Could not locate CC_MultilevelSensor_SensorHandlerTypes.h")
        
        with open(header_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        sensor_names = []
        sensor_scales = []
        
        scale_pattern = r'#define\s+(SENSOR_SCALE_\w+)\s+'
        for match in re.finditer(scale_pattern, content):
            scale_name = match.group(1)
            if scale_name not in sensor_scales:
                sensor_scales.append(scale_name)
        
        enum_pattern = r'typedef\s+enum\s+sensor_name\s*\{([^}]+)\}\s*sensor_name_t;'
        enum_match = re.search(enum_pattern, content, re.DOTALL)
        if enum_match:
            enum_body = enum_match.group(1)
            name_pattern = r'(SENSOR_NAME_\w+)'
            for match in re.finditer(name_pattern, enum_body):
                name = match.group(1)
                # Exclude sentinel value
                if name != 'SENSOR_NAME_MAX_COUNT' and name not in sensor_names:
                    sensor_names.append(name)
        
        return sensor_names, sensor_scales

    @classmethod
    def _get_names(cls) -> list[str]:
        """Get list of valid sensor names from header file."""
        if not hasattr(cls, '_cached_names'):
            names, _ = cls._parse_header_file()
            cls._cached_names = names
        return cls._cached_names

    @classmethod
    def _get_scales(cls) -> list[str]:
        """Get list of valid sensor scales from header file."""
        if not hasattr(cls, '_cached_scales'):
            _, scales = cls._parse_header_file()
            cls._cached_scales = scales
        return cls._cached_scales

    @property
    def _names(self) -> list[str]:
        """Get list of valid sensor names from header file."""
        return self._get_names()

    @property
    def _scales(self) -> list[str]:
        """Get list of valid sensor scales from header file."""
        return self._get_scales()

    def __init__(self) -> None:
        """Constructor
        """
        super().__init__('zw_cc_multilevel_sensor', [
            'cc_multilevel_sensor_config.c.jinja', 'cc_multilevel_sensor_config.h.jinja'], 'endpoints')

    def _validate(self, configuration: dict[str, dict[str, Any]]):
        for endpoint in configuration[self.component][self.variable]:
            for key, value in endpoint["sensors"].items():
                # A key represents one instance but since this is a dictionary
                # we don't need to ensure that it is unique because python would
                # throw an error in case of non unique keys
                if value['name'] not in self._names:
                    raise ValueError(f"Invalid name {value['name']} for sensor for {key}")
                for scale in value['scales']:
                    if scale not in self._scales:
                        raise ValueError(f'Invalid scale {scale} for sensor for {key}')
