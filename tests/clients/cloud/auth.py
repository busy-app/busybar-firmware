"""
Cloud authentication client using PKCE flow.

Implements the full sign-in flow against busy-cloud-core:
1. Generate code_verifier / code_challenge
2. POST /auth/sign-in → auth_code
3. POST /auth/exchange → session JWT token
"""

from __future__ import annotations

import logging
import secrets
from base64 import urlsafe_b64encode
from hashlib import sha256

import allure
import requests
from pydantic import BaseModel


# === Response Models ===


class SignInSuccess(BaseModel):
    auth_code: str


class SignInResponse(BaseModel):
    success: SignInSuccess


class ExchangeSuccess(BaseModel):
    token: str


class ExchangeResponse(BaseModel):
    success: ExchangeSuccess


# === Auth Client ===


class CloudAuthClient:
    """
    PKCE-based authentication client for busy-cloud-core.

    Usage:
        client = CloudAuthClient("https://cloud.dev.busy.app")
        client.authenticate("user@example.com", "password")
        session = client.get_authenticated_session()
    """

    USER_AGENT = "BSB-AutoTest/1.0"

    def __init__(self, base_url: str, basic_auth: tuple[str, str] | None = None):
        self.base_url = base_url.rstrip("/")
        self.basic_auth = basic_auth
        self.token: str | None = None
        self.logger = logging.getLogger("CloudAuthClient")

    @staticmethod
    def _generate_pkce() -> tuple[str, str]:
        """Generate PKCE code_verifier and code_challenge pair."""
        code_verifier = secrets.token_urlsafe(32)
        digest = sha256(code_verifier.encode("utf-8")).digest()
        code_challenge = urlsafe_b64encode(digest).decode("utf-8").rstrip("=")
        return code_verifier, code_challenge

    def authenticate(self, email: str, password: str) -> str:
        """
        Perform full PKCE authentication flow.

        Returns the session JWT token.
        """
        code_verifier, code_challenge = self._generate_pkce()

        with allure.step("Cloud auth: sign-in"):
            self.logger.info(f"Signing in as {email} to {self.base_url}")
            resp = requests.post(
                f"{self.base_url}/api/v0/auth/sign-in",
                json={
                    "username": email,
                    "password": password,
                    "code_challenge": code_challenge,
                },
                headers={"User-Agent": self.USER_AGENT},
                auth=self.basic_auth,
                timeout=15,
            )
            resp.raise_for_status()
            sign_in = SignInResponse.model_validate(resp.json())
            auth_code = sign_in.success.auth_code
            self.logger.info("Received auth_code")

        with allure.step("Cloud auth: exchange code for token"):
            resp = requests.post(
                f"{self.base_url}/api/v0/auth/exchange",
                json={
                    "auth_code": auth_code,
                    "code_verifier": code_verifier,
                },
                headers={"User-Agent": self.USER_AGENT},
                auth=self.basic_auth,
                timeout=15,
            )
            resp.raise_for_status()
            exchange = ExchangeResponse.model_validate(resp.json())
            self.token = exchange.success.token
            self.logger.info("Authenticated successfully")

        return self.token

    def get_authenticated_session(self) -> requests.Session:
        """Return a requests.Session with Bearer token set."""
        if not self.token:
            raise RuntimeError("Not authenticated. Call authenticate() first.")

        session = requests.Session()
        session.headers.update({
            "Authorization": f"Bearer {self.token}",
            "User-Agent": self.USER_AGENT,
            "Accept": "application/json",
        })
        return session
