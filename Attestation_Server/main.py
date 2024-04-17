from flask import Flask, request, jsonify
import hashlib

app = Flask(__name__)

runtime_environments = ["env0", "env1", "env2", "env3", "env4", "env5", "env6", "env7", "env8", "env9"]

@app.route('/verify', methods=['POST'])
def verify_shares():
    data = request.get_json()
    all_valid = True

    for item in data['data_shares']:
        share = item['share']
        signature = item['signature']
        user_id = item['user_id']

        rte = runtime_environments[user_id]

        combined_data = f"{share}-{rte}"
        hash_result = hashlib.sha256(combined_data.encode()).hexdigest()

        if hash_result != signature:
            all_valid = False
            break

    return jsonify({'status': all_valid})

if __name__ == '__main__':
    app.run(debug=True, port=8000)
