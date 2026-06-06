from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS
import subprocess
import json
import os

app = Flask(__name__)
CORS(app)

# serve the web folder through Flask
@app.route('/')
def index():
    return send_from_directory('web', 'index.html')

@app.route('/<path:filename>')
def static_files(filename):
    return send_from_directory('web', filename)

@app.route('/run', methods=['POST'])
def run():
    try:
        data = request.get_json()
        input_content = data['input']

        with open('data/input.txt', 'w') as f:
            f.write(input_content)

        result = subprocess.run(
            ['./routexpert.exe'],
            capture_output=True,
            text=True
        )

        with open('data/output.json', 'r') as f:
            output = json.load(f)

        return jsonify(output)

    except Exception as e:
        return jsonify({'error': str(e)})

if __name__ == '__main__':
    print("Open http://localhost:5000 in your browser")
    app.run(port=5000, debug=False)