from flask import Blueprint, request, jsonify
from flask_jwt_extended import jwt_required, get_jwt_identity
from database import db, Match, PlayerStats
import json

game_bp = Blueprint('game', __name__)

class FootballGame:
    def __init__(self):
        self.players = {}
        self.ball_position = [0, 0, 0]  # x, y, z
        self.score = [0, 0]  # team1, team2
        self.time_elapsed = 0
        self.is_active = True
    
    def update_player_position(self, player_id, position):
        self.players[player_id] = position
    
    def kick_ball(self, force, direction):
        # Simplified ball physics
        self.ball_position[0] += force * direction[0]
        self.ball_position[1] += force * direction[1]
        self.ball_position[2] += force * direction[2]
        
        # Check for goals
        self.check_goal()
    
    def check_goal(self):
        # Simple goal detection
        if (-50 < self.ball_position[0] < 50 and 
            self.ball_position[1] > 70 and 
            -10 < self.ball_position[2] < 10):
            self.score[1] += 1  # Team 2 scores
        elif (-50 < self.ball_position[0] < 50 and 
              self.ball_position[1] < -70 and 
              -10 < self.ball_position[2] < 10):
            self.score[0] += 1  # Team 1 scores

@game_bp.route('/create_match', methods=['POST'])
@jwt_required()
def create_match():
    try:
        user_id = get_jwt_identity()
        game = FootballGame()
        
        new_match = Match(
            player1_id=user_id,
            game_state=json.dumps({
                'players': game.players,
                'ball_position': game.ball_position,
                'score': game.score,
                'time_elapsed': game.time_elapsed
            })
        )
        
        db.session.add(new_match)
        db.session.commit()
        
        return jsonify({
            'match_id': new_match.id,
            'message': 'Match created successfully'
        }), 201
        
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@game_bp.route('/join_match/<int:match_id>', methods=['POST'])
@jwt_required()
def join_match(match_id):
    try:
        user_id = get_jwt_identity()
        match = Match.query.get(match_id)
        
        if not match:
            return jsonify({'error': 'Match not found'}), 404
        
        if match.player2_id is None:
            match.player2_id = user_id
            db.session.commit()
            
        return jsonify({
            'match_id': match.id,
            'message': 'Joined match successfully'
        }), 200
        
    except Exception as e:
        return jsonify({'error': str(e)}), 500