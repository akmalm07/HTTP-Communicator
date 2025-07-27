from flask import Flask, request, make_response

app = Flask(__name__)

@app.route('/')
def hello():
    return 'Hello'

@app.route('/upload', methods=['POST'])
def upload():
    raw_data = request.get_data().decode('utf-8') # Read raw POST body as plain text
    print(f"Received Plain Text: {raw_data}")
    return 'Plain Text Received!'

@app.route('/test', methods=['POST'])
def test():
    data = request.json
    print(f"Received JSON: {data}")
    response = make_response('Hello, Keep-Alive!')
    response.headers['Connection'] = 'keep-alive'  # Optional, Flask defaults to this in HTTP/1.1
    return response

if __name__ == '__main__':
    app.run(
        host="0.0.0.0",  # or "127.0.0.1"
        port=8080,
        debug=True       # enables live reload + error messages
    )
