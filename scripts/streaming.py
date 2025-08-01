#!/usr/bin/env python3

import pygame
import websocket
import time
import json
from flipper.app import App


class Main(App):

    def init(self):
        self.parser.add_argument(
            "display", help="Required display", choices=["front", "back"]
        )
        self.parser.add_argument(
            "--timeout",
            help="Ping timeout value in seconds",
            default=5,
            type=int,
            required=False,
        )
        self.parser.set_defaults(func=self.main)

    @staticmethod
    def back_convert_b4_to_b8(data):
        i = 0
        j = 0
        b8_bytes = bytearray(len(data) * 2)
        while i < len(data):
            px1 = data[i] & 0x0F
            px2 = (data[i] >> 4) & 0x0F
            b8_bytes[j] = px1
            b8_bytes[j + 1] = px2
            i += 1
            j += 2
        return b8_bytes

    @staticmethod
    def rle_decompress(data: bytes, blksize: int) -> bytearray:
        index = 0
        data_len = len(data)
        decompressed = bytearray()

        while index < data_len:
            ctrl_byte = data[index]
            index += 1

            if int(ctrl_byte) & 0x80:
                # Unique blocks: ctrl_byte & 0x7F = unique sequence length
                count = ctrl_byte & 0x7F
                block_data = data[index : index + count * blksize]
                decompressed.extend(block_data)
                index += count * blksize
            else:
                # Repeated block: ctrl_byte = repeat count
                count = ctrl_byte
                block = data[index : index + blksize]
                decompressed.extend(block * count)
                index += blksize
        return decompressed

    @staticmethod
    def manual_rescale(value, original_min, original_max, new_min, new_max):
        return ((value - original_min) / (original_max - original_min)) * (
            new_max - new_min
        ) + new_min

    @staticmethod
    def back_color_palette_generate():
        palette = list()
        for x in range(0, 16):
            value = Main.manual_rescale(x, 0, 16, 0, 255)
            palette.append((value, value, value))
        return palette

    def prepare_screen(self, display_index):
        if display_index == 0:
            self.original_width = 72
            self.original_height = 16
            self.scale = 10
            self.image_mode = "BGR"
            self.blk_size = 3
        else:
            self.original_width = 160
            self.original_height = 80
            self.scale = 5
            self.back_color_palette = Main.back_color_palette_generate()
            self.image_mode = "P"
            self.blk_size = 2
        self.size = self.width, self.height = (
            self.original_width * self.scale,
            self.original_height * self.scale,
        )
        self._display_surf = pygame.display.set_mode(
            self.size, pygame.HWSURFACE | pygame.DOUBLEBUF
        )

    def switch_display(self, display_index):
        self.prepare_screen(display_index)

        display_json = {"display": display_index}

        self.ws.send_text(json.dumps(display_json))
        self.ws.recv()
        self.display_index = display_index
        self.new_display_index = display_index

    def on_init(self):
        pygame.init()
        self._running = True
        self.ws = websocket.WebSocket()
        self.ws.connect("ws://10.0.4.20/api/screen/ws")
        if self.args.display == "front":
            self.switch_display(0)
        else:
            self.switch_display(1)
        self.time = time.time()

    def get_frame(self):
        compressed = self.ws.recv()
        try:
            decompressed = Main.rle_decompress(compressed, self.blk_size)

            if self.display_index == 0:
                self.data = decompressed
            else:
                self.data = self.back_convert_b4_to_b8(decompressed)

            self.logger.debug(f"RLE: {len(compressed)}, NonRLE: {len(self.data)}")
        except:
            self.data.clear()
            self.logger.warning("Unable to decompress, skip frame")

    def try_ping(self):
        current_time = time.time()
        if current_time - self.time > self.args.timeout:
            self.logger.debug("Ping")
            self.ws.ping()
            self.time = current_time

    def on_loop(self):
        if self.new_display_index != self.display_index:
            self.switch_display(self.new_display_index)

        self.get_frame()
        self.try_ping()

    def on_event(self, event):
        if event.type == pygame.QUIT:
            self._running = False
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_SPACE:
                self.new_display_index = self.new_display_index ^ 1

    def on_render(self):
        if len(self.data) > 0:
            surf = pygame.image.frombuffer(
                self.data, [self.original_width, self.original_height], self.image_mode
            )

            if self.display_index == 1:
                surf.set_palette(self.back_color_palette)

            surf = pygame.transform.scale_by(surf, self.scale)
            self._display_surf.blit(surf, [0, 0])
            pygame.display.flip()

    def on_cleanup(self):
        pygame.quit()

    def main(self):
        if self.on_init() == False:
            self._running = False

        while self._running:
            for event in pygame.event.get():
                self.on_event(event)
            self.on_loop()
            self.on_render()
        self.on_cleanup()
        return 0


if __name__ == "__main__":
    Main()()
