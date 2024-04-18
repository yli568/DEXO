import json
import socket
from web3 import Web3
from cryptography.fernet import Fernet

# Global variables to store seller nodes and price
seller_nodes = []
price = 0

def get_seller_initialized_info(contract):
    global seller_nodes, price
    event_filter = contract.events.SellerInitialized.createFilter(fromBlock='latest')
    events = event_filter.get_all_entries()
    if events:
        event_args = events[0]['args']
        seller_nodes = event_args['sellerNodes']
        price = event_args['price']

def wait_for_key_revealed(contract, node_id):
    event_filter = contract.events.KeyRevealed.createFilter(fromBlock='latest')
    while True:
        events = event_filter.get_new_entries()
        for event in events:
            if event['args']['sellerNode'] == seller_nodes[node_id]:
                return event['args']['key']
        time.sleep(1)  # Wait before checking again

def merkle_tree_hash(data):
    from web3.auto import w3
    layer = [w3.keccak(text=item).hex() for item in data]
    while len(layer) > 1:
        if len(layer) % 2 == 1:
            layer.append(layer[-1])
        layer = [w3.keccak(hexstr=layer[i] + layer[i+1]).hex() for i in range(0, len(layer), 2)]
    return layer[0]

def decrypt(token, cipher_suite):
    return cipher_suite.decrypt(token.encode()).decode()

def receive_data(ip, port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind((ip, port))
    s.listen()
    conn, addr = s.accept()
    data_fragments = []
    while True:
        data = conn.recv(1024)
        if not data:
            break
        data_fragments.append(data.decode())
    conn.close()
    s.close()
    return json.loads(''.join(data_fragments))

def main():
    web3 = Web3(Web3.HTTPProvider('http://127.0.0.1:8545'))
    contract_address = 'YOUR_CONTRACT_ADDRESS'
    abi = json.loads('YOUR_CONTRACT_ABI')
    contract = web3.eth.contract(address=contract_address, abi=abi)

    get_seller_initialized_info(contract)

    my_ip = 'localhost'
    my_port = 10002
    contract.functions.buyerClaim(my_ip, my_port).transact({'from': web3.eth.accounts[0]})

    print("Waiting to receive data...")
    data = receive_data(my_ip, my_port)
    print("Data received from seller nodes:", data)

    for item in data:
        node_id = item['node_id']
        node_data = item['data']
        r_z_value = contract.functions.r_z(node_id).call()
        is_valid = r_z_value == merkle_tree_hash(node_data)
        print(f"Merkle tree hash validation for node {node_id}:", is_valid)

        if is_valid:
            seller_node_address = seller_nodes[node_id]
            contract.functions.accept(seller_node_address).transact({
                'from': web3.eth.accounts[0],
                'value': price
            })
            print(f"Payment of {price} transferred to node {node_id}. Waiting for key reveal...")

            # Wait for key reveal
            key = wait_for_key_revealed(contract, node_id)
            cipher_suite = Fernet(key)
            decrypted_data = decrypt(node_data, cipher_suite)
            print(f"Decrypted data from node {node_id}:", decrypted_data)

if __name__ == "__main__":
    main()
