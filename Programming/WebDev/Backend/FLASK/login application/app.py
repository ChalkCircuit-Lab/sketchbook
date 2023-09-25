from flask import Flask
from flask import request
from markupsafe import escape
from flask import render_template

app = Flask(__name__)


@app.route('/login/<username>')
def login(username):
    if request.method == 'POST':
        return 'post'
        return do_the_login(escape(username))
    else:
        return show_the_login_form(escape(username))

def show_the_login_form(username):
    return f'User tries to login: {username}'

@app.route('/')
def index():
    return 'Index Page'

@app.route('/hello/')
@app.route('/hello/<name>')
def hello(name=None):
    return render_template('hello.html', name=name)

@app.route('/user/<username>')
def show_user_profile(username):
    # show the user profile for that user
    return f'User {escape(username)}'

@app.route('/post/<int:post_id>')
def show_post(post_id):
    # show the post with the given id, the id is an integer
    return f'Post {post_id}'

@app.route('/path/<path:subpath>')
def show_subpath(subpath):
    # show the subpath after /path/
    return f'Subpath {escape(subpath)}'



