from flask import Flask, request

app = Flask(__name__)

@app.route('/')
def hello():
    return 'Hello'

@app.route('/upload', methods=['POST'])
def upload():
    raw_data = request.get_data().decode('utf-8') # Read raw POST body as plain text
    print(f"Received Plain Text: {raw_data}")
    return 'Plain Text Received!'

# Removed duplicate '/' route (thanks)
# Alternatively, you can change its route if needed:
# @app.route('/thanks')
# def thanks():
#     return 'Thanks!'

if __name__ == '__main__':
    app.run(
        host="0.0.0.0",  # or "127.0.0.1"
        port=8080,
        debug=True       # enables live reload + error messages
    )