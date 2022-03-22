https://github.com/pyenv/pyenv
git clone https://github.com/pyenv/pyenv.git ~/.pyenv
https://github.com/pyenv/pyenv-virtualenv

Step 1: Create pyenv
Step 2: Install 3.8.12
Step 3: Activate virtual environment of 3.8.12
Step 4: Install poetry
Step 5: cd to source directory/
Step 6: poetry install

Run 'poetry update' to update python dependencies.

Use the following command to run the application.
$ gunicorn -k geventwebsocket.gunicorn.workers.GeventWebSocketWorker -w 1 -b localhost:5000 app:app_

Refer to https://flask-socketio.readthedocs.io/en/latest/deployment.html
