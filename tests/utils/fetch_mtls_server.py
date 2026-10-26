"""Local HTTPS server requiring a client certificate for Fetch tests."""

from __future__ import annotations

import queue
import ssl
from pathlib import Path

from utils.fetch_http_server import FetchHTTPServer, FetchRequestHandler


class FetchMTLSServer(FetchHTTPServer):
    """Fetch test server with TLS client-certificate verification enabled."""

    def __init__(
        self,
        server_address,
        *,
        server_certificate_path: Path,
        server_private_key_path: Path,
        client_ca_pem: str,
        allow_partial_chain: bool = False,
        tls_version: str | None = None,
    ):
        super().__init__(server_address, FetchRequestHandler)

        context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        context.minimum_version = ssl.TLSVersion.TLSv1_2
        if tls_version is not None:
            versions = {
                "TLSv1.2": ssl.TLSVersion.TLSv1_2,
                "TLSv1.3": ssl.TLSVersion.TLSv1_3,
            }
            selected_version = versions[tls_version]
            context.minimum_version = selected_version
            context.maximum_version = selected_version
        context.load_cert_chain(server_certificate_path, server_private_key_path)
        context.load_verify_locations(cadata=client_ca_pem)
        context.verify_mode = ssl.CERT_REQUIRED
        if allow_partial_chain:
            context.verify_flags |= ssl.VERIFY_X509_PARTIAL_CHAIN
        self._tls_context = context
        self.tls_errors = queue.Queue()

    def process_request_thread(self, request, client_address):
        """Perform each TLS handshake in its request worker, not the accept loop."""
        request.settimeout(10)
        try:
            tls_request = self._tls_context.wrap_socket(request, server_side=True)
        except (OSError, ssl.SSLError) as error:
            self.tls_errors.put(error)
            request.close()
            return

        tls_request.settimeout(15)
        super().process_request_thread(tls_request, client_address)

    def url(self, path):
        host, port = self.server_address[:2]
        return f"https://{host}:{port}{path}"
