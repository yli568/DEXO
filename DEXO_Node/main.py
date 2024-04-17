import asyncio
from server import Server

def main():
    host = 'localhost'
    port =   10001
    node_id = 0
    server = Server(host, port, node_id)
    asyncio.run(server.run_server())

if __name__ == "__main__":
    main()
