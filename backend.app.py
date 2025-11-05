from flask import Flask, request, jsonify
from flask_cors import CORS
from flask_socketio import SocketIO, emit
from auth import auth_bp, jwt
from game_logic import game_bp
from database import db, User, PlayerStats, Match
import eventlet
import os

app = Flask(__name__)
app.config['SECRET_KEY'] = 'f66_football_secret_key'
app.config['SQLALCHEMY_DATABASE_URI'] = 'mysql+mysqlconnector://username:password@localhost/f66_football'
app.config['JWT_SECRET_KEY'] = 'jwt_f66_secret_key'

# Initialize extensions
db.init_app(app)
jwt.init_app(app)
CORS(app)
socketio = SocketIO(app, cors_allowed_origins="*", async_mode='eventlet')

# Register blueprints
app.register_blueprint(auth_bp, url_prefix='/auth')
app.register_blueprint(game_bp, url_prefix='/game')

# Real-time game events
@socketio.on('connect')
def handle_connect():
    print('Client connected')
    emit('connected', {'message': 'Connected to F66 Game Server'})

@socketio.on('join_match')
def handle_join_match(data):
    match_id = data['match_id']
    user_id = data['user_id']
    # Add player to match room
    emit('player_joined', {'user_id': user_id}, room=match_id, broadcast=True)

@socketio.on('player_move')
def handle_player_move(data):
    match_id = data['match_id']
    player_data = data['player_data']
    # Broadcast player movement to all clients in match
    emit('player_update', player_data, room=match_id, broadcast=True)

@socketio.on('kick_ball')
def handle_kick_ball(data):
    match_id = data['match_id']
    ball_data = data['ball_data']
    # Update ball physics and broadcast
    emit('ball_update', ball_data, room=match_id, broadcast=True)

@socketio.on('disconnect')
def handle_disconnect():
    print('Client disconnected')

if __name__ == '__main__':
    with app.app_context():
        db.create_all()
    socketio.run(app, debug=True, host='0.0.0.0', port=5000)