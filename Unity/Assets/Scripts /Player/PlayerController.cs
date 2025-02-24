using System;
using System.Collections;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using UnityEngine;

public class PlayerController : MonoBehaviour
{
    private CharacterController controller;
    private Vector3 direction;
    public float forwardSpeed;
    public float maxSpeed;
    private int desiredLane = 1; // 0: left, 1: middle, 2: right
    public float laneDistance = 4; // Distance between lanes

    public float jumpForce = 10f;
    public float Gravity = -20f;

    private float ySpeed = 0f;
    private bool isSliding = false;
    public Animator animator;

    // Lowered threshold for better response
    private float movementThreshold = 100f;  
    private bool canMoveLeft = true;
    private bool canMoveRight = true;
    private bool isJumping = false;

    // TCP Variables
    private Thread tcpThread;
    private TcpListener listener;
    private volatile bool running = true;
    
    private float accelX = 0, accelY = 0;

    void Start()
    {
        controller = GetComponent<CharacterController>();
        StartTCPListener();
    }

    void Update()
    {
        if (!PlayerManager.isGameStarted)
        {
            return;
        }

        // Debugging: Print X and Y values
        Debug.Log($"AccelX: {accelX}, AccelY: {accelY}");

        // Increase speed over time
        if (forwardSpeed < maxSpeed)
        {
            forwardSpeed += 0.1f * Time.deltaTime;
        }

        animator.SetBool("isGameStarted", true);
        direction.z = forwardSpeed;
        animator.SetBool("isGrounded", controller.isGrounded);

        if (controller.isGrounded)
        {
            isJumping = false;
            ySpeed = -0.5f;
            if (accelY > movementThreshold && !isJumping) // Jump when tilting up
            {
                Jump();
            }
        }
        else
        {
            ySpeed += Gravity * Time.deltaTime;
        }

        if (accelY < -movementThreshold && !isSliding) // Slide when tilting down
        {
            StartCoroutine(Slide());
        }

        // **Fix: Left/Right Lane Switching**
        if (accelX < -movementThreshold && canMoveRight) // Tilt Right
        {
            if (desiredLane < 2)
            {
                desiredLane++;
                canMoveRight = false; 
                canMoveLeft = true;  
                Debug.Log($"Moved Right, desiredLane = {desiredLane}");
                StartCoroutine(ResetLaneSwitchLock());
            }
        }
        else if (accelX > movementThreshold && canMoveLeft) // Tilt Left
        {
            if (desiredLane > 0)
            {
                desiredLane--;
                canMoveLeft = false; 
                canMoveRight = true;
                Debug.Log($"Moved Left, desiredLane = {desiredLane}");
                StartCoroutine(ResetLaneSwitchLock());
            }
        }

        float targetX = (desiredLane - 1) * laneDistance;
        Vector3 targetPosition = new Vector3(targetX, transform.position.y, transform.position.z);

        Vector3 moveDirection = new Vector3(targetX - transform.position.x, ySpeed, 0);
        controller.Move(moveDirection * Time.deltaTime * 10); // Smooth movement

        Debug.Log($"DesiredLane: {desiredLane}, TargetX: {targetX}, CurrentX: {transform.position.x}");

        direction.y = ySpeed;
        controller.Move(direction * Time.deltaTime);
    }

    private void Jump()
    {
        if (!isJumping){
            ySpeed = jumpForce;
            isJumping = true;
        }
    }

    private IEnumerator Slide()
    {
        isSliding = true;
        animator.SetBool("isSliding", true);
        controller.center = new Vector3(0, -0.5f, 0);
        controller.height = 1;

        yield return new WaitForSeconds(1.3f);

        controller.center = new Vector3(0, 0, 0);
        controller.height = 2;
        animator.SetBool("isSliding", false);
        isSliding = false;
    }

    private IEnumerator ResetLaneSwitchLock()
    {
        yield return new WaitForSeconds(0.2f);
        canMoveLeft = true;
        canMoveRight = true;
    }

    private void StartTCPListener()
    {
        tcpThread = new Thread(() =>
        {
            listener = new TcpListener(IPAddress.Any, 5005);
            listener.Start();
            Debug.Log("Listening for FPGA data on port 5005...");

            while (running)
            {
                using (TcpClient client = listener.AcceptTcpClient())
                using (NetworkStream stream = client.GetStream())
                {
                    byte[] buffer = new byte[1024];
                    int bytesRead;

                    while ((bytesRead = stream.Read(buffer, 0, buffer.Length)) > 0)
                    {
                        string data = Encoding.ASCII.GetString(buffer, 0, bytesRead).Trim();
                        ParseAccelerometerData(data);
                    }
                }
            }
        });

        tcpThread.IsBackground = true;
        tcpThread.Start();
    }

    private void ParseAccelerometerData(string data)
    {
        try
        {
            string[] values = data.Split(',');
            if (values.Length >= 2)
            {
                accelX = float.Parse(values[0]); // X-axis (left/right)
                accelY = float.Parse(values[1]); // Y-axis (up/down)
                Debug.Log($"Received Data: X = {accelX}, Y = {accelY}");
            }
        }
        catch (Exception ex)
        {
            Debug.LogError("Error parsing accelerometer data: " + ex.Message);
        }
    }

    private void OnControllerColliderHit(ControllerColliderHit hit)
    {
        if (hit.transform.tag == "Obstacle")
        {
            Debug.Log("Game Over: Hit Obstacle!");
            PlayerManager.gameOver = true;
            FindObjectOfType<AudioManager>().PlaySound("GameOver");

        }
    }


    private void OnApplicationQuit()
    {
        running = false;
        listener.Stop();
        tcpThread.Abort();
    }
}
