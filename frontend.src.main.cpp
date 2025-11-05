#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "game_engine.h"
#include "network_manager.h"
#include "renderer.h"

class F66FootballGame {
private:
    GameEngine gameEngine;
    NetworkManager networkManager;
    Renderer renderer;
    bool isRunning;
    std::string authToken;
    int userId;

public:
    F66FootballGame() : isRunning(false) {}

    bool initialize() {
        std::cout << "Initializing F66 Football Game..." << std::endl;
        
        // Initialize systems
        if (!renderer.initialize()) {
            std::cerr << "Failed to initialize renderer!" << std::endl;
            return false;
        }
        
        if (!networkManager.initialize()) {
            std::cerr << "Failed to initialize network!" << std::endl;
            return false;
        }
        
        if (!gameEngine.initialize()) {
            std::cerr << "Failed to initialize game engine!" << std::endl;
            return false;
        }
        
        std::cout << "F66 Game initialized successfully!" << std::endl;
        return true;
    }

    void login(const std::string& email, const std::string& password) {
        auto response = networkManager.authenticate(email, password);
        if (response.success) {
            authToken = response.authToken;
            userId = response.userId;
            std::cout << "Login successful! Welcome to F66 Football." << std::endl;
        } else {
            std::cerr << "Login failed: " << response.errorMessage << std::endl;
        }
    }

    void run() {
        isRunning = true;
        auto lastTime = std::chrono::high_resolution_clock::now();
        
        while (isRunning) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;
            
            // Handle input
            handleInput();
            
            // Update game state
            update(deltaTime);
            
            // Render frame
            render();
            
            // Cap at 60 FPS
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }

    void handleInput() {
        // Handle keyboard input for player movement
        if (renderer.isKeyPressed(GLFW_KEY_W)) {
            gameEngine.movePlayer(userId, 0.0f, 0.0f, 1.0f);
        }
        if (renderer.isKeyPressed(GLFW_KEY_S)) {
            gameEngine.movePlayer(userId, 0.0f, 0.0f, -1.0f);
        }
        if (renderer.isKeyPressed(GLFW_KEY_A)) {
            gameEngine.movePlayer(userId, -1.0f, 0.0f, 0.0f);
        }
        if (renderer.isKeyPressed(GLFW_KEY_D)) {
            gameEngine.movePlayer(userId, 1.0f, 0.0f, 0.0f);
        }
        if (renderer.isKeyPressed(GLFW_KEY_SPACE)) {
            gameEngine.kickBall(userId, 10.0f);
        }
    }

    void update(float deltaTime) {
        gameEngine.update(deltaTime);
        
        // Send player updates to server
        auto playerState = gameEngine.getPlayerState(userId);
        networkManager.sendPlayerUpdate(authToken, playerState);
        
        // Receive game state updates
        auto gameState = networkManager.receiveGameState(authToken);
        if (gameState.valid) {
            gameEngine.applyGameState(gameState);
        }
    }

    void render() {
        renderer.beginFrame();
        
        // Render football field
        renderer.renderField();
        
        // Render players
        auto players = gameEngine.getAllPlayers();
        for (const auto& player : players) {
            renderer.renderPlayer(player);
        }
        
        // Render ball
        auto ball = gameEngine.getBall();
        renderer.renderBall(ball);
        
        // Render UI
        renderer.renderUI(gameEngine.getScore(), gameEngine.getTime());
        
        renderer.endFrame();
    }

    void shutdown() {
        isRunning = false;
        renderer.shutdown();
        networkManager.shutdown();
        std::cout << "F66 Football Game shut down." << std::endl;
    }
};

int main() {
    F66FootballGame game;
    
    if (!game.initialize()) {
        return -1;
    }
    
    // Example login (in real game, get from user input)
    game.login("player@f66.com", "password123");
    
    // Run main game loop
    game.run();
    
    game.shutdown();
    return 0;
}