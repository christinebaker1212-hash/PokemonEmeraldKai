# usage: lagproxy.py LISTEN_PORT TARGET_PORT DELAY_MS [JITTER_MS]
# TCP proxy that holds every chunk DELAY_MS (+ random jitter, order kept) in each direction.
import asyncio, sys, time, random
LP, TP, D = int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3]) / 1000.0
J = (int(sys.argv[4]) / 1000.0) if len(sys.argv) > 4 else 0.0

async def pipe(reader, writer):
    q = asyncio.Queue()
    async def rd():
        last = 0.0
        while True:
            data = await reader.read(65536)
            due = max(time.monotonic() + D + random.random() * J, last)
            last = due
            await q.put((due, data))
            if not data:
                return
    async def wr():
        while True:
            due, data = await q.get()
            dt = due - time.monotonic()
            if dt > 0:
                await asyncio.sleep(dt)
            if not data:
                writer.close()
                return
            writer.write(data)
            await writer.drain()
    await asyncio.gather(rd(), wr(), return_exceptions=True)

async def handle(cr, cw):
    sr, sw = await asyncio.open_connection('127.0.0.1', TP)
    await asyncio.gather(pipe(cr, sw), pipe(sr, cw), return_exceptions=True)

async def main():
    srv = await asyncio.start_server(handle, '127.0.0.1', LP)
    async with srv:
        await srv.serve_forever()
asyncio.run(main())
