import enum
import json
from pathlib import Path
from typing import List

"""
btmeshconf_generation_log.json example
{
  "result_code": "0",
  "result_message": "",
  "logs": [
    {
      "code": 1,
      "type": "error",
      "message": "Short, essence of the the (error) message",
      "description": "Detailed explanation, if necessary",
      "traceback": [
        "See the formatting in the file and on the UI as well",
        "traceback",
        "             lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum",
        "      lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum lorem ipsum"
      ]
    }
  ],
  "validation_markers": [
    {
      "level": "warning",
      "message": "xyz model corresponds to asd but asd is not instantiated"
    },
    {
      "level": "error",
      "message": "Something something very bad"
    }
  ]
}
"""

# Encoder for enum
class GeneratorEncoder(json.JSONEncoder):
    def default(self, o):
        return o.__dict__

class ErrorLevel(str, enum.Enum):
    # Use strings for JSON encode
    ERROR = "error"
    WARN = "warning"
    INFO = "info"
    
    # Use int for determining the maximum error severity
    def __int__(self) -> int:
        match self:
            case ErrorLevel.ERROR:
                return 1
            case ErrorLevel.WARN:
                return 2
            case ErrorLevel.INFO:
                return 3

class GeneratorLog:
    def __init__(self,
                 code: int = 0,
                 type: ErrorLevel = ErrorLevel.INFO,
                 message: str = "",
                 description: str = "",
                 traceback: List[str] = []):
        self.code = code
        self.type = type
        self.message = message
        self.description = description
        self.traceback = traceback

class ValidationMarkers:
    def __init__(self,
                 level: ErrorLevel = ErrorLevel.INFO,
                 message: str = ""):
        self.level = level
        self.message = message

class LogObject:
    def __init__(self,
                 result_code: int = 0,
                 result_message: str = "",
                 logs: List[GeneratorLog] = [],
                 validation_markers: List[ValidationMarkers] = []):
        self.result_code = result_code
        self.result_message = result_message
        self.logs = logs
        self.validation_markers = validation_markers

class GeneratorLogger:
    result_code = 0
    result_message = ""
    logs = []
    validation_markers = []

    @classmethod
    def add_log(cls,
                code: int,
                type: ErrorLevel,
                message: str,
                description: str,
                traceback: List[str]) -> None:
        cls.logs.append(GeneratorLog(code, type, message, description, traceback))
        cls.calculate_result()
    
    @classmethod
    def add_marker(cls,
                   level: ErrorLevel,
                   message: str) -> None:
        cls.validation_markers.append(ValidationMarkers(level, message))
        cls.calculate_result()

    @classmethod
    def calculate_result(cls):
        cls.result_code = 0
        cls.result_message = "Generation successful"
        
        # The most severe error level will be the overall result code
        # If no logs or validation markers exist, the generation is considered successful.
        for log in cls.logs:
            if cls.result_code == 0 or int(log.type) < cls.result_code:
                if log.type != ErrorLevel.INFO:
                  # Ignore INFO log, it's not considered a failure
                  cls.result_code = int(log.type)

        for marker in cls.validation_markers:
            if cls.result_code == 0 or int(marker.level) < cls.result_code:
                cls.result_code = int(marker.level)

        if cls.result_code != 0:
            cls.result_message = "Generation failed"

    @classmethod
    def write_logs(cls, output_dir: Path) -> None:
        if len(cls.logs) == 0 and len(cls.validation_markers) == 0:
            cls.add_log(0, ErrorLevel.INFO, "Generation successful", "", [])
        json_log = LogObject(cls.result_code, cls.result_message, cls.logs, cls.validation_markers)
        json_dump = json.dumps(json_log, indent=2, cls=GeneratorEncoder)
        log_path = output_dir / "btmeshconf_generation_log.json"
        with log_path.open("w") as f:
            f.write(json_dump)
            f.close()
    
    # Utility functions for testing
    @classmethod
    def reset(cls) -> None:
        cls.result_code = 0
        cls.result_message = ""
        cls.logs = []
        cls.validation_markers = []
