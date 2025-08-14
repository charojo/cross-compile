"""Simplified stand-ins for generated protobuf classes.

These dataclasses provide SerializeToString and ParseFromString methods
compatible with the worker's expectations without requiring the protobuf
runtime. The wire format is JSON-encoded bytes which is sufficient for unit
tests.
"""
from __future__ import annotations
from dataclasses import dataclass
import json

@dataclass
class GetUserRequest:
    user_id: int = 0

    def SerializeToString(self) -> bytes:  # pragma: no cover - trivial
        return json.dumps({"user_id": self.user_id}).encode()

    def ParseFromString(self, data: bytes) -> "GetUserRequest":
        obj = json.loads(data.decode())
        self.user_id = obj.get("user_id", 0)
        return self

@dataclass
class UpdateUserRequest:
    user_id: int = 0
    field: str = ""
    value: str = ""

    def SerializeToString(self) -> bytes:  # pragma: no cover - trivial
        return json.dumps({
            "user_id": self.user_id,
            "field": self.field,
            "value": self.value,
        }).encode()

    def ParseFromString(self, data: bytes) -> "UpdateUserRequest":
        obj = json.loads(data.decode())
        self.user_id = obj.get("user_id", 0)
        self.field = obj.get("field", "")
        self.value = obj.get("value", "")
        return self

@dataclass
class UpdateUserResponse:
    success: bool = False
    message: str = ""

    def SerializeToString(self) -> bytes:  # pragma: no cover - trivial
        return json.dumps({"success": self.success, "message": self.message}).encode()

    def ParseFromString(self, data: bytes) -> "UpdateUserResponse":
        obj = json.loads(data.decode())
        self.success = obj.get("success", False)
        self.message = obj.get("message", "")
        return self

@dataclass
class UpdateUserEvent:
    user_id: int = 0
    field: str = ""
    value: str = ""

    def SerializeToString(self) -> bytes:  # pragma: no cover - trivial
        return json.dumps({
            "user_id": self.user_id,
            "field": self.field,
            "value": self.value,
        }).encode()

    def ParseFromString(self, data: bytes) -> "UpdateUserEvent":
        obj = json.loads(data.decode())
        self.user_id = obj.get("user_id", 0)
        self.field = obj.get("field", "")
        self.value = obj.get("value", "")
        return self
