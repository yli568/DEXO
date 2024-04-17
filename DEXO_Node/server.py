





import aiohttp
import asyncio
from datetime import datetime
from web3 import Web3
from aiohttp import web
from cryptography.fernet import Fernet

class Server:
    def __init__(self, host, port, node_id):
        self.host = host
        self.port = port
        self.node_id = node_id
    
    async def run_server(self):
        server = await asyncio.start_server(
            self.handle_client, self.host, self.port)

        addr = server.sockets[0].getsockname()
        print(f'Serving on {addr}')

        # Handle_client will serve every client(P-DApp server)
        async with server:
            await server.serve_forever()



    Receive data from P-DApp server
    async def handle_client(self, reader, writer):
        data_fragments = []

        while True:
            data = await reader.read(1024)
            if not data:
                break
            data_fragments.append(data.decode())

            try:
                message = ''.join(data_fragments)
                json_data = json.loads(message)
                print(f"Received complete JSON data from {writer.get_extra_info('peername')}")
                break
            except json.JSONDecodeError:
                continue

        # Here check if DAppId is valid, or use other identity authentication method, not the focus of this demo

        # Attestation
        is_valid = await self.attestation(json_data)
        if not is_valid:
            print("Attestation failed. Closing connection.")
            writer.close()
            await writer.wait_closed()
            return
        
        #########################################################################
        # After Attestation, prepare data for constructing contract on Ethereum 
        key = Fernet.generate_key()
        cipher_suite = Fernet(key)

        # x, z, and r_z are explained in paper & OptiSwap paper.
        x = [item['share'] for item in json_data['data_shares']]
        z = [encrypt(item, cipher_suite) for item in x]
        r_z = merkle_tree_hash(self.z)
        commitment = Web3.keccak(text=key).hex()

        price = json_data['price']
        auxiliaryInfo = json_data['auxiliaryInfo']
        dataSources = json_data['dataSources']

        w3 = Web3(HTTPProvider('http://localhost:8545')) # Your Ethereum node address
        contract = w3.eth.contract(address="contract_address", abi="contract_abi")
        local_filter = contract.events.PaymentMade.createFilter(fromBlock='latest')
        await self.initialize_contract(w3, contract, r_z, commitment)
        asyncio.create_task(self.monitor_payments(local_filter, z, w3, contract, key))


    
    # Send data to attestation server to get a feedback
    async def attestation(self, json_data):
        url = "http://localhost:8000/verify" # Attestation server address
        async with aiohttp.ClientSession() as session:
            async with session.post(url, json=json_data) as response:
                if response.status == 200:
                    result = await response.json()
                    # Return status, or if no status, return False
                    return result.get('status', False)
                else:
                    print("Failed to connect to attestation server.")
                    return False
    def encrypt(data, cipher_suite):
        return cipher_suite.encrypt(data.encode()).decode()
    def decrypt(token, cipher_suite):
        return cipher_suite.decrypt(token.encode()).decode()
    def merkle_tree_hash(data_list):
        layer = [Web3.keccak(text=item).hex() for item in data_list]
        while len(layer) > 1:
            if len(layer) % 2 == 1:
                layer.append(layer[-1])
            layer = [Web3.keccak(hexstr=layer[i] + layer[i + 1]).hex() for i in range(0, len(layer), 2)]
        return layer[0]

    async def initialize_contract(self, w3, contract, r_z, commitment):
        # Ensure you have the account unlocked or have the private key available for signing
        account = w3.eth.accounts[0]
        private_key = 'YOUR_PRIVATE_KEY'

        # Construct the transaction to call the initialize function with the necessary parameters
        tx = contract.functions.initialize(r_z, commitment).buildTransaction({
            'from': account,
            'nonce': w3.eth.getTransactionCount(account),
            # Add additional transaction parameters as needed (e.g., gas)
        })

        # Sign the transaction
        signed_tx = w3.eth.account.signTransaction(tx, private_key)
        # Send the transaction
        tx_hash = w3.eth.sendRawTransaction(signed_tx.rawTransaction)
        # Wait for the transaction to be mined
        tx_receipt = w3.eth.wait_for_transaction_receipt(tx_hash)
        print(f"Initialization transaction successful with receipt: {tx_receipt}")

    async def monitor_payments(self, local_filter, z, w3, contract, key):
        while True:
            for event in local_filter.get_new_entries():
                buyer_ip = event['args']['buyerIp']
                buyer_port = event['args']['buyerPort']
                # Assuming the buyer's address is available in the event logs(designed in the contract)
                await self.send_data_to_buyer(buyer_ip,buyer_port, z)
                await self.reveal_key(w3, contract, key)

    # async def send_data_to_buyer(self, buyer_ip, buyer_port, data):
    #     json_data = json.dumps({"data": data}).encode('utf-8')
    #     reader, writer = await asyncio.open_connection(buyer_ip, buyer_port)
    #     writer.write(json_data)
    #     await writer.drain()
    #     writer.close()
    #     await writer.wait_closed()
    #     print(f"Sent JSON data to {buyer_ip}:{buyer_port}")
    async def send_data_to_buyer(self, buyer_ip, buyer_port, data):
        # json_data = json.dumps(data).encode('utf-8')
        # reader, writer = await asyncio.open_connection(buyer_ip, buyer_port)
        # writer.write(json_data)
        # await writer.drain()
        # writer.close()
        # await writer.wait_closed()
        # print(f"Sent JSON data to {buyer_ip}:{buyer_port}")
        packaged_data = {
            'node_id': self.node_id,
            'data': data
        }
        reader, writer = await asyncio.open_connection(buyer_ip, buyer_port)
        writer.write(json_data)
        await writer.drain()
        writer.close()
        await writer.wait_closed()
        print(f"Sent JSON data to {buyer_ip}:{buyer_port}")


    async def reveal_key(self, w3, contract, key):
        account = w3.eth.accounts[0]
        private_key = 'YOUR_PRIVATE_KEY'  # This should be securely managed
        tx = contract.functions.revealKey(key).buildTransaction({
            'from': account,
            'nonce': w3.eth.getTransactionCount(account),
            # Specify the gas limit for the transaction; adjust the value as needed
            # 'gas': 1000000,
        })

        signed_tx = w3.eth.account.signTransaction(tx, private_key)
        tx_hash = w3.eth.sendRawTransaction(signed_tx.rawTransaction)
        tx_receipt = w3.eth.wait_for_transaction_receipt(tx_hash)

        print(f"Key revealed with transaction receipt: {tx_receipt}")