using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerMovement : MonoBehaviour
{
    public float speed = 5.0f;

    void Update()
    {
        float moveX = Input.GetAxis("Horizontal"); // Left/Right
        float moveZ = Input.GetAxis("Vertical"); // Forward/Backward

        transform.Translate(new Vector3(moveX, 0, moveZ) * speed * Time.deltaTime);
    }
}
