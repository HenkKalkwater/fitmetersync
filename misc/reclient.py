#!/usr/bin/env python3
import argparse
import asyncio
import logging
import struct

LOGGER = logging.getLogger(__name__)

class Client:
    def __init__(self, reader, writer):
        self._reader: asyncio.StreamReader = reader
        self._writer: asyncio.StreamWriter = writer
        self._inqueue = asyncio.Queue()
        self._outqueue = asyncio.Queue()
        self._incoming_task = asyncio.create_task(self.handle_incoming())
        self._outgoing_task = asyncio.create_task(self.handle_outgoing())

    async def send(self, data: bytes):
        await self._outqueue.put(data)

    async def stop(self):
        self._incoming_task.cancel()
        self._outgoing_task.cancel()
        await asyncio.gather(self._incoming_task, self._outgoing_task)

    async def handle_incoming(self):
        try:
            while True:
                raw_length = await self._reader.readexactly(4)
                length = struct.unpack(">I", raw_length)
                LOGGER.debug(f"Packet of {length} bytes incoming")
                data = await self._reader.readexactly(length)
                LOGGER.debug(data)
        except asyncio.CancelledError:
            print("Reader closed")

    async def handle_outgoing(self):
        try:
            while True:
                item = await self._outqueue.get()
                data_len = len(item)
                data = struct.pack(f">I", data_len)
                data += item
                self._writer.write(data)
                await self._writer.drain()
                self._outqueue.task_done()
        except asyncio.CancelledError:
            await self._writer.drain()
            self._writer.close()
            await self._writer.wait_closed()
            print("Writer closed")


async def main():
    logging.basicConfig(format=logging.BASIC_FORMAT)
    parser = argparse.ArgumentParser()
    parser.add_argument("host", type=str)
    parser.add_argument("port", type=int, nargs="?", default=8476)

    args = parser.parse_args()
    reader, writer = await asyncio.open_connection(host=args.host, 
            port=args.port)

    client = Client(reader, writer)
    await client.send("Hello world".encode("UTF-8"))
    await asyncio.sleep(5)
    await client.stop()

if __name__ == "__main__":
    asyncio.run(main())
