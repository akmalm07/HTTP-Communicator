from flask import Flask

app = Flask(__name__)

@app.route('/')
def hello():
    return 'Hello'

app.run(
    host="0.0.0.0",  # or "127.0.0.1"
    port=8080,
    debug=True       # enables live reload + error messages
)