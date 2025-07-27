from flask import Flask, request, make_response

app = Flask(__name__)


@app.route('/')
def hello():
    response = make_response('Hello, Keep-Alive!')
    response.headers['Connection'] = 'keep-alive'  # Optional, Flask defaults to this in HTTP/1.1
    return response

@app.route('/upload', methods=['POST'])
def upload():
    raw_data = request.data.decode('utf-8')  # Read raw POST body as plain text
    print(f"Received Plain Text: {raw_data}")
    response = make_response('Hello, Keep-Alive!')
    response.headers['Connection'] = 'keep-alive'  # Optional, Flask defaults to this in HTTP/1.1
    return response


@app.route('/test', methods=['POST'])
def test():
    data = request.json

    print(f"Received JSON: {data}")
    response = make_response('Hello, Keep-Alive!')
    response.headers['Connection'] = 'keep-alive'  # Optional, Flask defaults to this in HTTP/1.1
    return response


app.run(
    host="0.0.0.0",  # or "127.0.0.1"
    port=8080,
    debug=True       # enables live reload + error messages
)