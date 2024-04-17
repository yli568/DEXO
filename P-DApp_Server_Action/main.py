import socket
import json

# # Run these codes to construct the smart contract if you did not construct a contract before
# w3 = Web3(Web3.HTTPProvider('http://localhost:8545'))
# assert w3.isConnected()
# contract_abi = 'contract_abi'
# contract_bytecode = '' # the compiled smart contract
# Contract = w3.eth.contract(abi=contract_abi, bytecode=contract_bytecode)
# acct = w3.eth.account.privateKeyToAccount('YOUR_PRIVATE_KEY')
# # put in parameters
# construct_txn = Contract.constructor().buildTransaction({
#     'from': acct.address,
#     'nonce': w3.eth.getTransactionCount(acct.address),
#     'gas': , # gas estimated
#     'gasPrice': # w3.toWei('21', 'gwei') # how much you are willing to pay for gas
#     })
# signed = acct.signTransaction(construct_txn)
# tx_hash = w3.eth.sendRawTransaction(signed.rawTransaction)
# tx_receipt = w3.eth.waitForTransactionReceipt(tx_hash)
# print(f"Contract Deployed At: {tx_receipt.contractAddress}")


# ip of nodes, here we only show the transfer with one DEXO node.
ip = 'localhost'
port = 10001

collected_shares = []

for i in range(10):
    json_file_name = f"user{i}.json"
    try:
        with open(json_file_name, 'r') as file:
            data = json.load(file)
            for share in data['shares']:
                if share['share_id'] == 0:
                    share_info = {
                        'share': share['share'],
                        'signature': share['signature'],
                        'user_id': data['ID']
                    }
                    collected_shares.append(share_info)
                    break 
    except FileNotFoundError:
        print(f"Error: File {json_file_name} not found.")
        continue

output_data = {
    'data_shares': collected_shares
}

json_data = json.dumps(output_data)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((ip, port))
    s.sendall(json_data.encode('utf-8'))

    response = s.recv(1024)
    print(f"Received from server: {response.decode('utf-8')}")
