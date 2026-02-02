import socket


class Stream:
    def __init__(self, portname: tuple[str, int]):
        self.address = portname[0]
        self.port = portname[1]
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.transmitted = 0
        self.received = 0

    def open(self):
        self.socket.connect((self.address, self.port))

    def close(self):
        self.socket.close()

    def write(self, data: bytes):
        self.transmitted += len(data)
        self.socket.sendall(data)

    def try_read(self, size: int):
        data = self.socket.recv(size)
        self.received += len(data)
        return data

    def read(self, size: int):
        data = self.try_read(size)
        while len(data) < size:
            data += self.try_read(size - len(data))
        return data

    @property
    def in_waiting(self):
        return 1


class BufferedRead:
    def __init__(self, stream: Stream):
        self.buffer = bytearray()
        self.stream = stream

    def until(self, eol: str = "\n", cut_eol: bool = True):
        eol_bytes = eol.encode("ascii")
        while True:
            # search in buffer
            i = self.buffer.find(eol_bytes)
            if i >= 0:
                if cut_eol:
                    read = self.buffer[:i]
                else:
                    read = self.buffer[: i + len(eol_bytes)]
                self.buffer = self.buffer[i + len(eol_bytes) :]
                return read

            # read and append to buffer
            i = max(1, self.stream.in_waiting)
            data = self.stream.read(i)
            self.buffer.extend(data)


class Cli:
    CLI_PROMPT = ">: "
    CLI_EOL = "\r\n"

    def __init__(self, portname: tuple[str, int]):
        self.port = Stream(portname)
        self.read = BufferedRead(self.port)

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.stop()

    def start(self):
        self.port.open()
        self.read.until(self.CLI_PROMPT)
        # Send a command with a known syntax to make sure the buffer is flushed
        self.send("uptime\r")
        self.read.until("Uptime: ")
        # And read buffer until we get prompt
        self.read.until(self.CLI_PROMPT)

    def stop(self) -> None:
        self.port.close()

    def send(self, line: str) -> None:
        self.port.write(line.encode("ascii"))

    def send_and_wait_eol(self, line: str):
        self.send(line)
        return self.read.until(self.CLI_EOL)

    def send_and_wait_prompt(self, line: str):
        self.send(line)
        return self.read.until(self.CLI_PROMPT)
