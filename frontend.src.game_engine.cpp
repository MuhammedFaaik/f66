#include "game_engine.h"
#include <cmath>
#include <algorithm>

bool GameEngine::initialize() {
    // Initialize football field dimensions
    fieldWidth = 105.0f;
    fieldHeight = 68.0f;
    
    // Initialize ball
    ball.position = {0.0f, 0.0f, 0.0f};
    ball.velocity = {0.0f, 0.0f, 0.0f};
    ball.radius = 0.11f; // Standard football size
    
    // Initialize physics
    gravity = -9.8f;
    friction = 0.98f;
    
    std::cout << "Game Engine initialized successfully!" << std::endl;
    return true;
}

void GameEngine::update(float deltaTime) {
    // Update ball physics
    updateBallPhysics(deltaTime);
    
    // Update player positions
    for (auto& player : players) {
        updatePlayerPhysics(player.second, deltaTime);
    }
    
    // Check for collisions
    checkCollisions();
    
    // Check for goals
    checkGoal();
    
    // Update game time
    gameTime += deltaTime;
}

void GameEngine::updateBallPhysics(float deltaTime) {
    // Apply gravity if ball is in air
    if (ball.position.y > 0) {
        ball.velocity.y += gravity * deltaTime;
    } else {
        ball.position.y = 0;
        ball.velocity.y = 0;
    }
    
    // Apply friction
    ball.velocity.x *= friction;
    ball.velocity.z *= friction;
    
    // Update position
    ball.position.x += ball.velocity.x * deltaTime;
    ball.position.y += ball.velocity.y * deltaTime;
    ball.position.z += ball.velocity.z * deltaTime;
    
    // Boundary check
    if (std::abs(ball.position.x) > fieldWidth / 2) {
        ball.position.x = std::copysign(fieldWidth / 2, ball.position.x);
        ball.velocity.x *= -0.8f; // Bounce with energy loss
    }
    
    if (std::abs(ball.position.z) > fieldHeight / 2) {
        ball.position.z = std::copysign(fieldHeight / 2, ball.position.z);
        ball.velocity.z *= -0.8f;
    }
}

void GameEngine::movePlayer(int playerId, float dx, float dy, float dz) {
    if (players.find(playerId) != players.end()) {
        Player& player = players[playerId];
        player.velocity.x = dx * player.speed;
        player.velocity.z = dz * player.speed;
        
        // Update position
        player.position.x += player.velocity.x;
        player.position.z += player.velocity.z;
        
        // Boundary check
        player.position.x = std::clamp(player.position.x, -fieldWidth/2, fieldWidth/2);
        player.position.z = std::clamp(player.position.z, -fieldHeight/2, fieldHeight/2);
    }
}

void GameEngine::kickBall(int playerId, float power) {
    if (players.find(playerId) != players.end()) {
        Player& player = players[playerId];
        
        // Calculate direction from player to ball
        float dx = ball.position.x - player.position.x;
        float dz = ball.position.z - player.position.z;
        float distance = std::sqrt(dx*dx + dz*dz);
        
        // Only kick if player is close to ball
        if (distance < 2.0f) {
            // Normalize direction
            if (distance > 0) {
                dx /= distance;
                dz /= distance;
            }
            
            // Apply kick force
            ball.velocity.x = dx * power;
            ball.velocity.z = dz * power;
            ball.velocity.y = power * 0.3f; // Lift the ball
            
            std::cout << "Player " << playerId << " kicked the ball with power " << power << std::endl;
        }
    }
}

void GameEngine::checkGoal() {
    // Check if ball is in goal (simplified)
    bool isInLeftGoal = ball.position.z < -fieldHeight/2 && 
                       std::abs(ball.position.x) < 7.32f/2 && // goal width
                       ball.position.y < 2.44f; // goal height
    
    bool isInRightGoal = ball.position.z > fieldHeight/2 && 
                        std::abs(ball.position.x) < 7.32f/2 &&
                        ball.position.y < 2.44f;
    
    if (isInLeftGoal) {
        score.team2++;
        std::cout << "Goal! Team 2 scores! Current score: " 
                  << score.team1 << " - " << score.team2 << std::endl;
        resetBall();
    } else if (isInRightGoal) {
        score.team1++;
        std::cout << "Goal! Team 1 scores! Current score: " 
                  << score.team1 << " - " << score.team2 << std::endl;
        resetBall();
    }
}

void GameEngine::resetBall() {
    ball.position = {0.0f, 0.0f, 0.0f};
    ball.velocity = {0.0f, 0.0f, 0.0f};
}

void GameEngine::addPlayer(int playerId, const std::string& name, bool isHomeTeam) {
    Player newPlayer;
    newPlayer.id = playerId;
    newPlayer.name = name;
    newPlayer.speed = 5.0f;
    newPlayer.position = {0.0f, 0.0f, isHomeTeam ? -20.0f : 20.0f};
    newPlayer.team = isHomeTeam ? 0 : 1;
    
    players[playerId] = newPlayer;
    std::cout << "Added player: " << name << " (ID: " << playerId << ")" << std::endl;
}