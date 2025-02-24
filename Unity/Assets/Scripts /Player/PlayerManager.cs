using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;
using UnityEngine.UI;
using TMPro;

public class PlayerManager : MonoBehaviour
{
    public static bool gameOver;  // Static variable to check if the game is over
    public GameObject gameOverPanel;  // Reference to the game over UI panel

    public static bool isGameStarted;  // Static variable to check if the game has started
    public GameObject startingText;

    public static int numberofCoins;
    public TMP_Text coinsText;

    // Start is called once before the first execution of Update after the MonoBehaviour is created
    void Start()
    {
        Time.timeScale = 1;  // Ensure the game starts running
        gameOver = false;    // Initially, game is not over
        gameOverPanel.SetActive(false);  // Ensure game over panel is not visible at the start
        isGameStarted = false;  // Initially, the game has not started
        numberofCoins = 0;
    }

    // Update is called once per frame
    void Update()
    {
        // Check if the player presses the Space key to start the game
        if (!isGameStarted && Input.GetKeyDown(KeyCode.Space))  // Spacebar to start the game
        {
            Debug.Log("Game Started!");
            isGameStarted = true;  // The game has started
            Destroy(startingText);
        }

        if (gameOver)
        {
            // Show the game over panel and pause the game
            gameOverPanel.SetActive(true);
            Time.timeScale = 0;  // Pause the game
        }
        coinsText.text = "Coins: " + numberofCoins;

        // Restart the game when "R" key is pressed (optional, you can change the key)
        if (gameOver && Input.GetKeyDown(KeyCode.R))  
        {
            Debug.Log("Restarting the game...");
            RestartGame();
        }
    }

    // This method will be called to end the game
    public static void EndGame()
    {
        Debug.Log("Game Over Triggered");
        gameOver = true;
    }

    // This method will reset the game
    void RestartGame()
    {
        // Reset anything you need here (player position, score, etc.)
        Debug.Log("Restarting game...");
        gameOver = false;  
        Time.timeScale = 1;  // Resume the game
        gameOverPanel.SetActive(false);  // Hide the game over panel

        // Optionally, you can reload the scene here if required
        // SceneManager.LoadScene(SceneManager.GetActiveScene().name);
    }
}
